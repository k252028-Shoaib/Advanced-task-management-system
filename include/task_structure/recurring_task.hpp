#pragma once
#include "task.hpp"
#include <chrono>

class recurring_task : public task {
private:
    std::chrono::minutes recurrence_interval;
    int occurrences_left; // -1 means infinite recurrence
    date end_date;
    bool has_end_date;

    void calculate_next_occurrence();
    void reset_subtasks();

public:
    recurring_task(int id, 
                   std::string name,
                   date start = date(0,0,0,1,1,1970), 
                   date due = date(0,0,0,1,1,1970),
                   date reminder = date(0,0,0,1,1,1970), 
                   int priority = 0, 
                   std::string description = "", 
                   int category_id = 0,
                   std::chrono::minutes interval, 
                   int occurrences = -1, 
                   date end = date(0,0,0,1,1,1970));

    bool complete_task() override;

    // Getters for File I/O
    std::chrono::minutes get_interval() const { return recurrence_interval; }
    int get_occurrences() const { return occurrences_left; }
    date get_end_date() const { return end_date; }
    bool get_has_end() const { return has_end_date; }

    void set_interval(std::chrono::minutes new_interval);
    void set_occurrences(int count);
    void set_end_date(date end);
    void clear_end_date();
};