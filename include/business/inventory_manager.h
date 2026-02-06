#pragma once

#include <string>

class InventoryManager {
public:
    static bool updateStock(const std::string& bookId, int quantityChange, const std::string& reason);
    static bool setStock(const std::string& bookId, int newQuantity, const std::string& reason);
    static int getStock(const std::string& bookId);
    static bool checkLowStock(const std::string& bookId);
};