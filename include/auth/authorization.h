#pragma once

#include <string>
#include <map>
#include <functional>
#include <memory>
#include "auth/jwt_handler.h"

struct RequestContext {
    std::string userId;
    std::string email;
    UserRole role;
    std::string ip;
    std::string userAgent;
    bool isAuthenticated;
};

class AuthorizationContext {
public:
    static AuthorizationContext& getInstance();
    
    void setJWTHandler(std::shared_ptr<JWTHandler> handler);
    
    RequestContext extractContext(const std::string& authToken);
    
    bool canAccessUser(const RequestContext& ctx, const std::string& targetUserId);
    bool canAccessOrder(const RequestContext& ctx, const std::string& orderUserId);
    bool canAccessCart(const RequestContext& ctx, const std::string& cartUserId);
    bool canModifyBook(const RequestContext& ctx);
    bool canModifyOrder(const RequestContext& ctx, const std::string& orderUserId);
    bool canAccessAdminEndpoints(const RequestContext& ctx);
    bool canAccessInternalEndpoints(const RequestContext& ctx);
    
    bool hasRole(const RequestContext& ctx, UserRole requiredRole);
    
    void logAccessAttempt(const RequestContext& ctx, const std::string& operation, bool authorized);

private:
    AuthorizationContext() = default;
    ~AuthorizationContext() = default;
    AuthorizationContext(const AuthorizationContext&) = delete;
    AuthorizationContext& operator=(const AuthorizationContext&) = delete;

    std::shared_ptr<JWTHandler> jwtHandler_;
};