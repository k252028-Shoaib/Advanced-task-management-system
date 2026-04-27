#pragma once
#include <vector>
#include <memory> // For std::unique_ptr
#include "task_structure/task.hpp"
#include "task_structure/recurring_task.hpp"
#include "task_structure/category.hpp"

struct task_members {
        std::string name;
        std::string description = "";
        date start = date(0,0,0,1,1,1970);
        date due = date(0,0,0,1,1,1970);
        date reminder = date(0,0,0,1,1,1970);
        int priority = 0;
        int category_id = 0;
};

struct recurring_task_members : public task_members {
    std::chrono::minutes interval;
    int occurrences = -1;
    date end_date = date(0,0,0,1,1,1970);
};

class data_management {
private:
    // The Master Lists
    std::vector<std::unique_ptr<task>> tasks_arr;
    std::vector<category> categories_arr;

    // Internal ID counters
    int next_task_id = 0;
    int next_category_id = 1;

    // Private helper for file I/O
    std::string get_save_path();

public:
    data_management();
    ~data_management();

    // --- Task Management ---
    // We return a raw pointer so the GUI can view/edit, 
    // but the unique_ptr keeps ownership.
    task* add_task(const task_members& req);
    
    task* add_recurring_task(const recurring_task_members& req);
                             
    bool delete_task(int id);
    task* find_task_by_id(int id);

    // --- Category Management ---
    void add_category(std::string name, std::string color);
    void delete_category(int id);
    const std::vector<category>& get_categories() const { return categories_arr; }

    void save_to_file();
    void load_from_file();
    
    // Get the whole list for the GUI to display
    const std::vector<std::unique_ptr<task>>& get_all_tasks() const { return tasks_arr; }
};