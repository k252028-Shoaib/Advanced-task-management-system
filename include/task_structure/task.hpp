#pragma once
#include <vector>
#include <string>
#include "date/date.hpp"

class subtask;

class task{
    protected:
        int task_id;
        std::string task_name;
        std::string task_description;
        int priority_level;

        date start_time;
        date reminder_time;
        date due_time;
        date completed_at;

        bool is_completed;
        bool has_start_time;
        bool has_due_time;
        bool has_reminder_time;

        int category_id;

        std::vector<subtask> subtasks;
 
        subtask* find_subtask(int id);
    public:
        enum class SubtaskResult {
            NOT_FOUND,
            COMPLETED,
            ALL_SUBTASKS_COMPLETED
        };

        task(int id, std::string name, date start = date(0,0,0,1,1,1970), date due = date(0,0,0,1,1,1970),
        date reminder = date(0,0,0,1,1,1970) ,int priority = 0, std::string description = "", int category_id = 0);

        bool add_subtask(std::string sub_name, std::string sub_desc, std::chrono::minutes estimated_duration_minutes);
        SubtaskResult complete_subtask(int id); 
        bool edit_subtask(int id, std::string new_name, std::string new_desc); 
        bool delete_subtask(int id); 
        bool uncomplete_subtask(int id);
        bool check_all_subtasks_done() const;
        void load_subtask(int id, std::string name, std::string desc, std::chrono::minutes estimated, bool is_comp);
        const std::vector<subtask>& get_subtasks() const { return subtasks; }

        virtual bool complete_task();
        void uncomplete_task(); 

        // Getters
        int get_id()const{return task_id;}
        std::string get_name() const {return task_name;}
        std::string get_description() const { return task_description; }

        int get_category_id()const{return category_id;}
        int get_priority() const{return priority_level;}
        int get_status() const{return is_completed;}
        
        date get_start_time() const { return start_time; }
        date get_due_time() const { return due_time; }
        date get_reminder_time() const { return reminder_time; }
        
        bool get_has_start() const { return has_start_time; }
        bool get_has_due() const { return has_due_time; }
        bool get_has_reminder() const { return has_reminder_time; }

        // --- Editing ---
        void set_name(std::string new_name);
        void set_description(std::string new_desc);
        void set_priority(int new_priority);
        void set_category(int new_cat_id);

        void set_start_date(date new_start);
        void clear_start_date();

        void set_due_date(date new_due);
        void clear_due_date();

        void set_reminder_date(date new_reminder);
        void clear_reminder_date();

        bool is_overdue();
        bool is_complete();

        virtual ~task() = default;
};