#pragma once
#include <string>

class category {
public:
    int id;
    std::string name;
    std::string hex_color; // e.g., "#FF5733" for the GUI to use

    category(int id, std::string name, std::string color = "#FFFFFF")
        : id(id), name(name), hex_color(color) {}
};