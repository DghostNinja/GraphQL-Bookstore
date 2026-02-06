#include "resolvers/user_resolvers.h"
#include "database/connection.h"
#include "auth/authorization.h"
#include "business/cart_manager.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

std::map<std::string, QueryResolver> queryUserResolvers;
std::map<std::string, MutationResolver> mutationUserResolvers;

void UserResolvers::registerResolvers() {
    queryUserResolvers["me"] = QueryResolver("me", resolveMe);
    queryUserResolvers["me"].setRequireAuth(true);
    
    queryUserResolvers["user"] = QueryResolver("user", resolveUser);
    queryUserResolvers["user"].setRequireAuth(true);
    queryUserResolvers["user"].setRequireOwnership(true);
    
    queryUserResolvers["users"] = QueryResolver("users", resolveUsers);
    queryUserResolvers["users"].setRequireAuth(true);
    queryUserResolvers["users"].setRequiredRole(UserRole::STAFF);
    
    queryUserResolvers["_internalUserSearch"] = QueryResolver("_internalUserSearch", resolveInternalUserSearch);
    queryUserResolvers["_internalUserSearch"].setRequireAuth(true);
    
    mutationUserResolvers["updateProfile"] = MutationResolver("updateProfile", resolveUpdateProfile);
    mutationUserResolvers["updateProfile"].setRequireAuth(true);
    mutationUserResolvers["updateProfile"].setRequireOwnership(true);
    
    mutationUserResolvers["deleteAccount"] = MutationResolver("deleteAccount", resolveDeleteAccount);
    mutationUserResolvers["deleteAccount"].setRequireAuth(true);
    mutationUserResolvers["deleteAccount"].setRequireOwnership(true);
}

ResolverResult UserResolvers::resolveMe(const ResolverParams& params) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT id, email, first_name, last_name, role, is_active, "
                        "phone, address, city, state, zip_code, country, created_at, last_login "
                        "FROM users WHERE id = $1";
    
    auto result = conn->executeQuery(query, {params.authContext.userId});
    
    if (PQntuples(result) == 0) {
        conn->clearResult(result);
        return ResolverResult::errorResult("User not found");
    }
    
    json user;
    user["id"] = PQgetvalue(result, 0, 0);
    user["email"] = PQgetvalue(result, 0, 1);
    user["firstName"] = PQgetvalue(result, 0, 2);
    user["lastName"] = PQgetvalue(result, 0, 3);
    user["role"] = PQgetvalue(result, 0, 4);
    user["isActive"] = std::string(PQgetvalue(result, 0, 5)) == "t";
    user["phone"] = PQgetvalue(result, 0, 6);
    user["address"] = PQgetvalue(result, 0, 7);
    user["city"] = PQgetvalue(result, 0, 8);
    user["state"] = PQgetvalue(result, 0, 9);
    user["zipCode"] = PQgetvalue(result, 0, 10);
    user["country"] = PQgetvalue(result, 0, 11);
    user["createdAt"] = PQgetvalue(result, 0, 12);
    user["lastLogin"] = PQgetvalue(result, 0, 13);
    
    conn->clearResult(result);
    
    return ResolverResult::successResult(user.dump());
}

ResolverResult UserResolvers::resolveUser(const ResolverParams& params) {
    std::string userId = params.arguments.at("id");
    
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT id, email, first_name, last_name, role, is_active, "
                        "phone, address, city, state, zip_code, country, created_at "
                        "FROM users WHERE id = $1";
    
    auto result = conn->executeQuery(query, {userId});
    
    if (PQntuples(result) == 0) {
        conn->clearResult(result);
        return ResolverResult::errorResult("User not found");
    }
    
    json user;
    user["id"] = PQgetvalue(result, 0, 0);
    user["email"] = PQgetvalue(result, 0, 1);
    user["firstName"] = PQgetvalue(result, 0, 2);
    user["lastName"] = PQgetvalue(result, 0, 3);
    user["role"] = PQgetvalue(result, 0, 4);
    user["isActive"] = std::string(PQgetvalue(result, 0, 5)) == "t";
    
    conn->clearResult(result);
    
    return ResolverResult::successResult(user.dump());
}

ResolverResult UserResolvers::resolveUsers(const ResolverParams& params) {
    int limit = 20;
    int offset = 0;
    std::string roleFilter;
    
    if (params.arguments.count("limit")) {
        limit = std::stoi(params.arguments.at("limit"));
    }
    if (params.arguments.count("offset")) {
        offset = std::stoi(params.arguments.at("offset"));
    }
    if (params.arguments.count("role")) {
        roleFilter = params.arguments.at("role");
    }
    
    std::string data = getAllUsers(limit, offset, roleFilter, params.authContext);
    
    return ResolverResult::successResult(data);
}

ResolverResult UserResolvers::resolveUpdateProfile(const ResolverParams& params) {
    json input;
    try {
        input = json.Parse(params.arguments.at("input"));
    } catch (...) {
        return ResolverResult::errorResult("Invalid input");
    }
    
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "UPDATE users SET ";
    std::vector<std::string> updates;
    std::vector<std::string> values;
    int paramCount = 1;
    
    if (input.count("firstName")) {
        updates.push_back("first_name = $" + std::to_string(paramCount++));
        values.push_back(input["firstName"]);
    }
    if (input.count("lastName")) {
        updates.push_back("last_name = $" + std::to_string(paramCount++));
        values.push_back(input["lastName"]);
    }
    if (input.count("phone")) {
        updates.push_back("phone = $" + std::to_string(paramCount++));
        values.push_back(input["phone"]);
    }
    if (input.count("address")) {
        updates.push_back("address = $" + std::to_string(paramCount++));
        values.push_back(input["address"]);
    }
    if (input.count("city")) {
        updates.push_back("city = $" + std::to_string(paramCount++));
        values.push_back(input["city"]);
    }
    if (input.count("state")) {
        updates.push_back("state = $" + std::to_string(paramCount++));
        values.push_back(input["state"]);
    }
    if (input.count("zipCode")) {
        updates.push_back("zip_code = $" + std::to_string(paramCount++));
        values.push_back(input["zipCode"]);
    }
    
    query += "updated_at = NOW() ";
    
    for (const auto& update : updates) {
        query += ", " + update;
    }
    
    query += " WHERE id = $" + std::to_string(paramCount++);
    values.push_back(params.authContext.userId);
    
    conn->executeQuery(query, values);
    
    return resolveMe(params);
}

ResolverResult UserResolvers::resolveDeleteAccount(const ResolverParams& params) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "DELETE FROM users WHERE id = $1";
    conn->executeQuery(query, {params.authContext.userId});
    
    json result;
    result["success"] = true;
    
    return ResolverResult::successResult(result.dump());
}

ResolverResult UserResolvers::resolveInternalUserSearch(const ResolverParams& params) {
    std::string email = params.arguments.at("email");
    
    std::string data = searchUsersByEmail(email, params.authContext);
    
    return ResolverResult::successResult(data);
}

std::string UserResolvers::getUserById(const std::string& userId, const RequestContext& ctx) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT id, email, first_name, last_name, role, is_active, "
                        "phone, address, city, state, zip_code, country, created_at "
                        "FROM users WHERE id = $1";
    
    auto result = conn->executeQuery(query, {userId});
    
    if (PQntuples(result) == 0) {
        conn->clearResult(result);
        return "null";
    }
    
    json user;
    user["id"] = PQgetvalue(result, 0, 0);
    user["email"] = PQgetvalue(result, 0, 1);
    user["firstName"] = PQgetvalue(result, 0, 2);
    user["lastName"] = PQgetvalue(result, 0, 3);
    user["role"] = PQgetvalue(result, 0, 4);
    user["isActive"] = std::string(PQgetvalue(result, 0, 5)) == "t";
    user["phone"] = PQgetvalue(result, 0, 6);
    user["address"] = PQgetvalue(result, 0, 7);
    user["city"] = PQgetvalue(result, 0, 8);
    user["state"] = PQgetvalue(result, 0, 9);
    user["zipCode"] = PQgetvalue(result, 0, 10);
    user["country"] = PQgetvalue(result, 0, 11);
    user["createdAt"] = PQgetvalue(result, 0, 12);
    
    conn->clearResult(result);
    
    return user.dump();
}

std::string UserResolvers::getAllUsers(int limit, int offset, const std::string& roleFilter, const RequestContext& ctx) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT id, email, first_name, last_name, role, is_active, "
                        "phone, address, city, state, zip_code, country, created_at "
                        "FROM users ";
    
    if (!roleFilter.empty()) {
        query += "WHERE role = '" + roleFilter + "' ";
    }
    
    query += "ORDER BY created_at DESC LIMIT $" + std::to_string(1) + 
             " OFFSET $" + std::to_string(2);
    
    auto result = conn->executeQuery(query, {std::to_string(limit), std::to_string(offset)});
    
    json users = json::array();
    int numRows = PQntuples(result);
    
    for (int i = 0; i < numRows; i++) {
        json user;
        user["id"] = PQgetvalue(result, i, 0);
        user["email"] = PQgetvalue(result, i, 1);
        user["firstName"] = PQgetvalue(result, i, 2);
        user["lastName"] = PQgetvalue(result, i, 3);
        user["role"] = PQgetvalue(result, i, 4);
        user["isActive"] = std::string(PQgetvalue(result, i, 5)) == "t";
        user["phone"] = PQgetvalue(result, i, 6);
        user["address"] = PQgetvalue(result, i, 7);
        user["city"] = PQgetvalue(result, i, 8);
        user["state"] = PQgetvalue(result, i, 9);
        user["zipCode"] = PQgetvalue(result, i, 10);
        user["country"] = PQgetvalue(result, i, 11);
        user["createdAt"] = PQgetvalue(result, i, 12);
        
        users.push_back(user);
    }
    
    conn->clearResult(result);
    
    return users.dump();
}

std::string UserResolvers::searchUsersByEmail(const std::string& email, const RequestContext& ctx) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT id, email, first_name, last_name, role, is_active, "
                        "phone, address, city, state, zip_code, country, created_at "
                        "FROM users WHERE email ILIKE $1";
    
    auto result = conn->executeQuery(query, {"%" + email + "%"});
    
    json users = json::array();
    int numRows = PQntuples(result);
    
    for (int i = 0; i < numRows; i++) {
        json user;
        user["id"] = PQgetvalue(result, i, 0);
        user["email"] = PQgetvalue(result, i, 1);
        user["firstName"] = PQgetvalue(result, i, 2);
        user["lastName"] = PQgetvalue(result, i, 3);
        user["role"] = PQgetvalue(result, i, 4);
        user["isActive"] = std::string(PQgetvalue(result, i, 5)) == "t";
        user["phone"] = PQgetvalue(result, i, 6);
        user["address"] = PQgetvalue(result, i, 7);
        user["city"] = PQgetvalue(result, i, 8);
        user["state"] = PQgetvalue(result, i, 9);
        user["zipCode"] = PQgetvalue(result, i, 10);
        user["country"] = PQgetvalue(result, i, 11);
        user["createdAt"] = PQgetvalue(result, i, 12);
        
        users.push_back(user);
    }
    
    conn->clearResult(result);
    
    return users.dump();
}