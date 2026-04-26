#include "task_structure/subtask.hpp"

subtask::subtask(int id, std::string n, std::string desc,  std::chrono::minutes estimated_duration_minutes) 
    : subtask_id(id), subtask_name(n), subtask_description(desc), completed(false), 
    estimated_duration_minutes(estimated_duration_minutes), completed_at(0,0,0,1,1,1970) {}

int subtask::get_id() const{return subtask_id;}

void subtask::mark_complete() {
    completed = true;
    completed_at.update_date();
}

void subtask::mark_uncomplete() {
    completed = false;
    completed_at.convert_to_timepoint(0,0,0,1,1,1970);
}

bool subtask::is_complete() const{return completed;}

bool subtask::operator==(const subtask& other) const {
    return this->subtask_id == other.subtask_id;
}

time_t subtask::get_estimated_duration_minutes() const{
    auto tp = std::chrono::system_clock::time_point() + estimated_duration_minutes;
    return std::chrono::system_clock::to_time_t(tp);
}