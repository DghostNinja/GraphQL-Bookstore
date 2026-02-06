#pragma once

#include <string>
#include <map>
#include <memory>
#include "utils/simple_json.h"

using json = SimpleJson::JsonValue;

class CartManager {
public:
    static CartManager& getInstance();
    
    std::string getCart(const std::string& userId);
    std::string addToCart(const std::string& userId, const std::string& bookId, int quantity);
    std::string removeFromCart(const std::string& userId, const std::string& bookId);
    std::string updateCartItem(const std::string& userId, const std::string& bookId, int quantity);
    std::string clearCart(const std::string& userId);
    std::string applyCoupon(const std::string& userId, const std::string& couponCode);
    std::string removeCoupon(const std::string& userId);
    
private:
    CartManager() = default;
    ~CartManager() = default;
    CartManager(const CartManager&) = delete;
    CartManager& operator=(const CartManager&) = delete;
    
    std::string calculateCartTotals(const std::string& cartId);
    std::string validateCoupon(const std::string& couponCode, double subtotal);
};