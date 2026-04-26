#include "task_structure/recurring_task.hpp"
#include "task_structure/subtask.hpp"

recurring_task::recurring_task(int id, std::string name,date start, date due,date reminder, int priority, std::string description, 
                               int category_id ,std::chrono::minutes interval, int occurrences, date end)
    : task(id, name, start, due, reminder, priority, description, category_id),
      recurrence_interval(interval), occurrences_left(occurrences), end_date(end) 
{
    has_end_date = !(end == date(0,0,0,1,1,1970));
}

void recurring_task::reset_subtasks() {
    // Loop through and un-complete all subtasks for the next occurrence
    for (auto& sub : subtasks) {
        uncomplete_subtask(sub.get_id());
    }
}

void recurring_task::calculate_next_occurrence() {
    // Push all the active dates forward by the interval amount
    if (has_start_time) start_time.advance_by_minutes(recurrence_interval);
    if (has_due_time) due_time.advance_by_minutes(recurrence_interval);
    if (has_reminder_time) reminder_time.advance_by_minutes(recurrence_interval);
}

bool recurring_task::complete_task() {
    if (!check_all_subtasks_done()) return false;

    // 1. Are we tracking a specific number of occurrences?
    bool is_last_occurrence = false;
    if (occurrences_left > 0) {
        occurrences_left--;
        if (occurrences_left == 0) is_last_occurrence = true;
    }

    // 2. If it's the absolute last time, mark it fully complete forever.
    if (is_last_occurrence) {
        is_completed = true;
        completed_at.update_date();
        return true;
    }

    // 3. Otherwise, push the dates forward.
    calculate_next_occurrence();
    
    // 4. Did pushing the dates forward push it PAST the end date?
    if (has_end_date && end_date < due_time) {
        // It expired! Mark it fully complete.
        is_completed = true;
        completed_at.update_date();
        return true;
    }

    // 5. It successfully recurred. Reset subtasks and keep it active.
    reset_subtasks();
    return true; 
}

// --- SETTERS ---
void recurring_task::set_interval(std::chrono::minutes new_interval) {
    recurrence_interval = new_interval;
}

void recurring_task::set_occurrences(int count) {
    occurrences_left = count;
}

void recurring_task::set_end_date(date end) {
    end_date = end;
    has_end_date = true;
}

void recurring_task::clear_end_date() {
    has_end_date = false;
}