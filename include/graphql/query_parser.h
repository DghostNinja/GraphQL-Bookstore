#pragma once

#include <string>
#include <vector>
#include <map>

struct GraphQLField {
    std::string name;
    std::string alias;
    std::map<std::string, std::string> arguments;
    std::vector<GraphQLField> subFields;
};

struct GraphQLOperation {
    std::string type;
    std::string name;
    std::vector<GraphQLField> fields;
    std::map<std::string, std::string> variables;
};

class GraphQLQueryParser {
public:
    static GraphQLOperation parseQuery(const std::string& query);
    static bool validateQuery(const GraphQLOperation& operation);
    static std::string generateError(const std::string& message, const std::vector<int>& locations);
    
private:
    static size_t findOperationName(const std::string& query);
    static std::vector<GraphQLField> parseFields(const std::string& fieldsStr);
    static GraphQLField parseSingleField(const std::string& fieldStr);
    static std::map<std::string, std::string> parseArguments(const std::string& argsStr);
    static void processArgument(const std::string& argStr, std::map<std::string, std::string>& args);
    static std::string trim(const std::string& str);
};