#include "messageBox.h"
#include <windows.h>

void showErrorMessageBox(const std::string& message, const std::string& title) {
    MessageBox(
        nullptr,
        message.c_str(), // Error Message
        title.c_str(),                                    // Window Title
        MB_OK | MB_ICONERROR                              // Buttons & Icon
    );
}

void showWarningMessageBox(const std::string& message, const std::string& title) {
    MessageBox(
        nullptr,
        message.c_str(), // Error Message
        title.c_str(),                                    // Window Title
        MB_OK | MB_ICONWARNING                            // Buttons & Icon
    );
}