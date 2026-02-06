#include "graphql/resolver.h"
#include <iostream>

Resolver::Resolver(const std::string& name, ResolverFunc func)
    : name_(name), resolverFunc_(func), requireAuth_(false), requiredRole_(UserRole::USER), requireOwnership_(false) {
}

const std::string& Resolver::getName() const {
    return name_;
}

ResolverResult Resolver::resolve(const ResolverParams& params) const {
    try {
        if (requireAuth_ && !params.authContext.isAuthenticated) {
            return ResolverResult::errorResult("Authentication required");
        }
        
        if (requiredRole_ != UserRole::USER && 
            params.authContext.role < requiredRole_) {
            return ResolverResult::errorResult("Insufficient permissions");
        }
        
        if (!checkAuthorization(params)) {
            return ResolverResult::errorResult("Authorization failed");
        }
        
        return resolverFunc_(params);
    } catch (const std::exception& e) {
        return ResolverResult::errorResult(std::string("Resolver error: ") + e.what());
    }
}

void Resolver::setRequireAuth(bool require) {
    requireAuth_ = require;
}

void Resolver::setRequiredRole(UserRole role) {
    requiredRole_ = role;
}

void Resolver::setRequireOwnership(bool require) {
    requireOwnership_ = require;
}

bool Resolver::requiresAuth() const {
    return requireAuth_;
}

UserRole Resolver::getRequiredRole() const {
    return requiredRole_;
}

bool Resolver::requiresOwnership() const {
    return requireOwnership_;
}

bool Resolver::checkAuthorization(const ResolverParams& params) const {
    if (!requireOwnership_) {
        return true;
    }
    
    if (params.arguments.count("userId") == 0 &&
        params.arguments.count("id") == 0 &&
        params.arguments.count("orderId") == 0 &&
        params.arguments.count("cartId") == 0) {
        return true;
    }
    
    std::string targetId;
    if (params.arguments.count("userId")) {
        targetId = params.arguments.at("userId");
    } else if (params.arguments.count("id")) {
        targetId = params.arguments.at("id");
    } else if (params.arguments.count("orderId")) {
        targetId = params.arguments.at("orderId");
    } else if (params.arguments.count("cartId")) {
        targetId = params.arguments.at("cartId");
    }
    
    if (params.authContext.userId == targetId) {
        return true;
    }
    
    if (params.authContext.role >= UserRole::STAFF) {
        return true;
    }
    
    return false;
}

QueryResolver::QueryResolver(const std::string& name, ResolverFunc func)
    : Resolver(name, func) {
}

MutationResolver::MutationResolver(const std::string& name, ResolverFunc func)
    : Resolver(name, func) {
}

SubscriptionResolver::SubscriptionResolver(const std::string& name, ResolverFunc func)
    : Resolver(name, func) {
}