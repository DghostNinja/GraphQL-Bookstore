#include "resolvers/admin_resolvers.h"
#include "database/connection.h"
#include "auth/authorization.h"
#include "utils/webhook_manager.h"
#include "business/inventory_manager.h"
#include <nlohmann/json.hpp>
#include <chrono>
#include <regex>

using json = nlohmann::json;

std::map<std::string, QueryResolver> queryAdminResolvers;
std::map<std::string, MutationResolver> mutationAdminResolvers;

void AdminResolvers::registerResolvers() {
    queryAdminResolvers["_internalUserSearch"] = QueryResolver("_internalUserSearch", resolveInternalUserSearch);
    queryAdminResolvers["_internalUserSearch"].setRequireAuth(true);
    
    queryAdminResolvers["_internalOrdersByDate"] = QueryResolver("_internalOrdersByDate", resolveInternalOrdersByDate);
    queryAdminResolvers["_internalOrdersByDate"].setRequireAuth(true);
    
    queryAdminResolvers["_systemStats"] = QueryResolver("_systemStats", resolveSystemStats);
    queryAdminResolvers["_systemStats"].setRequireAuth(true);
    
    queryAdminResolvers["_fetchExternalResource"] = QueryResolver("_fetchExternalResource", resolveFetchExternalResource);
    queryAdminResolvers["_fetchExternalResource"].setRequireAuth(true);
    
    queryAdminResolvers["_validateWebhookUrl"] = QueryResolver("_validateWebhookUrl", resolveValidateWebhookUrl);
    queryAdminResolvers["_validateWebhookUrl"].setRequireAuth(true);
    
    mutationAdminResolvers["_testWebhook"] = MutationResolver("_testWebhook", resolveTestWebhook);
    mutationAdminResolvers["_testWebhook"].setRequireAuth(true);
    
    mutationAdminResolvers["_validateImportSource"] = MutationResolver("_validateImportSource", resolveValidateImportSource);
    mutationAdminResolvers["_validateImportSource"].setRequireAuth(true);
    
    mutationAdminResolvers["_fetchBookMetadata"] = MutationResolver("_fetchBookMetadata", resolveFetchBookMetadata);
    mutationAdminResolvers["_fetchBookMetadata"].setRequireAuth(true);
    
    mutationAdminResolvers["_debugQuery"] = MutationResolver("_debugQuery", resolveDebugQuery);
    mutationAdminResolvers["_debugQuery"].setRequireAuth(true);
    
    mutationAdminResolvers["_exportSchema"] = MutationResolver("_exportSchema", resolveExportSchema);
    mutationAdminResolvers["_exportSchema"].setRequireAuth(true);
    
    mutationAdminResolvers["_bulkUpdateUsers"] = MutationResolver("_bulkUpdateUsers", resolveBulkUpdateUsers);
    mutationAdminResolvers["_bulkUpdateUsers"].setRequireAuth(true);
    
    mutationAdminResolvers["_exportUserData"] = MutationResolver("_exportUserData", resolveExportUserData);
    mutationAdminResolvers["_exportUserData"].setRequireAuth(true);
    
    mutationAdminResolvers["_importUsers"] = MutationResolver("_importUsers", resolveImportUsers);
    mutationAdminResolvers["_importUsers"].setRequireAuth(true);
    
    mutationAdminResolvers["_updateInventory"] = MutationResolver("_updateInventory", resolveUpdateInventory);
    mutationAdminResolvers["_updateInventory"].setRequireAuth(true);
}

ResolverResult AdminResolvers::resolveInternalUserSearch(const ResolverParams& params) {
    std::string email = params.arguments.at("email");
    
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT id, email, first_name, last_name, role, is_active, "
                        "phone, address, city, state, zip_code, country, created_at, "
                        "password_hash "
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
        user["passwordHash"] = PQgetvalue(result, i, 13);
        
        users.push_back(user);
    }
    
    conn->clearResult(result);
    
    return ResolverResult::successResult(users.dump());
}

ResolverResult AdminResolvers::resolveInternalOrdersByDate(const ResolverParams& params) {
    std::string startDate = params.arguments.at("startDate");
    std::string endDate = params.arguments.at("endDate");
    
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT o.id, o.order_number, o.user_id, o.status, o.total_amount, "
                        "o.shipping_address, o.billing_address, o.notes, o.created_at, "
                        "o.payment_method, o.payment_status, "
                        "u.email, u.first_name, u.last_name, u.phone, u.address, u.password_hash "
                        "FROM orders o JOIN users u ON o.user_id = u.id "
                        "WHERE o.created_at >= $1 AND o.created_at <= $2 "
                        "ORDER BY o.created_at DESC";
    
    auto result = conn->executeQuery(query, {startDate, endDate});
    
    json orders = json::array();
    int numRows = PQntuples(result);
    
    for (int i = 0; i < numRows; i++) {
        json order;
        order["id"] = PQgetvalue(result, i, 0);
        order["orderNumber"] = PQgetvalue(result, i, 1);
        order["userId"] = PQgetvalue(result, i, 2);
        order["status"] = PQgetvalue(result, i, 3);
        order["totalAmount"] = std::stod(PQgetvalue(result, i, 4));
        order["shippingAddress"] = PQgetvalue(result, i, 5);
        order["billingAddress"] = PQgetvalue(result, i, 6);
        order["notes"] = PQgetvalue(result, i, 7);
        order["createdAt"] = PQgetvalue(result, i, 8);
        order["paymentMethod"] = PQgetvalue(result, i, 9);
        order["paymentStatus"] = PQgetvalue(result, i, 10);
        order["userEmail"] = PQgetvalue(result, i, 11);
        order["userFirstName"] = PQgetvalue(result, i, 12);
        order["userLastName"] = PQgetvalue(result, i, 13);
        order["userPhone"] = PQgetvalue(result, i, 14);
        order["userAddress"] = PQgetvalue(result, i, 15);
        order["userPasswordHash"] = PQgetvalue(result, i, 16);
        
        orders.push_back(order);
    }
    
    conn->clearResult(result);
    
    return ResolverResult::successResult(orders.dump());
}

ResolverResult AdminResolvers::resolveSystemStats(const ResolverParams& params) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    json stats;
    
    std::string userCountQuery = "SELECT COUNT(*) FROM users";
    auto userCountResult = conn->executeQuery(userCountQuery, {});
    stats["totalUsers"] = std::stoi(PQgetvalue(userCountResult, 0, 0));
    conn->clearResult(userCountResult);
    
    std::string orderCountQuery = "SELECT COUNT(*) FROM orders";
    auto orderCountResult = conn->executeQuery(orderCountQuery, {});
    stats["totalOrders"] = std::stoi(PQgetvalue(orderCountResult, 0, 0));
    conn->clearResult(orderCountResult);
    
    std::string revenueQuery = "SELECT COALESCE(SUM(total_amount), 0) FROM orders WHERE payment_status = 'completed'";
    auto revenueResult = conn->executeQuery(revenueQuery, {});
    stats["totalRevenue"] = std::stod(PQgetvalue(revenueResult, 0, 0));
    conn->clearResult(revenueResult);
    
    std::string cartQuery = "SELECT COUNT(*) FROM shopping_carts";
    auto cartResult = conn->executeQuery(cartQuery, {});
    stats["activeCarts"] = std::stoi(PQgetvalue(cartResult, 0, 0));
    conn->clearResult(cartResult);
    
    std::string lowStockQuery = "SELECT COUNT(*) FROM books WHERE stock_quantity <= low_stock_threshold";
    auto lowStockResult = conn->executeQuery(lowStockQuery, {});
    stats["lowStockBooks"] = std::stoi(PQgetvalue(lowStockResult, 0, 0));
    conn->clearResult(lowStockResult);
    
    return ResolverResult::successResult(stats.dump());
}

ResolverResult AdminResolvers::resolveFetchExternalResource(const ResolverParams& params) {
    std::string url = params.arguments.at("url");
    
    std::string response = performSSRFRequest(url, params.authContext);
    
    return ResolverResult::successResult(response);
}

ResolverResult AdminResolvers::resolveValidateWebhookUrl(const ResolverParams& params) {
    std::string url = params.arguments.at("url");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    std::string response = performSSRFRequest(url, params.authContext);
    auto endTime = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    json result;
    result["valid"] = true;
    result["statusCode"] = 200;
    result["responseTime"] = duration.count();
    result["responseBody"] = response;
    
    return ResolverResult::successResult(result.dump());
}

ResolverResult AdminResolvers::resolveTestWebhook(const ResolverParams& params) {
    std::string url = params.arguments.at("url");
    std::string eventType = params.arguments.count("eventType") ? params.arguments.at("eventType") : "test";
    std::string payload = params.arguments.count("payload") ? params.arguments.at("payload") : "{}";
    
    std::string response = WebhookManager::getInstance().sendWebhook(url, eventType, payload);
    
    json result = json.Parse(response);
    
    return ResolverResult::successResult(result.dump());
}

ResolverResult AdminResolvers::resolveValidateImportSource(const ResolverParams& params) {
    std::string url = params.arguments.at("url");
    
    std::string response = performSSRFRequest(url, params.authContext);
    
    json result;
    result["valid"] = true;
    result["statusCode"] = 200;
    result["responseBody"] = response;
    
    return ResolverResult::successResult(result.dump());
}

ResolverResult AdminResolvers::resolveFetchBookMetadata(const ResolverParams& params) {
    std::string isbn = params.arguments.at("isbn");
    std::string sourceUrl = params.arguments.count("sourceUrl") ? params.arguments.at("sourceUrl") : "";
    
    json metadata;
    metadata["isbn"] = isbn;
    
    if (!sourceUrl.empty()) {
        std::string response = performSSRFRequest(sourceUrl, params.authContext);
        metadata["rawData"] = response;
        metadata["source"] = sourceUrl;
    }
    
    return ResolverResult::successResult(metadata.dump());
}

ResolverResult AdminResolvers::resolveDebugQuery(const ResolverParams& params) {
    std::string query = params.arguments.at("query");
    
    json debugResult;
    debugResult["query"] = query;
    debugResult["executionTime"] = 42.5;
    debugResult["databaseQueries"] = {
        "SELECT * FROM users WHERE id = '" + params.authContext.userId + "'",
        "SELECT * FROM orders WHERE user_id = '" + params.authContext.userId + "'"
    };
    debugResult["variables"] = json.Parse("{}");
    debugResult["context"] = "User authenticated as " + params.authContext.email;
    
    return ResolverResult::successResult(debugResult.dump());
}

ResolverResult AdminResolvers::resolveExportSchema(const ResolverParams& params) {
    std::string format = params.arguments.count("format") ? params.arguments.at("format") : "json";
    
    json schema;
    schema["version"] = "1.0.0";
    schema["generatedAt"] = std::to_string(std::time(nullptr));
    schema["format"] = format;
    schema["schema"] = "Full GraphQL schema definition...";
    schema["internalEndpoints"] = {
        "_internalUserSearch",
        "_internalOrdersByDate",
        "_systemStats",
        "_fetchExternalResource",
        "_testWebhook",
        "_importUsers"
    };
    schema["hiddenMutations"] = {
        "_bulkUpdateUsers",
        "_exportUserData",
        "_updateInventory"
    };
    
    return ResolverResult::successResult(schema.dump());
}

ResolverResult AdminResolvers::resolveBulkUpdateUsers(const ResolverParams& params) {
    json input;
    try {
        input = json.Parse(params.arguments.at("input"));
    } catch (...) {
        return ResolverResult::errorResult("Invalid input");
    }
    
    json userIds = input["userIds"];
    json updates = input["updates"];
    
    auto conn = DatabasePool::getInstance().getConnection();
    
    int updatedCount = 0;
    std::vector<std::string> errors;
    
    for (const auto& userId : userIds) {
        try {
            std::string query = "UPDATE users SET ";
            std::vector<std::string> setClauses;
            std::vector<std::string> values;
            int paramCount = 1;
            
            if (updates.count("firstName")) {
                setClauses.push_back("first_name = $" + std::to_string(paramCount++));
                values.push_back(updates["firstName"]);
            }
            if (updates.count("lastName")) {
                setClauses.push_back("last_name = $" + std::to_string(paramCount++));
                values.push_back(updates["lastName"]);
            }
            if (updates.count("role")) {
                setClauses.push_back("role = $" + std::to_string(paramCount++));
                values.push_back(updates["role"]);
            }
            if (updates.count("isActive")) {
                setClauses.push_back("is_active = $" + std::to_string(paramCount++));
                values.push_back(updates["isActive"] ? "true" : "false");
            }
            
            if (!setClauses.empty()) {
                query += setClauses[0];
                for (size_t i = 1; i < setClauses.size(); i++) {
                    query += ", " + setClauses[i];
                }
                
                query += ", updated_at = NOW() WHERE id = $" + std::to_string(paramCount++);
                values.push_back(userId);
                
                conn->executeQuery(query, values);
                updatedCount++;
            }
        } catch (const std::exception& e) {
            errors.push_back("Failed to update user " + userId.get<std::string>() + ": " + e.what());
        }
    }
    
    json result;
    result["success"] = true;
    result["updatedCount"] = updatedCount;
    result["errors"] = errors;
    
    return ResolverResult::successResult(result.dump());
}

ResolverResult AdminResolvers::resolveExportUserData(const ResolverParams& params) {
    std::string format = params.arguments.count("format") ? params.arguments.at("format") : "json";
    
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT id, email, first_name, last_name, role, is_active, "
                        "phone, address, city, state, zip_code, country, created_at, "
                        "last_login, password_hash "
                        "FROM users ORDER BY created_at DESC";
    
    auto result = conn->executeQuery(query, {});
    
    json exportData = json::array();
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
        user["lastLogin"] = PQgetvalue(result, i, 13);
        user["passwordHash"] = PQgetvalue(result, i, 14);
        
        exportData.push_back(user);
    }
    
    conn->clearResult(result);
    
    if (format == "csv") {
        std::string csv = "id,email,firstName,lastName,role,isActive,phone,address,city,state,zipCode,country,createdAt,lastLogin,passwordHash\n";
        for (const auto& user : exportData) {
            csv += user["id"].get<std::string>() + ",";
            csv += user["email"].get<std::string>() + ",";
            csv += user["firstName"].get<std::string>() + ",";
            csv += user["lastName"].get<std::string>() + ",";
            csv += user["role"].get<std::string>() + ",";
            csv += user["isActive"].get<bool>() ? "true" : "false" + ",";
            csv += user["phone"].get<std::string>() + ",";
            csv += user["address"].get<std::string>() + ",";
            csv += user["city"].get<std::string>() + ",";
            csv += user["state"].get<std::string>() + ",";
            csv += user["zipCode"].get<std::string>() + ",";
            csv += user["country"].get<std::string>() + ",";
            csv += user["createdAt"].get<std::string>() + ",";
            csv += user["lastLogin"].get<std::string>() + ",";
            csv += user["passwordHash"].get<std::string>() + "\n";
        }
        
        return ResolverResult::successResult(csv);
    }
    
    return ResolverResult::successResult(exportData.dump());
}

ResolverResult AdminResolvers::resolveImportUsers(const ResolverParams& params) {
    std::string fileUrl = params.arguments.at("fileUrl");
    
    std::string userData = performSSRFRequest(fileUrl, params.authContext);
    
    auto conn = DatabasePool::getInstance().getConnection();
    
    json response;
    response["success"] = true;
    response["importedCount"] = 0;
    response["failedCount"] = 0;
    response["errors"] = json::array();
    
    try {
        json users = json.Parse(userData);
        
        for (const auto& user : users) {
            try {
                std::string query = "INSERT INTO users (email, password_hash, first_name, last_name, role, is_active) "
                                  "VALUES ($1, $2, $3, $4, $5, $6) "
                                  "ON CONFLICT (email) DO NOTHING";
                
                std::string passwordHash = "$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/LewY5GyY2aYjQFq.m";
                
                conn->executeQuery(query, {
                    user["email"],
                    passwordHash,
                    user["firstName"],
                    user["lastName"],
                    user.value("role", "user"),
                    user.value("isActive", true) ? "true" : "false"
                });
                
                response["importedCount"] = response["importedCount"].get<int>() + 1;
            } catch (const std::exception& e) {
                response["failedCount"] = response["failedCount"].get<int>() + 1;
                response["errors"].push_back("Failed to import user: " + std::string(e.what()));
            }
        }
    } catch (const std::exception& e) {
        response["success"] = false;
        response["errors"].push_back("Failed to parse user data: " + std::string(e.what()));
    }
    
    return ResolverResult::successResult(response.dump());
}

ResolverResult AdminResolvers::resolveUpdateInventory(const ResolverParams& params) {
    std::string bookId = params.arguments.at("bookId");
    int quantity = std::stoi(params.arguments.at("quantity"));
    
    InventoryManager::setStock(bookId, quantity, "Admin manual update");
    
    auto conn = DatabasePool::getInstance().getConnection();
    std::string query = "SELECT id, title, isbn, stock_quantity FROM books WHERE id = $1";
    auto result = conn->executeQuery(query, {bookId});
    
    if (PQntuples(result) == 0) {
        conn->clearResult(result);
        return ResolverResult::errorResult("Book not found");
    }
    
    json book;
    book["id"] = PQgetvalue(result, 0, 0);
    book["title"] = PQgetvalue(result, 0, 1);
    book["isbn"] = PQgetvalue(result, 0, 2);
    book["stockQuantity"] = std::stoi(PQgetvalue(result, 0, 3));
    
    conn->clearResult(result);
    
    return ResolverResult::successResult(book.dump());
}

std::string AdminResolvers::performSSRFRequest(const std::string& url, const RequestContext& ctx) {
    std::string sanitizedUrl = url;
    
    if (url.find("http://") != 0 && url.find("https://") != 0) {
        sanitizedUrl = "http://" + url;
    }
    
    return WebhookManager::getInstance().fetchExternalResource(sanitizedUrl);
}

bool AdminResolvers::isSafeUrl(const std::string& url) {
    std::regex internalPattern(R"(^(localhost|127\.0\.0\.1|0\.0\.0\.0|::1|192\.168\.|10\.|172\.(1[6-9]|2[0-9]|3[0-1])\.|169\.254\.\.\.)");
    
    if (std::regex_search(url, internalPattern)) {
        return false;
    }
    
    std::regex filePattern(R"(^file://)");
    if (std::regex_search(url, filePattern)) {
        return false;
    }
    
    return true;
}