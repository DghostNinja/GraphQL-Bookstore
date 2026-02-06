#pragma once

#include <string>

class PaymentProcessor {
public:
    static std::string processPayment(const std::string& orderId, const std::string& userId, 
                                     double amount, const std::string& method, 
                                     const std::string& cardNumber, const std::string& cardExpiry,
                                     const std::string& cardCvv, const std::string& cardholderName);
    static std::string refundPayment(const std::string& paymentId, double amount, const std::string& reason);
    static std::string getPaymentStatus(const std::string& paymentId);
};