#include "gui/application.hpp"
#include <iostream>
#include <stdexcept>

int main() {
    try {
        // 1. Instantiate the Application
        // The constructor handles GLFW, ImGui, Fonts, and Data Loading
        Application app;

        // 2. Start the Main Loop
        // This will run until the user closes the window
        app.run();
    }
    catch (const std::exception& e) {
        // 3. Professional Error Handling
        // If initialization fails (e.g., assets missing), we catch it here.
        std::cerr << "CRITICAL ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}