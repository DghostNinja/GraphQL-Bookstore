#ifndef UTILS_H
#define UTILS_H

#include <string>

// Function to escape JSON strings
std::string escapeJson(const std::string& input);

// Callback for CURL write function
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);

// JSON validation
bool isValidJson(const std::string& json);
std::string normalizeJson(const std::string& input);

#endif // UTILS_H
