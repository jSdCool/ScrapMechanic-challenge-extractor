#include "registry.hpp"

#include <windows.h>
#include <iostream>
#include <string>
#include <tchar.h> // For generic text mappings
#include <locale>
#include <codecvt> // Required header

using namespace std;

std::string wstring_to_string(const std::wstring&);

/**Function to read a string value from the registry
 * @param hKeyRoot The root key (e.g., HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER)
 * @param subKey The subkey path (e.g., L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion")
 * @param valueName The name of the value to read (e.g., L"ProductName")
 * @return The read string value, or an empty string on failure
 */
string ReadRegistryStringValue(HKEY hKeyRoot, const string& subKey, const string& valueName) {
    HKEY hKey;
    // Open the registry key with read access
    LSTATUS lStatus = RegOpenKeyEx(
        hKeyRoot,              // Root key
        subKey.c_str(),    // Subkey path
        0,                     // Reserved
        KEY_READ,              // Access rights
        &hKey                  // Output handle to the open key
    );

    if (lStatus != ERROR_SUCCESS) {
        std::wcerr << L"Error opening registry key: " << lStatus << std::endl;
        return "";
    }

    // Determine the required buffer size for the value data
    DWORD dwType;
    DWORD dwDataSize = 0;
    lStatus = RegQueryValueEx(
        hKey,                  // Handle to the open key
        valueName.c_str(), // Value name
        nullptr,                  // Reserved
        &dwType,               // Output type of the data
        nullptr,                  // Output buffer (NULL to get size)
        &dwDataSize            // Output size of the buffer
    );

    if (lStatus != ERROR_SUCCESS) {
        std::wcerr << L"Error getting registry value size: " << lStatus << std::endl;
        RegCloseKey(hKey);
        return "";
    }

    if (dwType != REG_SZ && dwType != REG_EXPAND_SZ) {
        std::wcerr << L"Registry value is not a string type" << std::endl;
        RegCloseKey(hKey);
        return "";
    }

    // Allocate a buffer of the correct size
    std::string value;
    value.resize(dwDataSize / sizeof(char));

    // Retrieve the value data
    lStatus = RegQueryValueEx(
        hKey,                  // Handle to the open key
        valueName.c_str(), // Value name
        nullptr,                  // Reserved
        &dwType,               // Output type of the data
        reinterpret_cast<LPBYTE>(&value[0]), // Output buffer
        &dwDataSize            // Input/Output size of the buffer
    );

    // Close the registry key handle
    RegCloseKey(hKey);

    if (lStatus == ERROR_SUCCESS) {
        // Remove the null terminator from the end of the string
        if (!value.empty() && value.back() == L'\0') {
            value.pop_back();
        }
        // wcout << " RAW: "<<value <<" length: " << value.size() <<endl;
        return value;
    } else {
        std::wcerr << L"Error reading registry value: " << lStatus << std::endl;
        return "";
    }
}

// std::string wstring_to_string(const std::wstring& wstr) {
//     // Setup converter
//     using convert_type = std::codecvt_utf8<wchar_t>;
//     std::wstring_convert<convert_type, wchar_t> converter;
//
//     // Use converter (.to_bytes: wstr -> str, .from_bytes: str -> wstr)
//     return converter.to_bytes(wstr);
// }

// int main() {
//     std::wstring subKey = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
//     std::wstring valueName = L"ProductName";
//     std::wstring productName = ReadRegistryStringValue(HKEY_LOCAL_MACHINE, subKey, valueName);
//
//     if (!productName.empty()) {
//         std::wcout << L"Windows Product Name: " << productName << std::endl;
//     }
//
//     return 0;
// }

HKEY extractRootKey(const string &key, int &lengthOut) {
    // HKEY_CLASSES_ROOT
    // HKEY_CURRENT_USER
    // HKEY_LOCAL_MACHINE
    // HKEY_USERS
    // HKEY_CURRENT_CONFIG

    if (key.starts_with("HKEY_CLASSES_ROOT")) {
        lengthOut = 17;
        return HKEY_CLASSES_ROOT;
    }
    if (key.starts_with("HKEY_CURRENT_USER")) {
        lengthOut = 17;
        return HKEY_CURRENT_USER;
    }
    if (key.starts_with("HKEY_LOCAL_MACHINE")) {
        lengthOut = 18;
        return HKEY_LOCAL_MACHINE;
    }
    if (key.starts_with("HKEY_USERS")) {
        lengthOut = 10;
        return HKEY_USERS;
    }
    if (key.starts_with("HKEY_CURRENT_CONFIG")) {
        lengthOut = 19;
        return HKEY_CURRENT_CONFIG;
    }
    throw std::runtime_error("Error extracting root key, Unknown root key: "+key);
}

std::string readRegistryKey(const std::string &key) {
    int hkeyLength = 0;
    // ReSharper disable once CppLocalVariableMayBeConst
    HKEY hKey = extractRootKey(key, hkeyLength);
    //remove the root key from the key
    string subKey = key.substr(hkeyLength+1);
    //remove the last slash from the sub key to get the value to read
    const size_t indexOFLastSlash = subKey.find_last_of('\\');
    if (indexOFLastSlash == std::string::npos) {
        throw std::runtime_error("Error parsing registry key, provided values does not reference a value");
    }
    const string value = subKey.substr(indexOFLastSlash+1);
    subKey = subKey.substr(0, indexOFLastSlash);

    return ReadRegistryStringValue(hKey, subKey,value);
}