#include "../../include/data_management/data_management.hpp"
#include "../../include/task_structure/task.hpp"
#include "../../include/task_structure/subtask.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>

data_management::data_management() : next_task_id(1), next_category_id(1) {
    // Initialize with a "General" category
    add_category("General", "#FFFFFF");
}

// --- Task Management ---

task* data_management::add_task(const task_members& req) {
    auto new_task = std::make_unique<task>(
        next_task_id++, 
        req.name, 
        req.start, 
        req.due, 
        req.reminder, 
        req.priority, 
        req.description, 
        req.category_id
    );

    task* ptr = new_task.get();
    tasks_arr.push_back(std::move(new_task));
    return ptr;
}

task* data_management::add_recurring_task(const recurring_task_members& req) {
    // Polymorphism: recurring_task is stored in a unique_ptr<task>
    auto new_task = std::make_unique<recurring_task>(
        next_task_id++, 
        req.name, 
        req.start, 
        req.due, 
        req.reminder, 
        req.priority, 
        req.description, 
        req.category_id,
        req.interval,
        req.occurrences,
        req.end_date
    );
    
    task* ptr = new_task.get();
    tasks_arr.push_back(std::move(new_task));
    return ptr;
}

bool data_management::delete_task(int id) {
    task* temp = find_task_by_id(id);
    if(temp == nullptr) return false;

    std::erase(tasks_arr, temp);
    return true;
}

task* data_management::find_task_by_id(int id) {
    for (auto& t : tasks_arr) {
        if (t->get_id() == id) return t.get();
    }
    return nullptr;
}

// --- Category Management ---

void data_management::add_category(std::string name, std::string color) {
    categories_arr.emplace_back(next_category_id++, name, color);
}

void data_management::delete_category(int id) {
    // Cannot delete the "General" category (ID 1)
    if (id == 1) return;

    std::erase_if(categories_arr, [id](const category& c) {
        return c.id == id;
    });

    // Clean up: Any tasks assigned to this category revert to "General" (ID 1)
    for (auto& t : tasks_arr) {
        if (t->get_category_id() == id) {
            t->set_category(1);
        }
    }
}


//Filing:


void data_management::save_to_file() {
    std::ofstream file("database.txt");
    if (!file.is_open()) return;

    // 1. Save Categories
    file << "[CATEGORIES] " << categories_arr.size() << "\n";
    for (const auto& cat : categories_arr) {
        file << cat.id << "|" << cat.name << "|" << cat.hex_color << "\n";
    }

    // 2. Save Tasks
    file << "[TASKS] " << tasks_arr.size() << "\n";
    for (const auto& t : tasks_arr) {
        // Identify Type
        recurring_task* rec = dynamic_cast<recurring_task*>(t.get());
        if (rec) file << "REC_TASK|";
        else file << "TASK|";

        // Save Basic Task Data
        file << t->get_id() << "|"
             << t->get_name() << "|"
             << t->get_priority() << "|"
             << t->get_category_id() << "|"
             << t->get_status() << "|"
             << t->get_start_time().get_time_t() << "|" 
             << t->get_due_time().get_time_t() << "|"   
             << t->get_reminder_time().get_time_t() << "|" 
             << t->get_has_start() << "|"
             << t->get_has_due() << "|"
             << t->get_has_reminder() << "|";

        if (rec) {
            file << rec->get_interval().count() << "|"
                 << rec->get_occurrences() << "|"
                 << rec->get_end_date().get_time_t() << "|"
                 << rec->get_has_end() << "|"; 
        }

        //3. Save Subtasks for this task
        const auto& subs = t->get_subtasks();
        file << "\n[SUBTASKS] " << subs.size() << "\n";
        for (const auto& sub : subs) {
            file << sub.get_id() << "|"
                 << sub.get_name() << "|"
                 << sub.is_complete() << "|"
                 << sub.get_description() << "|"
                 << sub.get_estimated_duration_minutes() << "\n";
        }
        file << "[END_TASK]\n";
    }
    file.close();
}

void data_management::load_from_file() {
    std::ifstream file("database.txt");
    if (!file.is_open()) return;

    tasks_arr.clear();
    categories_arr.clear();
    std::string line;

    // 1. Load Categories
    if (std::getline(file, line) && line.find("[CATEGORIES]") != std::string::npos) {
        int cat_count = std::stoi(line.substr(line.find(" ") + 1));
        for (int i = 0; i < cat_count; ++i) {
            if (std::getline(file, line)) {
                std::stringstream ss(line);
                std::string s_id, s_name, s_color;
                std::getline(ss, s_id, '|');
                std::getline(ss, s_name, '|');
                std::getline(ss, s_color, '|');

                int id = std::stoi(s_id);
                categories_arr.emplace_back(id, s_name, s_color);
                if (id >= next_category_id) next_category_id = id + 1;
            }
        }
    }

    // 2. Load Tasks
    if (std::getline(file, line) && line.find("[TASKS]") != std::string::npos) {
        int task_count = std::stoi(line.substr(line.find(" ") + 1));
        for (int i = 0; i < task_count; ++i) {
            if (!std::getline(file, line)) break;
            std::stringstream ss(line);
            std::string type, s_id, s_name, s_prio, s_cat, s_stat, s_start, s_due, s_rem, b_start, b_due, b_rem;

            std::getline(ss, type, '|');
            std::getline(ss, s_id, '|');
            std::getline(ss, s_name, '|');
            std::getline(ss, s_prio, '|');
            std::getline(ss, s_cat, '|');
            std::getline(ss, s_stat, '|');
            std::getline(ss, s_start, '|');
            std::getline(ss, s_due, '|');
            std::getline(ss, s_rem, '|');
            std::getline(ss, b_start, '|');
            std::getline(ss, b_due, '|');
            std::getline(ss, b_rem, '|');

            std::unique_ptr<task> t;
            int tid = std::stoi(s_id);

            if (type == "REC_TASK") {
                std::string s_int, s_occ, s_end, b_end;
                std::getline(ss, s_int, '|');
                std::getline(ss, s_occ, '|');
                std::getline(ss, s_end, '|');
                std::getline(ss, b_end, '|');

                auto rec = std::make_unique<recurring_task>(tid, s_name, std::chrono::minutes(std::stoi(s_int)));
                rec->set_occurrences(std::stoi(s_occ));
                if (b_end == "1") {
                    date ed; ed.from_time_t(std::stoll(s_end));
                    rec->set_end_date(ed);
                }
                t = std::move(rec);
            } else {
                t = std::make_unique<task>(tid, s_name);
            }

            // Apply Shared Data
            t->set_priority(std::stoi(s_prio));
            t->set_category(std::stoi(s_cat));
            if (s_stat == "1") t->complete_task();
            
            if (b_start == "1") { date d; d.from_time_t(std::stoll(s_start)); t->set_start_date(d); }
            if (b_due == "1")   { date d; d.from_time_t(std::stoll(s_due)); t->set_due_date(d); }
            if (b_rem == "1")   { date d; d.from_time_t(std::stoll(s_rem)); t->set_reminder_date(d); }

            // 3. Load Subtasks
            if (std::getline(file, line) && line.find("[SUBTASKS]") != std::string::npos) {
                int sub_count = std::stoi(line.substr(line.find(" ") + 1));
                for (int j = 0; j < sub_count; ++j) {
                    std::getline(file, line);
                    std::stringstream sub_ss(line);
                    std::string sub_id, sub_name, sub_stat, sub_desc, sub_edm;
                    std::getline(sub_ss, sub_id, '|');
                    std::getline(sub_ss, sub_name, '|');
                    std::getline(sub_ss, sub_stat, '|');
                    std::getline(sub_ss, sub_desc, '|');
                    std::getline(sub_ss, sub_edm, '|');

                    t->add_subtask(sub_name, sub_desc, std::chrono::minutes(std::stoi(sub_edm)));
                    if (sub_stat == "1") t->complete_subtask(std::stoi(sub_id));
                }
            }
            std::getline(file, line); // Consume [END_TASK]

            if (tid >= next_task_id) next_task_id = tid + 1;
            tasks_arr.push_back(std::move(t));
        }
    }
    file.close();
}