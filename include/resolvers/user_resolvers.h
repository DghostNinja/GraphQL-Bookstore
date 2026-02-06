#pragma once

#include "graphql/resolver.h"
#include <memory>

class UserResolvers {
public:
    static void registerResolvers();
    
    static ResolverResult resolveMe(const ResolverParams& params);
    static ResolverResult resolveUser(const ResolverParams& params);
    static ResolverResult resolveUsers(const ResolverParams& params);
    static ResolverResult resolveUpdateProfile(const ResolverParams& params);
    static ResolverResult resolveDeleteAccount(const ResolverParams& params);
    static ResolverResult resolveInternalUserSearch(const ResolverParams& params);
    
private:
    static std::string getUserById(const std::string& userId, const RequestContext& ctx);
    static std::string getAllUsers(int limit, int offset, const std::string& roleFilter, const RequestContext& ctx);
    static std::string searchUsersByEmail(const std::string& email, const RequestContext& ctx);
};