#pragma once
#include <chrono>
#include <iostream>

class date {
private:
    std::chrono::system_clock::time_point tp;

    // Internal helper to get Y/M/D/H/M/S components
    struct Components { int y, mon, d, h, min, s; };
    Components convert_to_components() const;

public:
    date(); // Current time
    date(int s, int m, int h, int d, int mon, int y);

    void display_date() const;
    void update_date(); // Sets to system now
    void convert_to_timepoint(int s, int m, int h, int d, int mon, int y);
    void advance_by_minutes(std::chrono::minutes mins);
    
    time_t get_time_t() const;
    void from_time_t(time_t t);
    static time_t get_time_elapsed(time_t t);

    // Operators
    friend std::ostream& operator<<(std::ostream& os, const date& d);
    friend std::istream& operator>>(std::istream& is, date& d);
    bool operator<(const date& other) const;
    bool operator==(const date& other) const;
};