#include "business/inventory_manager.h"
#include "database/connection.h"

bool InventoryManager::updateStock(const std::string& bookId, int quantityChange, const std::string& reason) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT stock_quantity FROM books WHERE id = $1";
    auto result = conn->executeQuery(query, {bookId});
    
    if (PQntuples(result) == 0) {
        conn->clearResult(result);
        return false;
    }
    
    int currentStock = std::stoi(PQgetvalue(result, 0, 0));
    conn->clearResult(result);
    
    int newStock = currentStock + quantityChange;
    
    if (newStock < 0) {
        newStock = 0;
    }
    
    std::string updateQuery = "UPDATE books SET stock_quantity = $1, updated_at = NOW() WHERE id = $2";
    conn->executeQuery(updateQuery, {std::to_string(newStock), bookId});
    
    return true;
}

bool InventoryManager::setStock(const std::string& bookId, int newQuantity, const std::string& reason) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "UPDATE books SET stock_quantity = $1, updated_at = NOW() WHERE id = $2";
    conn->executeQuery(query, {std::to_string(newQuantity), bookId});
    
    return true;
}

int InventoryManager::getStock(const std::string& bookId) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT stock_quantity FROM books WHERE id = $1";
    auto result = conn->executeQuery(query, {bookId});
    
    if (PQntuples(result) == 0) {
        conn->clearResult(result);
        return -1;
    }
    
    int stock = std::stoi(PQgetvalue(result, 0, 0));
    conn->clearResult(result);
    
    return stock;
}

bool InventoryManager::checkLowStock(const std::string& bookId) {
    auto conn = DatabasePool::getInstance().getConnection();
    
    std::string query = "SELECT stock_quantity, low_stock_threshold FROM books WHERE id = $1";
    auto result = conn->executeQuery(query, {bookId});
    
    if (PQntuples(result) == 0) {
        conn->clearResult(result);
        return false;
    }
    
    int stock = std::stoi(PQgetvalue(result, 0, 0));
    int threshold = std::stoi(PQgetvalue(result, 0, 1));
    conn->clearResult(result);
    
    return stock <= threshold;
}