#pragma once

#include "graphql/resolver.h"
#include <memory>

class AdminResolvers {
public:
    static void registerResolvers();
    
    static ResolverResult resolveInternalUserSearch(const ResolverParams& params);
    static ResolverResult resolveInternalOrdersByDate(const ResolverParams& params);
    static ResolverResult resolveSystemStats(const ResolverParams& params);
    static ResolverResult resolveFetchExternalResource(const ResolverParams& params);
    static ResolverResult resolveValidateWebhookUrl(const ResolverParams& params);
    static ResolverResult resolveTestWebhook(const ResolverParams& params);
    static ResolverResult resolveValidateImportSource(const ResolverParams& params);
    static ResolverResult resolveFetchBookMetadata(const ResolverParams& params);
    static ResolverResult resolveDebugQuery(const ResolverParams& params);
    static ResolverResult resolveExportSchema(const ResolverParams& params);
    static ResolverResult resolveBulkUpdateUsers(const ResolverParams& params);
    static ResolverResult resolveExportUserData(const ResolverParams& params);
    static ResolverResult resolveImportUsers(const ResolverParams& params);
    static ResolverResult resolveUpdateInventory(const ResolverParams& params);
    
private:
    static std::string performSSRFRequest(const std::string& url, const RequestContext& ctx);
    static bool isSafeUrl(const std::string& url);
};