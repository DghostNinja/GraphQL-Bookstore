#include "auth/jwt_handler.h"
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <iostream>

JWTHandler::JWTHandler(const std::string& secret)
    : secret_(secret), expirationSeconds_(86400) {
}

JWTHandler::~JWTHandler() {
}

std::string JWTHandler::generateToken(const std::string& userId, const std::string& email, UserRole role) {
    UserClaims claims;
    claims.userId = userId;
    claims.email = email;
    claims.role = roleToString(role);
    claims.roleId = static_cast<int>(role);
    claims.issuedAt = std::chrono::system_clock::now();
    claims.expiresAt = claims.issuedAt + std::chrono::seconds(expirationSeconds_);
    
    return generateToken(claims);
}

 std::string JWTHandler::generateToken(const UserClaims& claims) {
    jwt_t* jwt = nullptr;
    int ret = jwt_new(&jwt);
    
    if (ret != 0 || jwt == nullptr) {
        throw std::runtime_error("Failed to create JWT");
    }
    
    ret = jwt_set_alg(jwt, JWT_ALG_HS256, (const unsigned char*)secret_.c_str(), secret_.length());
    if (ret != 0) {
        jwt_free(jwt);
        throw std::runtime_error("Failed to set JWT algorithm");
    }
    
    jwt_add_grant(jwt, "sub", claims.userId.c_str());
    jwt_add_grant(jwt, "email", claims.email.c_str());
    jwt_add_grant(jwt, "role", claims.role.c_str());
    jwt_add_grant_int(jwt, "role_id", claims.roleId);
    
    auto iat = std::chrono::system_clock::to_time_t(claims.issuedAt);
    auto exp = std::chrono::system_clock::to_time_t(claims.expiresAt);
    
    jwt_add_grant_int(jwt, "iat", static_cast<long>(iat));
    jwt_add_grant_int(jwt, "exp", static_cast<long>(exp));
    
    char* token = jwt_encode_str(jwt);
    std::string tokenStr(token);
    
    jwt_free_str(token);
    jwt_free(jwt);
    
    return tokenStr;
}

bool JWTHandler::verifyToken(const std::string& token, UserClaims& claims) {
    jwt_t* jwt = nullptr;
    int ret = jwt_decode_2(&jwt, token.c_str(), nullptr);
    
    if (ret != 0 || jwt == nullptr) {
        return false;
    }
    
    time_t now = time(nullptr);
    time_t exp = jwt_get_grant_int(jwt, "exp");
    
    if (now >= exp) {
        jwt_free(jwt);
        return false;
    }
    
    const char* sub = jwt_get_grant(jwt, "sub");
    const char* email = jwt_get_grant(jwt, "email");
    const char* role = jwt_get_grant(jwt, "role");
    int roleId = jwt_get_grant_int(jwt, "role_id");
    
    claims.userId = sub ? sub : "";
    claims.email = email ? email : "";
    claims.role = role ? role : "";
    claims.roleId = roleId;
    
    jwt_free(jwt);
    return true;
}

bool JWTHandler::validateToken(const std::string& token) {
    UserClaims claims;
    return verifyToken(token, claims);
}

std::string JWTHandler::getUserIdFromToken(const std::string& token) {
    jwt_t* jwt = nullptr;
    int ret = jwt_decode_2(&jwt, token.c_str(), nullptr);
    
    if (ret != 0 || jwt == nullptr) {
        return "";
    }
    
    const char* sub = jwt_get_grant(jwt, "sub");
    std::string userId = sub ? sub : "";
    
    jwt_free(jwt);
    return userId;
}

UserRole JWTHandler::getRoleFromToken(const std::string& token) {
    jwt_t* jwt = nullptr;
    int ret = jwt_decode_2(&jwt, token.c_str(), nullptr);
    
    if (ret != 0 || jwt == nullptr) {
        return UserRole::USER;
    }
    
    const char* role = jwt_get_grant(jwt, "role");
    UserRole roleEnum = stringToRole(role ? role : "user");
    
    jwt_free(jwt);
    return roleEnum;
}

void JWTHandler::setSecret(const std::string& secret) {
    secret_ = secret;
}

void JWTHandler::setExpirationSeconds(int seconds) {
    expirationSeconds_ = seconds;
}

std::string JWTHandler::roleToString(UserRole role) {
    switch (role) {
        case UserRole::USER: return "user";
        case UserRole::STAFF: return "staff";
        case UserRole::ADMIN: return "admin";
        default: return "user";
    }
}

UserRole JWTHandler::stringToRole(const std::string& roleStr) {
    if (roleStr == "admin") return UserRole::ADMIN;
    if (roleStr == "staff") return UserRole::STAFF;
    return UserRole::USER;
}