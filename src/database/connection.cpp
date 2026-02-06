#include "database/connection.h"
#include <iostream>
#include <stdexcept>

DatabaseConnection::DatabaseConnection(const std::string& conninfo)
    : conninfo_(conninfo), conn_(nullptr), connected_(false) {
}

DatabaseConnection::~DatabaseConnection() {
    disconnect();
}

bool DatabaseConnection::connect() {
    conn_ = PQconnectdb(conninfo_.c_str());
    
    if (PQstatus(conn_) != CONNECTION_OK) {
        std::cerr << "Connection to database failed: " << PQerrorMessage(conn_) << std::endl;
        PQfinish(conn_);
        conn_ = nullptr;
        connected_ = false;
        return false;
    }
    
    connected_ = true;
    return true;
}

void DatabaseConnection::disconnect() {
    if (conn_ != nullptr) {
        PQfinish(conn_);
        conn_ = nullptr;
    }
    connected_ = false;
}

bool DatabaseConnection::isConnected() const {
    return connected_ && conn_ != nullptr;
}

PGresult* DatabaseConnection::executeQuery(const std::string& query) {
    if (!isConnected()) {
        throw std::runtime_error("Not connected to database");
    }
    
    PGresult* result = PQexec(conn_, query.c_str());
    
    ExecStatusType status = PQresultStatus(result);
    if (status != PGRES_TUPLES_OK && status != PGRES_COMMAND_OK) {
        std::string error = PQerrorMessage(conn_);
        PQclear(result);
        throw std::runtime_error("Query failed: " + error);
    }
    
    return result;
}

PGresult* DatabaseConnection::executeQuery(const std::string& query, const std::vector<std::string>& params) {
    if (!isConnected()) {
        throw std::runtime_error("Not connected to database");
    }
    
    std::vector<const char*> paramValues;
    for (const auto& param : params) {
        paramValues.push_back(param.c_str());
    }
    
    PGresult* result = PQexecParams(
        conn_, 
        query.c_str(), 
        params.size(),
        nullptr,
        paramValues.data(),
        nullptr,
        nullptr,
        0
    );
    
    ExecStatusType status = PQresultStatus(result);
    if (status != PGRES_TUPLES_OK && status != PGRES_COMMAND_OK) {
        std::string error = PQerrorMessage(conn_);
        PQclear(result);
        throw std::runtime_error("Query failed: " + error);
    }
    
    return result;
}

void DatabaseConnection::clearResult(PGresult* result) {
    if (result != nullptr) {
        PQclear(result);
    }
}

std::string DatabaseConnection::escapeString(const std::string& input) {
    if (!isConnected()) {
        throw std::runtime_error("Not connected to database");
    }
    
    char* escaped = new char[input.length() * 2 + 1];
    int error;
    PQescapeStringConn(conn_, escaped, input.c_str(), input.length(), &error);
    
    std::string result(escaped);
    delete[] escaped;
    
    return result;
}

bool DatabaseConnection::beginTransaction() {
    try {
        PGresult* result = executeQuery("BEGIN");
        clearResult(result);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool DatabaseConnection::commitTransaction() {
    try {
        PGresult* result = executeQuery("COMMIT");
        clearResult(result);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool DatabaseConnection::rollbackTransaction() {
    try {
        PGresult* result = executeQuery("ROLLBACK");
        clearResult(result);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

DatabasePool& DatabasePool::getInstance() {
    static DatabasePool instance;
    return instance;
}

void DatabasePool::initialize(const std::string& conninfo, int poolSize) {
    conninfo_ = conninfo;
    maxPoolSize_ = poolSize;
}

std::shared_ptr<DatabaseConnection> DatabasePool::getConnection() {
    std::lock_guard<std::mutex> lock(pool_mutex_);
    
    if (!pool_.empty()) {
        auto conn = pool_.back();
        pool_.pop_back();
        return conn;
    }
    
    auto newConn = std::make_shared<DatabaseConnection>(conninfo_);
    newConn->connect();
    return newConn;
}

void DatabasePool::releaseConnection(std::shared_ptr<DatabaseConnection> conn) {
    std::lock_guard<std::mutex> lock(pool_mutex_);
    
    if (pool_.size() < static_cast<size_t>(maxPoolSize_)) {
        pool_.push_back(conn);
    }
}