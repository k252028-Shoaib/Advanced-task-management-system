#include "task_structure/task.hpp"
#include "task_structure/subtask.hpp"
#include <vector>
using namespace std;

task::task(int id, std::string name, date start, date due, date reminder, int priority, std::string description, int category_id)
: task_id(id), task_name(name), start_time(start), due_time(due), reminder_time(reminder), priority_level(priority), 
    task_description(description), is_completed(false), category_id(category_id)  
{
    has_start_time = !(start == date(0,0,0,1,1,1970));
    has_reminder_time = !(reminder == date(0,0,0,1,1,1970));
    has_due_time   = !(due == date(0,0,0,1,1,1970));
}

//SUBTASK
subtask* task::find_subtask(int id) {
    for (auto& sub : subtasks) {
        if (sub.get_id() == id) {
            return &sub; // Return memory address of the found subtask
        }
    }
    return nullptr; // Not found
}

bool task::add_subtask(std::string sub_name, std::string sub_desc, std::chrono::minutes estimated_duration_minutes){
    int new_id = subtasks.empty() ? 1 : subtasks.back().get_id() + 1;
    
    subtasks.push_back(subtask(new_id, sub_name, sub_desc, estimated_duration_minutes));
    return true;
}

task::SubtaskResult task::complete_subtask(int id) {
    subtask* sub = find_subtask(id);
    
    if (sub == nullptr) {
        return SubtaskResult::NOT_FOUND;
    }

    sub->mark_complete();

    if (check_all_subtasks_done()) {
        return SubtaskResult::ALL_SUBTASKS_COMPLETED; // GUI should prompt user!
    }
    
    return SubtaskResult::COMPLETED; // Normal completion
}

bool task::uncomplete_subtask(int id){
    subtask* sub = find_subtask(id);
    if (sub == nullptr) return false;
    
    sub->mark_uncomplete();
    return true;
}

bool task::edit_subtask(int id, std::string new_name, std::string new_desc){
    subtask* sub = find_subtask(id);
    if (sub == nullptr) return false;
    
    sub->subtask_name = new_name;
    sub->subtask_description = new_desc;
    return true;
}

bool task::delete_subtask(int id){
    subtask* sub = find_subtask(id);
    if (sub == nullptr) return false;

    std::erase(subtasks, *sub); 
    return true;
}

bool task::check_all_subtasks_done() const{
    if (subtasks.empty()) return true; // If there are no subtasks, they are technically "all done"
    
    for (const auto& sub : subtasks) {
        if (!sub.is_complete()) {
            return false; // Found one that isn't done
        }
    }
    return true;
}


//TASK:
bool task::complete_task(){
    if(!check_all_subtasks_done()) return false;
    is_completed = true;
    completed_at.update_date();
    return true;
}  

void task::uncomplete_task(){
    is_completed = false;
    completed_at.convert_to_timepoint(0,0,0,1,1,1970);
}

bool task::is_overdue(){
    if(has_due_time) return due_time < date();
    return false;
}

bool task::is_complete(){
    return is_completed;
}

// --- TASK EDITING  ---

void task::set_name(std::string new_name) {
    if (!new_name.empty()) {
        task_name = new_name;
    }
}

void task::set_description(std::string new_desc) {
    task_description = new_desc;
}

void task::set_priority(int new_priority) {
    priority_level = new_priority;
}

void task::set_category(int new_cat_id) {
    category_id = new_cat_id;
}

// --- DATE MANAGEMENT ---

void task::set_start_date(date new_start) {
    start_time = new_start;
    has_start_time = true;
}

void task::clear_start_date() {
    has_start_time = false;
}

void task::set_due_date(date new_due) {
    due_time = new_due;
    has_due_time = true;
}

void task::clear_due_date() {
    has_due_time = false;
}

void task::set_reminder_date(date new_reminder) {
    reminder_time = new_reminder;
    has_reminder_time = true;
}

void task::clear_reminder_date() {
    has_reminder_time = false;
}