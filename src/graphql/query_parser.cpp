#include "graphql/query_parser.h"
#include <sstream>
#include <algorithm>
#include <regex>

GraphQLOperation GraphQLQueryParser::parseQuery(const std::string& query) {
    GraphQLOperation operation;
    operation.type = "query";
    operation.name = "";
    
    std::string trimmedQuery = trim(query);
    
    size_t braceOpen = trimmedQuery.find('{');
    if (braceOpen == std::string::npos) {
        return operation;
    }
    
    size_t opTypeStart = trimmedQuery.find_first_not_of(" \t\n\r");
    size_t opTypeEnd = opTypeStart;
    
    if (opTypeStart != std::string::npos) {
        opTypeEnd = trimmedQuery.find_first_of("({ \t\n\r", opTypeStart);
        std::string opType = trimmedQuery.substr(opTypeStart, opTypeEnd - opTypeStart);
        
        if (opType == "mutation" || opType == "Mutation") {
            operation.type = "mutation";
        } else if (opType == "subscription" || opType == "Subscription") {
            operation.type = "subscription";
        }
    }
    
    size_t opNameEnd = trimmedQuery.find('(');
    if (opNameEnd == std::string::npos || opNameEnd > braceOpen) {
        opNameEnd = braceOpen;
    }
    
    if (opNameEnd > opTypeStart) {
        std::string potentialName = trimmedQuery.substr(opTypeEnd, opNameEnd - opTypeStart);
        potentialName = trim(potentialName);
        if (!potentialName.empty() && potentialName != "{") {
            operation.name = potentialName;
        }
    }
    
    std::string fieldsStr = trimmedQuery.substr(braceOpen + 1);
    
    size_t lastBracePos = fieldsStr.rfind('}');
    if (lastBracePos != std::string::npos) {
        fieldsStr = fieldsStr.substr(0, lastBracePos);
    }
    
    operation.fields = parseFields(fieldsStr);
    
    return operation;
}

bool GraphQLQueryParser::validateQuery(const GraphQLOperation& operation) {
    return !operation.fields.empty();
}

std::string GraphQLQueryParser::generateError(const std::string& message, const std::vector<int>& locations) {
    std::string error = "{\"errors\": [{\"message\": \"" + message + "\"";
    
    if (!locations.empty()) {
        error += ", \"locations\": [{\"line\": " + std::to_string(locations[0]) + "}]";
    }
    
    error += "}]}";
    return error;
}

std::vector<GraphQLField> GraphQLQueryParser::parseFields(const std::string& fieldsStr) {
    std::vector<GraphQLField> fields;
    
    std::istringstream stream(fieldsStr);
    std::string fieldStr;
    int depth = 0;
    std::string currentField;
    
    for (char c : fieldsStr) {
        if (c == '{') {
            depth++;
            currentField += c;
        } else if (c == '}') {
            depth--;
            currentField += c;
            
            if (depth == 0) {
                if (!trim(currentField).empty()) {
                    GraphQLField field = parseSingleField(currentField);
                    if (!field.name.empty()) {
                        fields.push_back(field);
                    }
                }
                currentField.clear();
            }
        } else if (c == ',' && depth == 0) {
            if (!trim(currentField).empty()) {
                GraphQLField field = parseSingleField(currentField);
                if (!field.name.empty()) {
                    fields.push_back(field);
                }
            }
            currentField.clear();
        } else {
            currentField += c;
        }
    }
    
    if (!trim(currentField).empty()) {
        GraphQLField field = parseSingleField(currentField);
        if (!field.name.empty()) {
            fields.push_back(field);
        }
    }
    
    return fields;
}

GraphQLField GraphQLQueryParser::parseSingleField(const std::string& fieldStr) {
    GraphQLField field;
    std::string trimmed = trim(fieldStr);
    
    if (trimmed.empty() || trimmed == "{}") {
        return field;
    }
    
    size_t colonPos = trimmed.find(':');
    std::string beforeColon = trimmed;
    std::string afterColon;
    
    if (colonPos != std::string::npos) {
        beforeColon = trimmed.substr(0, colonPos);
        afterColon = trimmed.substr(colonPos + 1);
    }
    
    beforeColon = trim(beforeColon);
    
    size_t aliasPos = beforeColon.find(':');
    if (aliasPos != std::string::npos) {
        field.alias = trim(beforeColon.substr(0, aliasPos));
        field.name = trim(beforeColon.substr(aliasPos + 1));
    } else {
        field.name = beforeColon;
    }
    
    size_t parenOpen = field.name.find('(');
    if (parenOpen != std::string::npos) {
        size_t parenClose = field.name.find(')', parenOpen);
        if (parenClose != std::string::npos) {
            std::string argsStr = field.name.substr(parenOpen + 1, parenClose - parenOpen - 1);
            field.arguments = parseArguments(argsStr);
            field.name = field.name.substr(0, parenOpen);
        }
    }
    
    afterColon = trim(afterColon);
    if (!afterColon.empty()) {
        size_t braceOpen = afterColon.find('{');
        if (braceOpen != std::string::npos && braceOpen == 0) {
            size_t lastBrace = afterColon.rfind('}');
            if (lastBrace != std::string::npos) {
                std::string subFieldsStr = afterColon.substr(1, lastBrace - 1);
                field.subFields = parseFields(subFieldsStr);
            }
        }
    }
    
    return field;
}

std::map<std::string, std::string> GraphQLQueryParser::parseArguments(const std::string& argsStr) {
    std::map<std::string, std::string> args;
    
    std::string trimmed = trim(argsStr);
    if (trimmed.empty()) {
        return args;
    }
    
    std::istringstream stream(trimmed);
    std::string arg;
    
    int depth = 0;
    std::string currentArg;
    
    for (char c : trimmed) {
        if (c == '{' || c == '[') {
            depth++;
            currentArg += c;
        } else if (c == '}' || c == ']') {
            depth--;
            currentArg += c;
            
            if (depth == 0) {
                processArgument(currentArg, args);
                currentArg.clear();
            }
        } else if (c == ',' && depth == 0) {
            if (!trim(currentArg).empty()) {
                processArgument(currentArg, args);
            }
            currentArg.clear();
        } else {
            currentArg += c;
        }
    }
    
    if (!trim(currentArg).empty()) {
        processArgument(currentArg, args);
    }
    
    return args;
}

void GraphQLQueryParser::processArgument(const std::string& argStr, std::map<std::string, std::string>& args) {
    std::string trimmed = trim(argStr);
    if (trimmed.empty()) {
        return;
    }
    
    size_t colonPos = trimmed.find(':');
    if (colonPos == std::string::npos) {
        return;
    }
    
    std::string key = trim(trimmed.substr(0, colonPos));
    std::string value = trim(trimmed.substr(colonPos + 1));
    
    if (value.length() >= 2 && 
        ((value[0] == '"' && value[value.length()-1] == '"') ||
         (value[0] == '\'' && value[value.length()-1] == '\''))) {
        value = value.substr(1, value.length() - 2);
    }
    
    if (!key.empty()) {
        args[key] = value;
    }
}

std::string GraphQLQueryParser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return "";
    }
    
    size_t last = str.find_last_not_of(" \t\n\r");
    if (last == std::string::npos) {
        return str.substr(first);
    }
    
    return str.substr(first, last - first + 1);
}