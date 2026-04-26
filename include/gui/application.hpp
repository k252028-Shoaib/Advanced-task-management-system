#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h> // For std::string support
#include <string>
#include <vector>

#include "data_management/data_management.hpp"
#include "gui/IconsFontAwesome6.h"

class Application {
public:
    Application();
    ~Application();

    // The main execution loop
    void run();

private:
    // --- Initialization ---
    bool init_window();
    void init_imgui();
    void load_fonts();
    void cleanup();

    // --- UI Layers (Panels) ---
    void render_ui();
    void draw_dockspace();
    void draw_sidebar();
    void draw_main_content();
    void draw_task_list();
    void render_toasts();
    struct Toast {
        std::string message;
        float timer; // Countdown in seconds
    };
    std::vector<Toast> toasts;
    
    // --- Modals (Popups) ---
    void draw_task_details_modal();
    void draw_create_task_modal();
    void draw_subtask_warning_modal();
    void draw_delete_confirmation();
    void draw_auto_complete_modal();
    void add_toast(std::string msg) { toasts.push_back({msg, 3.0f}); }

    // --- Helpers ---
    ImVec4 hex_to_imvec4(const std::string& hex);
    void apply_theme(bool dark);
    std::string new_subtask_name_buffer = "";
    bool show_auto_complete_modal = false;
    task* task_to_auto_complete = nullptr;
    

    // --- Backend & Window ---
    GLFWwindow* window;
    data_management db;
    float scale_factor = 1.0f;
    

    // --- Font Pointers ---
    ImFont* font_regular = nullptr;
    ImFont* font_bold = nullptr;
    ImFont* font_icons = nullptr;

    // --- UI State (The "Memory") ---
    task* selected_task = nullptr;       // Pointer to task currently being viewed
    bool show_details_modal = false;
    bool show_create_modal = false;
    bool is_editing_mode = false;        // Toggles between View/Edit in the modal
    bool use_dark_theme = true;
    bool show_subtask_warning = false;
    std::string subtask_warning_name = "";

    // --- Filter Logic ---
    enum class FilterType { ALL, OVERDUE, TODAY, UPCOMING, COMPLETED };
    enum class SortType { NONE, PRIORITY, DUE_DATE };
    FilterType current_filter = FilterType::ALL;
    SortType current_sort = SortType::NONE;
    int filter_category_id = -1; // -1 means "All Categories"
    int task_to_delete = -1;

    // Temporary buffers for "Create Task" logic
    task_members task_buffer; // a struct in data_management.hpp

    // Date buffers
    int d_day = 1, d_mon = 1, d_year = 2026;
    bool is_recurring_toggle = false;

    // Recurring buffers
    int rec_interval_mins = 1440; // Default 1 day
    int rec_occurrences = -1;     // Default infinite
};