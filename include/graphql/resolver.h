#pragma once

#include <string>
#include <map>
#include <functional>
#include <memory>
#include <vector>
#include "auth/authorization.h"

struct ResolverParams {
    std::map<std::string, std::string> arguments;
    RequestContext authContext;
    std::string query;
    std::string operationName;
};

struct ResolverResult {
    bool success;
    std::string data;
    std::string error;
    std::vector<std::string> errors;
    
    ResolverResult() : success(false) {}
    static ResolverResult successResult(const std::string& data) {
        ResolverResult r;
        r.success = true;
        r.data = data;
        return r;
    }
    static ResolverResult errorResult(const std::string& error) {
        ResolverResult r;
        r.success = false;
        r.error = error;
        r.errors.push_back(error);
        return r;
    }
};

class Resolver {
public:
    using ResolverFunc = std::function<ResolverResult(const ResolverParams&)>;
    
    Resolver(const std::string& name, ResolverFunc func);
    
    const std::string& getName() const;
    ResolverResult resolve(const ResolverParams& params) const;
    
    void setRequireAuth(bool require);
    void setRequiredRole(UserRole role);
    void setRequireOwnership(bool require);
    
    bool requiresAuth() const;
    UserRole getRequiredRole() const;
    bool requiresOwnership() const;

protected:
    std::string name_;
    ResolverFunc resolverFunc_;
    bool requireAuth_;
    UserRole requiredRole_;
    bool requireOwnership_;
    
    virtual bool checkAuthorization(const ResolverParams& params) const;
};

class QueryResolver : public Resolver {
public:
    QueryResolver(const std::string& name, ResolverFunc func);
};

class MutationResolver : public Resolver {
public:
    MutationResolver(const std::string& name, ResolverFunc func);
};

class SubscriptionResolver : public Resolver {
public:
    SubscriptionResolver(const std::string& name, ResolverFunc func);
};