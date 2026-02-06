#pragma once

#include "graphql/resolver.h"
#include <memory>

class OrderResolvers {
public:
    static void registerResolvers();
    
    static ResolverResult resolveOrder(const ResolverParams& params);
    static ResolverResult resolveOrders(const ResolverParams& params);
    static ResolverResult resolveMyOrders(const ResolverParams& params);
    static ResolverResult resolveCreateOrder(const ResolverParams& params);
    static ResolverResult resolveCancelOrder(const ResolverParams& params);
    static ResolverResult resolveRequestRefund(const ResolverParams& params);
    static ResolverResult resolveUpdateOrderStatus(const ResolverParams& params);
    static ResolverResult resolveUpdateOrderAddress(const ResolverParams& params);
    static ResolverResult resolveInternalOrdersByDate(const ResolverParams& params);
    
private:
    static std::string getOrderById(const std::string& orderId, const RequestContext& ctx);
    static std::string getAllOrders(int limit, int offset, const std::string& statusFilter, 
                                     const std::string& userIdFilter, const RequestContext& ctx);
    static std::string getOrdersByDate(const std::string& startDate, const std::string& endDate, 
                                       const RequestContext& ctx);
};