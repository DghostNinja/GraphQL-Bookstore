#pragma once

#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <jwt.h>

enum class UserRole {
    USER = 0,
    STAFF = 1,
    ADMIN = 2
};

struct UserClaims {
    std::string userId;
    std::string email;
    std::string role;
    int roleId;
    std::chrono::system_clock::time_point issuedAt;
    std::chrono::system_clock::time_point expiresAt;
};

class JWTHandler {
public:
    JWTHandler(const std::string& secret);
    ~JWTHandler();

    std::string generateToken(const std::string& userId, const std::string& email, UserRole role);
    std::string generateToken(const UserClaims& claims);
    
    bool verifyToken(const std::string& token, UserClaims& claims);
    bool validateToken(const std::string& token);
    
    std::string getUserIdFromToken(const std::string& token);
    UserRole getRoleFromToken(const std::string& token);
    
    void setSecret(const std::string& secret);
    void setExpirationSeconds(int seconds);

private:
    std::string secret_;
    int expirationSeconds_;
    
    std::string roleToString(UserRole role);
    UserRole stringToRole(const std::string& roleStr);
};