#include "business/payment_processor.h"
#include "database/connection.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

using json = nlohmann::json;

std::string PaymentProcessor::processPayment(const std::string& orderId, const std::string& userId, 
                                              double amount, const std::string& method, 
                                              const std::string& cardNumber, const std::string& cardExpiry,
                                              const std::string& cardCvv, const std::string& cardholderName) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string transactionId = "TXN-" + std::to_string(std::time(nullptr)) + "-" + orderId;
    
    std::string lastFour = cardNumber.length() >= 4 ? cardNumber.substr(cardNumber.length() - 4) : "****";
    std::string cardType = "Unknown";
    
    if (cardNumber.starts_with("4")) {
        cardType = "Visa";
    } else if (cardNumber.starts_with("5")) {
        cardType = "MasterCard";
    } else if (cardNumber.starts_with("3")) {
        cardType = "American Express";
    }
    
    std::string insertQuery = "INSERT INTO payment_transactions (order_id, user_id, amount, currency, "
                             "payment_method, status, transaction_id, gateway_response) "
                             "VALUES ($1, $2, $3, 'USD', $4, 'completed', $5, $6) "
                             "RETURNING id";
    
    std::string gatewayResponse = "{\"status\": \"success\", \"message\": \"Payment processed\", "
                                 "\"card_type\": \"" + cardType + "\", \"last_four\": \"" + lastFour + "\"}";
    
    auto result = conn->executeQuery(insertQuery, {
        orderId,
        userId,
        std::to_string(amount),
        method,
        transactionId,
        gatewayResponse
    });
    
    std::string paymentId = PQgetvalue(result, 0, 0);
    conn->clearResult(result);
    
    std::string updateOrder = "UPDATE orders SET payment_status = 'completed', status = 'processing' WHERE id = $1";
    conn->executeQuery(updateOrder, {orderId});
    
    json response;
    response["id"] = paymentId;
    response["order"]["id"] = orderId;
    response["user"]["id"] = userId;
    response["amount"] = amount;
    response["currency"] = "USD";
    response["paymentMethod"] = method;
    response["status"] = "completed";
    response["transactionId"] = transactionId;
    response["lastFourDigits"] = lastFour;
    response["cardType"] = cardType;
    response["createdAt"] = std::to_string(std::time(nullptr));
    response["processedAt"] = std::to_string(std::time(nullptr));
    
    return response.dump();
}

std::string PaymentProcessor::refundPayment(const std::string& paymentId, double amount, const std::string& reason) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT order_id, user_id, amount FROM payment_transactions WHERE id = $1";
    auto result = conn->executeQuery(query, {paymentId});
    
    if (PQntuples(result) == 0) {
        conn->clearResult(result);
        json error;
        error["success"] = false;
        error["message"] = "Payment not found";
        return error.dump();
    }
    
    std::string orderId = PQgetvalue(result, 0, 0);
    std::string userId = PQgetvalue(result, 0, 1);
    double originalAmount = std::stod(PQgetvalue(result, 0, 2));
    conn->clearResult(result);
    
    if (amount <= 0) {
        amount = originalAmount;
    }
    
    std::string refundId = "REF-" + std::to_string(std::time(nullptr)) + "-" + paymentId;
    
    std::string insertRefund = "INSERT INTO payment_transactions (order_id, user_id, amount, currency, "
                              "payment_method, status, transaction_id, gateway_response) "
                              "VALUES ($1, $2, $3, 'USD', 'refund', 'completed', $4, $5) "
                              "RETURNING id";
    
    std::string response = "{\"status\": \"refunded\", \"reason\": \"" + reason + "\"}";
    
    auto refundResult = conn->executeQuery(insertRefund, {
        orderId,
        userId,
        std::to_string(amount),
        refundId,
        response
    });
    
    std::string newTransactionId = PQgetvalue(refundResult, 0, 0);
    conn->clearResult(refundResult);
    
    json refundResponse;
    refundResponse["id"] = newTransactionId;
    refundResponse["order"]["id"] = orderId;
    refundResponse["user"]["id"] = userId;
    refundResponse["amount"] = amount;
    refundResponse["status"] = "refunded";
    refundResponse["reason"] = reason;
    
    return refundResponse.dump();
}

std::string PaymentProcessor::getPaymentStatus(const std::string& paymentId) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT id, order_id, user_id, amount, payment_method, status, "
                       "transaction_id, gateway_response, created_at, processed_at "
                       "FROM payment_transactions WHERE id = $1";
    
    auto result = conn->executeQuery(query, {paymentId});
    
    if (PQntuples(result) == 0) {
        conn->clearResult(result);
        json error;
        error["success"] = false;
        error["message"] = "Payment not found";
        return error.dump();
    }
    
    json payment;
    payment["id"] = PQgetvalue(result, 0, 0);
    payment["order"]["id"] = PQgetvalue(result, 0, 1);
    payment["user"]["id"] = PQgetvalue(result, 0, 2);
    payment["amount"] = std::stod(PQgetvalue(result, 0, 3));
    payment["paymentMethod"] = PQgetvalue(result, 0, 4);
    payment["status"] = PQgetvalue(result, 0, 5);
    payment["transactionId"] = PQgetvalue(result, 0, 6);
    payment["gatewayResponse"] = PQgetvalue(result, 0, 7);
    payment["createdAt"] = PQgetvalue(result, 0, 8);
    payment["processedAt"] = PQgetvalue(result, 0, 9);
    
    conn->clearResult(result);
    
    return payment.dump();
}