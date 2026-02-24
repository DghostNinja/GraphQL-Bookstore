#include "utils.h"
#include <string>
#include <stack>
#include <iostream>

std::string escapeJson(const std::string& input) {
    std::string output;
    output.reserve(input.size());
    for (char c : input) {
        switch (c) {
            case '"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default: output += c;
        }
    }
    return output;
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    if (output) {
        output->append(static_cast<char*>(contents), totalSize);
    }
    return totalSize;
}

bool isValidJson(const std::string& json) {
    if (json.empty()) return false;
    
    std::stack<char> brackets;
    bool inString = false;
    bool escaped = false;
    
    for (size_t i = 0; i < json.length(); i++) {
        char c = json[i];
        
        if (escaped) {
            escaped = false;
            continue;
        }
        
        if (c == '\\') {
            escaped = true;
            continue;
        }
        
        if (c == '"') {
            inString = !inString;
            continue;
        }
        
        if (inString) continue;
        
        switch (c) {
            case '{':
            case '[':
                brackets.push(c);
                break;
            case '}':
                if (brackets.empty() || brackets.top() != '{') return false;
                brackets.pop();
                break;
            case ']':
                if (brackets.empty() || brackets.top() != '[') return false;
                brackets.pop();
                break;
        }
    }
    
    return brackets.empty() && !inString;
}

std::string normalizeJson(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    
    bool inString = false;
    bool escaped = false;
    bool expectColon = false;
    bool expectValue = true;
    int braceLevel = 0;
    int bracketLevel = 0;
    
    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i];
        char next = (i + 1 < input.length()) ? input[i + 1] : '\0';
        char prev = (i > 0) ? input[i - 1] : '\0';
        
        if (escaped) {
            result += c;
            escaped = false;
            continue;
        }
        
        if (c == '\\') {
            result += c;
            escaped = true;
            continue;
        }
        
        if (c == '"') {
            result += c;
            inString = !inString;
            if (inString) expectValue = false;
            continue;
        }
        
        if (inString) {
            result += c;
            continue;
        }
        
        if (c == '{' || c == '[') {
            if (!result.empty() && result.back() != '{' && result.back() != '[' && 
                result.back() != ',' && result.back() != ':' && result.back() != ' ' &&
                result.back() != '\n' && result.back() != '\t') {
                return "";
            }
            result += c;
            if (c == '{') braceLevel++;
            else bracketLevel++;
            expectValue = true;
        }
        else if (c == '}' || c == ']') {
            result += c;
            if (c == '}') braceLevel--;
            else bracketLevel--;
            expectValue = false;
        }
        else if (c == ':') {
            result += ':';
            expectValue = true;
        }
        else if (c == ',') {
            result += ',';
            expectValue = true;
        }
        else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (!result.empty() && result.back() != ' ' && result.back() != '\t' &&
                result.back() != '\n' && result.back() != '\r' && result.back() != '{' &&
                result.back() != '[' && result.back() != ',' && result.back() != ':') {
                result += c;
            }
        }
        else if (isalpha(c) || c == '_' || c == '-' || (c >= '0' && c <= '9') || c == '.' || c == '+' || c == '-') {
            if (expectValue && !result.empty() && result.back() != '{' && result.back() != '[' && 
                result.back() != ',' && result.back() != ':') {
                return "";
            }
            result += c;
            expectValue = false;
        }
    }
    
    if (braceLevel != 0 || bracketLevel != 0 || inString) {
        return "";
    }
    
    return result;
}
