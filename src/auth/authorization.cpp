#include "auth/authorization.h"
#include <iostream>
#include <algorithm>

AuthorizationContext& AuthorizationContext::getInstance() {
    static AuthorizationContext instance;
    return instance;
}

void AuthorizationContext::setJWTHandler(std::shared_ptr<JWTHandler> handler) {
    jwtHandler_ = handler;
}

RequestContext AuthorizationContext::extractContext(const std::string& authToken) {
    RequestContext ctx;
    ctx.isAuthenticated = false;
    ctx.role = UserRole::USER;
    
    if (authToken.empty()) {
        return ctx;
    }
    
    try {
        UserClaims claims;
        if (jwtHandler_ && jwtHandler_->verifyToken(authToken, claims)) {
            ctx.userId = claims.userId;
            ctx.email = claims.email;
            ctx.role = static_cast<UserRole>(claims.roleId);
            ctx.isAuthenticated = true;
        }
    } catch (const std::exception& e) {
        ctx.isAuthenticated = false;
    }
    
    return ctx;
}

bool AuthorizationContext::canAccessUser(const RequestContext& ctx, const std::string& targetUserId) {
    if (!ctx.isAuthenticated) {
        return false;
    }
    
    if (ctx.role == UserRole::ADMIN) {
        return true;
    }
    
    if (ctx.role == UserRole::STAFF) {
        return true;
    }
    
    return ctx.userId == targetUserId;
}

bool AuthorizationContext::canAccessOrder(const RequestContext& ctx, const std::string& orderUserId) {
    if (!ctx.isAuthenticated) {
        return false;
    }
    
    if (ctx.role == UserRole::ADMIN) {
        return true;
    }
    
    if (ctx.role == UserRole::STAFF) {
        return true;
    }
    
    return ctx.userId == orderUserId;
}

bool AuthorizationContext::canAccessCart(const RequestContext& ctx, const std::string& cartUserId) {
    if (!ctx.isAuthenticated) {
        return false;
    }
    
    return ctx.userId == cartUserId;
}

bool AuthorizationContext::canModifyBook(const RequestContext& ctx) {
    if (!ctx.isAuthenticated) {
        return false;
    }
    
    return ctx.role == UserRole::ADMIN || ctx.role == UserRole::STAFF;
}

bool AuthorizationContext::canModifyOrder(const RequestContext& ctx, const std::string& orderUserId) {
    if (!ctx.isAuthenticated) {
        return false;
    }
    
    if (ctx.role == UserRole::ADMIN) {
        return true;
    }
    
    if (ctx.role == UserRole::STAFF) {
        return true;
    }
    
    return ctx.userId == orderUserId;
}

bool AuthorizationContext::canAccessAdminEndpoints(const RequestContext& ctx) {
    return ctx.role == UserRole::ADMIN;
}

bool AuthorizationContext::canAccessInternalEndpoints(const RequestContext& ctx) {
    if (ctx.role == UserRole::ADMIN) {
        return true;
    }
    
    if (ctx.role == UserRole::STAFF) {
        return true;
    }
    
    return false;
}

bool AuthorizationContext::hasRole(const RequestContext& ctx, UserRole requiredRole) {
    if (!ctx.isAuthenticated) {
        return false;
    }
    
    return ctx.role >= requiredRole;
}

void AuthorizationContext::logAccessAttempt(const RequestContext& ctx, const std::string& operation, bool authorized) {
    std::string status = authorized ? "ALLOWED" : "DENIED";
    std::cout << "[" << status << "] User: " << ctx.email 
              << " (" << static_cast<int>(ctx.role) << ") "
              << "Operation: " << operation << std::endl;
}