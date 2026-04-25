#pragma once
#include <string>
#include "date/date.hpp"

class subtask;

class subtask{
    private:
        const int subtask_id;
        std::string subtask_name;
        std::string subtask_description;
        bool completed;
        date completed_at;
        std::chrono::minutes estimated_duration_minutes;
        subtask(int id, std::string name, std::string description = "", std::chrono::minutes estimated_duration_minutes = std::chrono::minutes(0));

    public:
        friend class task; 

        int get_id() const;
        void mark_complete();
        void mark_uncomplete();
        bool is_complete() const;

        std::string get_name() const { return subtask_name;}
        std::string get_description() const { return subtask_description; }
        time_t get_estimated_duration_minutes() const;
        bool operator==(const subtask& other) const;
};