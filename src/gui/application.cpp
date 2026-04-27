#include "gui/application.hpp"
#include "gui/IconsFontAwesome6.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "task_structure/subtask.hpp"

// Required for DPI Awareness on Windows
#ifdef _WIN32
#include <windows.h>
#endif

bool is_valid_date(int d, int m, int y) {
    using namespace std::chrono;
    
    // 1. Check if the date itself is valid (e.g., not Feb 30)
    year_month_day ymd{ year{y}, month{static_cast<unsigned>(m)}, day{static_cast<unsigned>(d)} };
    if (!ymd.ok()) return false;

    // 2. Compare to today (Midnight)
    auto input_days = sys_days{ymd};
    auto today = floor<days>(system_clock::now());
    
    return input_days >= today;
}

Application::Application() {
#ifdef _WIN32
    // Tells Windows 11 to let us handle scaling (prevents blur)
    SetProcessDPIAware(); 
#endif

    if (!init_window()) throw std::runtime_error("GLFW Init Failed");
    init_imgui();
    load_fonts();
    db.load_from_file();
}

Application::~Application() {
    db.save_to_file();
    cleanup();
}

bool Application::init_window() {
    if (!glfwInit()) return false;

    // OpenGL 3.0 + GLSL 130
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(1280, 720, "Advanced Task Manager", nullptr, nullptr);
    if (!window) return false;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync

    // Detect DPI Scale (e.g., 1.0 for 100%, 1.5 for 150%)
    float xscale, yscale;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    scale_factor = xscale; 

    return true;
}

void Application::init_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    
    apply_theme(use_dark_theme);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void Application::load_fonts() {
    ImGuiIO& io = ImGui::GetIO();
    
    // Clear any existing fonts to start fresh
    io.Fonts->Clear();

    float base_size = 18.0f * scale_factor;
    
    // 1. Setup Icon Config
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true; 
    icons_config.PixelSnapH = true;
    icons_config.GlyphOffset.y = 1.0f * scale_factor; // Better vertical alignment

    // 2. Load Regular Font + Icons
    font_regular = io.Fonts->AddFontFromFileTTF("assets/fonts/Inter_24pt-Regular.ttf", base_size);
    io.Fonts->AddFontFromFileTTF("assets/fonts/Font Awesome 6 Free-Solid-900.otf", base_size, &icons_config, icons_ranges);

    // 3. Load Bold Font + Icons
    font_bold = io.Fonts->AddFontFromFileTTF("assets/fonts/Inter_24pt-Bold.ttf", base_size + 2.0f);
    io.Fonts->AddFontFromFileTTF("assets/fonts/Font Awesome 6 Free-Solid-900.otf", base_size + 2.0f, &icons_config, icons_ranges);

    io.Fonts->Build();
}

ImVec4 Application::hex_to_imvec4(const std::string& hex) {
    int r, g, b;
    // Parses #RRGGBB format
    if (sscanf(hex.c_str(), "#%02x%02x%02x", &r, &g, &b) != 3) return ImVec4(1,1,1,1);
    return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

void Application::apply_theme(bool dark) {
    if (dark) ImGui::StyleColorsDark();
    else ImGui::StyleColorsLight();
    
    // Tweak styling for "Professional" look
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.ScaleAllSizes(scale_factor);
}

void Application::run() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Setup the Docking Root
        draw_dockspace();

        // 2. Render our UI Panels (This is where the buttons get clicked and set our triggers to true)
        render_ui();
        
        // 3. CHECK TRIGGERS: Open the popups exactly ONCE if a button was clicked
        if (trigger_details_modal) { ImGui::OpenPopup("Task Details"); trigger_details_modal = false; }
        if (trigger_create_modal) { ImGui::OpenPopup("Create New Task"); trigger_create_modal = false; }
        if (trigger_delete_modal) { ImGui::OpenPopup("Delete Task?"); trigger_delete_modal = false; }
        if (trigger_subtask_warning) { ImGui::OpenPopup("Subtask Warning"); trigger_subtask_warning = false; }

        // 4. Draw the Modals (These only actually draw anything if OpenPopup was called above)
        draw_task_details_modal();
        draw_create_task_modal();
        draw_delete_confirmation();
        draw_subtask_warning_modal();

        // Rendering
        render_toasts();
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        
        // Background color (matches theme)
        ImVec4 clear_color = use_dark_theme ? ImVec4(0.1f, 0.1f, 0.1f, 1.0f) : ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

void Application::draw_dockspace() {
    // This makes the DockSpace fill the entire GLFW window
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpaceParent", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    // Create the actual DockSpace
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    // Top Menu Bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save", "Ctrl+S")) db.save_to_file();
            if (ImGui::MenuItem("Exit")) glfwSetWindowShouldClose(window, true);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Dark Mode", nullptr, &use_dark_theme)) apply_theme(use_dark_theme);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::End();
}

void Application::render_ui() {
    draw_sidebar();
    draw_main_content();
}

void Application::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void Application::draw_sidebar() {
    ImGui::Begin(ICON_FA_LIST " Navigation");

    if (ImGui::Button(ICON_FA_PLUS " Create New Task", ImVec2(-1, 40))) {
        trigger_create_modal = true; // Set trigger, don't call OpenPopup here
        
        // Clean buffers and prepopulate today's date
        task_buffer = task_members();
        auto today = std::chrono::year_month_day{std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now())};
        d_day = static_cast<unsigned>(today.day());
        d_mon = static_cast<unsigned>(today.month());
        d_year = static_cast<int>(today.year());
        is_recurring_toggle = false;
    }
    ImGui::Spacing();
    ImGui::Separator();

    ImGui::TextDisabled("CATEGORIES");
    if (ImGui::Selectable(" All Categories", filter_category_id == -1)) filter_category_id = -1;
    for (const auto& cat : db.get_categories()) {
        ImGui::PushID(cat.id);
        
        // Color indicator
        ImGui::ColorButton("##c", hex_to_imvec4(cat.hex_color), ImGuiColorEditFlags_NoTooltip, ImVec2(10, 10));
        ImGui::SameLine();
        
        // The Selectable name
        if (ImGui::Selectable(cat.name.c_str(), filter_category_id == cat.id)) {
            filter_category_id = cat.id;
        }
    
        // Right-click to delete category
        if (ImGui::BeginPopupContextItem()) {
            if (cat.id != 1) { // Prevent deleting "General"
                if (ImGui::MenuItem(ICON_FA_TRASH_CAN " Delete Category")) {
                    db.delete_category(cat.id);
                    filter_category_id = -1;
                }
            } else {
                ImGui::TextDisabled("Cannot delete General");
            }
            ImGui::EndPopup();
        }
        
        ImGui::PopID();
    }

    ImGui::Separator();
    static char new_cat_name[32] = "";
    static ImVec4 new_cat_color = ImVec4(1, 1, 1, 1);
    
    ImGui::InputText("##catname", new_cat_name, 32);
    ImGui::ColorEdit4("##catcolor", (float*)&new_cat_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
    ImGui::SameLine();
    
    if (ImGui::Button(ICON_FA_PLUS)) {
        if (strlen(new_cat_name) > 0) {
            // Convert ImVec4 back to Hex string
            char hex[8];
            snprintf(hex, sizeof(hex), "#%02X%02X%02X", 
                     (int)(new_cat_color.x * 255), 
                     (int)(new_cat_color.y * 255), 
                     (int)(new_cat_color.z * 255));
            
            db.add_category(new_cat_name, hex);
            new_cat_name[0] = '\0'; // Clear input
        }
    }
    
    ImGui::Spacing();
    ImGui::TextDisabled("FILTERS");
    if (ImGui::Selectable(ICON_FA_CALENDAR " Pending Tasks", current_filter == FilterType::ALL)) current_filter = FilterType::ALL;
    if (ImGui::Selectable(ICON_FA_TRIANGLE_EXCLAMATION " Overdue", current_filter == FilterType::OVERDUE)) current_filter = FilterType::OVERDUE;
    if (ImGui::Selectable(ICON_FA_CHECK_DOUBLE " Completed", current_filter == FilterType::COMPLETED)) current_filter = FilterType::COMPLETED;

    ImGui::Spacing();
    ImGui::TextDisabled("SORT BY");
    if (ImGui::RadioButton("Default", (int*)&current_sort, 0)) current_sort = SortType::NONE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Priority", (int*)&current_sort, 1)) current_sort = SortType::PRIORITY;
    ImGui::SameLine();
    if (ImGui::RadioButton("Date", (int*)&current_sort, 2)) current_sort = SortType::DUE_DATE;

    ImGui::End();
}

void Application::draw_task_list() {
    using namespace std::chrono;

    // 1. Collect and Filter into a temporary list
    std::vector<task*> tasks_to_show;
    auto now_days = floor<days>(system_clock::now());

    for (auto& t_ptr : db.get_all_tasks()) {
        task* t = t_ptr.get();

        // --- Filter: Completion Status ---
        if (current_filter == FilterType::COMPLETED) {
            if (!t->is_complete()) continue;
        } else {
            if (t->is_complete()) continue;
        }

        // --- Filter: Category ---
        if (filter_category_id != -1 && t->get_category_id() != filter_category_id) continue;

        // --- Filter: Date Logic (C++20 Chrono) ---
        if (current_filter == FilterType::OVERDUE && !t->is_overdue()) continue;

        if (current_filter == FilterType::TODAY) {
            if (!t->get_has_due()) continue;
            auto due_days = floor<days>(system_clock::from_time_t(t->get_due_time().get_time_t()));
            if (due_days != now_days) continue;
        }

        if (current_filter == FilterType::UPCOMING) {
            if (!t->get_has_due()) continue;
            auto due_days = floor<days>(system_clock::from_time_t(t->get_due_time().get_time_t()));
            if (due_days <= now_days) continue;
        }

        tasks_to_show.push_back(t);
    }

    // 2. Sort the temporary list
    if (current_sort == SortType::PRIORITY) {
        std::sort(tasks_to_show.begin(), tasks_to_show.end(), [](task* a, task* b) {
            return a->get_priority() > b->get_priority();
        });
    } else if (current_sort == SortType::DUE_DATE) {
        std::sort(tasks_to_show.begin(), tasks_to_show.end(), [](task* a, task* b) {
            if (!a->get_has_due()) return false;
            if (!b->get_has_due()) return true;
            return a->get_due_time() < b->get_due_time();
        });
    }

    // 3. Render the Table
    static ImGuiTableFlags table_flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
    
    if (ImGui::BeginTable("TaskTable", 5, table_flags)) {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 30.0f);
        ImGui::TableSetupColumn("Task Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Priority", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Due Date", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableHeadersRow();

        for (task* t : tasks_to_show) {
            ImGui::TableNextRow();
            ImGui::PushID(t->get_id());

            // --- Column 0: Status Icon ---
            ImGui::TableSetColumnIndex(0);
            if (t->is_complete()) ImGui::TextColored(ImVec4(0, 1, 0, 1), ICON_FA_CIRCLE_CHECK);
            else if (t->is_overdue()) ImGui::TextColored(ImVec4(1, 0, 0, 1), ICON_FA_CIRCLE_EXCLAMATION);
            else ImGui::TextDisabled(ICON_FA_CIRCLE);

            // --- Column 1: Name & Recurring Icon ---
            ImGui::TableSetColumnIndex(1);
            if (dynamic_cast<recurring_task*>(t)) {
                ImGui::TextDisabled(ICON_FA_RETWEET); 
                ImGui::SameLine();
            }
            ImGui::Text("%s", t->get_name().c_str());

            // --- Column 2: Priority ---
            ImGui::TableSetColumnIndex(2);
            if (t->get_priority() >= 4) ImGui::TextColored(ImVec4(1, 0.2f, 0.2f, 1), ICON_FA_FIRE " HIGH");
            else if (t->get_priority() >= 2) ImGui::TextColored(ImVec4(1, 0.8f, 0, 1), "MED");
            else ImGui::TextDisabled("LOW");

            // --- Column 3: Due Date ---
            ImGui::TableSetColumnIndex(3);
            if (t->get_has_due()) {
                std::stringstream ss;
                ss << t->get_due_time();
                ImGui::Text("%s", ss.str().c_str());
            } else {
                ImGui::TextDisabled("None");
            }

            // --- Column 4: Actions ---
            ImGui::TableSetColumnIndex(4);
            if (ImGui::SmallButton(ICON_FA_CHECK)) {
                if (t->complete_task()) {
                    add_toast("Task completed!");
                } else {
                    subtask_warning_name = t->get_name();
                    trigger_subtask_warning = true; // Use the new trigger!
                }
            }
            ImGui::SameLine();
            if (ImGui::SmallButton(ICON_FA_EYE)) {
                selected_task = t;
                trigger_details_modal = true; // Use the new trigger!
                is_editing_mode = false;
            }
            ImGui::SameLine();
            if (ImGui::SmallButton(ICON_FA_TRASH_CAN)) {
                task_to_delete = t->get_id();
                trigger_delete_modal = true; // Use the new trigger!
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

void Application::draw_main_content() {
    // This is where the actual tasks will appear
    ImGui::Begin(ICON_FA_LIST_CHECK " Task Workspace");
    
    if (db.get_all_tasks().empty()) {
        ImGui::Text("No tasks found. Click 'New Task' to get started!");
    } else {
        draw_task_list();
    }

    ImGui::End();
}

void Application::draw_delete_confirmation() {
    if (ImGui::BeginPopupModal("Delete Task?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("This action cannot be undone.\nAre you sure you want to delete this task?");
        ImGui::Separator();

        if (ImGui::Button("Cancel", ImVec2(120, 0))) { 
            task_to_delete = -1;
            ImGui::CloseCurrentPopup(); 
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("Delete", ImVec2(120, 0))) {
            db.delete_task(task_to_delete);
            task_to_delete = -1;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor();
        ImGui::EndPopup();
    }
}

void Application::draw_task_details_modal() {
    if (!selected_task) return;

    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (ImGui::BeginPopupModal("Task Details", nullptr)) {
        
        // --- Header ---
        ImGui::PushFont(font_bold);
        ImGui::Text(ICON_FA_CIRCLE_INFO " %s", selected_task->get_name().c_str());
        ImGui::PopFont();
        
        ImGui::SameLine(ImGui::GetWindowWidth() - 120);
        ImGui::Checkbox("Edit Mode", &is_editing_mode);
        ImGui::Separator();

        // --- Core Info ---
        if (is_editing_mode) {
            std::string name = selected_task->get_name();
            if (ImGui::InputText("Task Name", &name)) selected_task->set_name(name);
            
            std::string desc = selected_task->get_description();
            if (ImGui::InputTextMultiline("Description", &desc)) selected_task->set_description(desc);
        } else {
            ImGui::TextWrapped("%s", selected_task->get_description().empty() ? "No description." : selected_task->get_description().c_str());
        }

        // --- Subtask Section ---
        ImGui::Spacing();
        ImGui::TextDisabled("SUBTASKS");
        ImGui::Separator();

        // Existing Subtasks
        for (const auto& sub : selected_task->get_subtasks()) {
            bool comp = sub.is_complete();
            if (ImGui::Checkbox(sub.get_name().c_str(), &comp)) {
                if (comp) {
                    auto result = selected_task->complete_subtask(sub.get_id());
                    if (result == task::SubtaskResult::ALL_SUBTASKS_COMPLETED) {
                        task_to_auto_complete = selected_task;
                        ImGui::OpenPopup("Auto-Complete?");
                    }
                } 
                else selected_task->uncomplete_subtask(sub.get_id());
            }
        }

        // Add New Subtask Input
        ImGui::Spacing();
        ImGui::InputText(ICON_FA_PLUS " New Subtask", &new_subtask_name_buffer);
        ImGui::SameLine();
        if (ImGui::Button("Add")) {
            if (!new_subtask_name_buffer.empty()) {
                selected_task->add_subtask(new_subtask_name_buffer, "", std::chrono::minutes(0));
                new_subtask_name_buffer = ""; // Clear buffer
            }
        }

        ImGui::Separator();
        if (ImGui::Button("Close", ImVec2(120, 0))) {
            selected_task = nullptr;
            ImGui::CloseCurrentPopup();
        }
        draw_auto_complete_modal(); 

        ImGui::EndPopup();
    }
}

void Application::draw_create_task_modal() {
    // 1. Calculate validity once at the start of the frame
    bool date_valid = is_valid_date(d_day, d_mon, d_year);
    bool name_valid = !task_buffer.name.empty();
    bool can_create = date_valid && name_valid;

    // Notice we pass 'nullptr' here now instead of '&show_create_modal'
    if (ImGui::BeginPopupModal("Create New Task", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        
        ImGui::InputText("Task Name", &task_buffer.name);
        ImGui::InputTextMultiline("Description", &task_buffer.description);
        ImGui::SliderInt("Priority", &task_buffer.priority, 1, 5);

        ImGui::TextDisabled("DUE DATE");
        ImGui::SetNextItemWidth(50); ImGui::InputInt("DD", &d_day, 0); ImGui::SameLine();
        ImGui::SetNextItemWidth(50); ImGui::InputInt("MM", &d_mon, 0); ImGui::SameLine();
        ImGui::SetNextItemWidth(70); ImGui::InputInt("YYYY", &d_year, 0);

        ImGui::Separator();
        ImGui::Checkbox("Is Recurring?", &is_recurring_toggle);
        if (is_recurring_toggle) {
            ImGui::InputInt("Interval (Mins)", &rec_interval_mins);
            ImGui::InputInt("Occurrences", &rec_occurrences);
        }

        ImGui::Separator();

        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            // We removed the boolean toggle, just tell ImGui to close it
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (!can_create) ImGui::BeginDisabled();
        
        if (ImGui::Button("Create Task", ImVec2(120, 0))) {
            date due_date(0, 0, 0, d_day, d_mon, d_year);
            task_buffer.due = due_date;

            if (is_recurring_toggle) {
                recurring_task_members rec_req;
                static_cast<task_members&>(rec_req) = task_buffer;
                rec_req.interval = std::chrono::minutes(rec_interval_mins);
                rec_req.occurrences = rec_occurrences;
                db.add_recurring_task(rec_req);
            } else {
                db.add_task(task_buffer);
            }

            task_buffer = task_members(); 
            // Tell ImGui to close it
            ImGui::CloseCurrentPopup();
        }

        if (!can_create) ImGui::EndDisabled(); 

        if (!can_create) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Check Name/Date!");
        }

        ImGui::EndPopup();
    }
}

void Application::draw_subtask_warning_modal() {
    if (ImGui::BeginPopupModal("Subtask Warning", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), ICON_FA_TRIANGLE_EXCLAMATION " Action Required");
        ImGui::Text("Cannot complete task: \"%s\"", subtask_warning_name.c_str());
        ImGui::Text("Please finish or delete all subtasks first.");
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            // We removed the boolean toggle, just tell ImGui to close it
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void Application::draw_auto_complete_modal() {
    if (ImGui::BeginPopupModal("Auto-Complete?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("All subtasks for \"%s\" are finished.", task_to_auto_complete->get_name().c_str());
        ImGui::Text("Would you like to mark the main task as complete?");
        ImGui::Separator();

        if (ImGui::Button("No", ImVec2(120, 0))) {
            // Just tell ImGui to close the popup
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Yes, Complete", ImVec2(120, 0))) {
            task_to_auto_complete->complete_task();
            add_toast("Task completed!"); // Trigger toast
            
            // Just tell ImGui to close the popup. 
            // (The user can close the background Details panel themselves)
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void Application::render_toasts() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 pos = viewport->WorkPos;
    pos.x += viewport->WorkSize.x - 20;
    pos.y += viewport->WorkSize.y - 20;

    for (int i = 0; i < toasts.size(); i++) {
        // Position each toast above the last
        ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y - (i * 50)), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
        
        ImGui::Begin(("Toast" + std::to_string(i)).c_str(), nullptr, 
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | 
            ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing);
        
        ImGui::Text(ICON_FA_CIRCLE_INFO " %s", toasts[i].message.c_str());
        ImGui::End();

        // Update timer
        toasts[i].timer -= ImGui::GetIO().DeltaTime;
    }

    // Remove expired toasts
    toasts.erase(std::remove_if(toasts.begin(), toasts.end(), 
        [](const Toast& t) { return t.timer <= 0; }), toasts.end());
}

