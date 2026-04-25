#include "date/date.hpp"

using namespace std::chrono;

date::date() : tp(system_clock::now()) {}

date::date(int s, int m, int h, int d, int mon, int y) {
    convert_to_timepoint(s, m, h, d, mon, y);
}

void date::convert_to_timepoint(int s, int m, int h, int d, int mon, int y) {
    year_month_day ymd{year{y}, month{static_cast<unsigned>(mon)}, day{static_cast<unsigned>(d)}};
    auto days = sys_days{ymd};
    tp = days + hours{h} + minutes{m} + seconds{s};
}

date::Components date::convert_to_components() const {
    auto dp = floor<days>(tp);
    year_month_day ymd{dp};
    hh_mm_ss time{floor<seconds>(tp - dp)};

    return {
        static_cast<int>(ymd.year()),
        static_cast<int>(static_cast<unsigned>(ymd.month())),
        static_cast<int>(static_cast<unsigned>(ymd.day())),
        static_cast<int>(time.hours().count()),
        static_cast<int>(time.minutes().count()),
        static_cast<int>(time.seconds().count())
    };
}

void date::advance_by_minutes(std::chrono::minutes mins) {
    tp += mins;
}

void date::display_date() const {
    auto c = convert_to_components();
    std::cout << "Date: " << c.d << "/" << c.mon << "/" << c.y << "\n"
              << "Time: " << c.h << ":" << c.min << ":" << c.s << std::endl;
}

void date::update_date() {
    tp = system_clock::now();
}

time_t date::get_time_t() const {
    return system_clock::to_time_t(tp);
}

void date::from_time_t(time_t t) {
    tp = std::chrono::system_clock::from_time_t(t);
}

time_t date::get_time_elapsed(time_t t) {
    return system_clock::to_time_t(system_clock::now()) - t;
}

std::ostream& operator<<(std::ostream& os, const date& d) {
    auto c = d.convert_to_components();
    os << c.d << "/" << c.mon << "/" << c.y << " " << c.h << ":" << c.min;
    return os;
}

std::istream& operator>>(std::istream& is, date& d) {
    int day, mon, year;
    if (is >> day >> mon >> year) {
        d.convert_to_timepoint(0, 0, 0, day, mon, year);
    }
    return is;
}

bool date::operator<(const date& other) const {
    return tp < other.tp;
}

bool date::operator==(const date& other) const {
    // Compares full precision (second-to-second equality)
    return floor<seconds>(tp) == floor<seconds>(other.tp);
}