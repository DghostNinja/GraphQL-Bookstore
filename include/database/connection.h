#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <mutex>
#include <libpq-fe.h>

class DatabaseConnection {
public:
    DatabaseConnection(const std::string& conninfo);
    ~DatabaseConnection();

    bool connect();
    void disconnect();
    bool isConnected() const;

    PGresult* executeQuery(const std::string& query);
    PGresult* executeQuery(const std::string& query, const std::vector<std::string>& params);
    
    void clearResult(PGresult* result);
    std::string escapeString(const std::string& input);
    
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

private:
    std::string conninfo_;
    PGconn* conn_;
    bool connected_;
};

class DatabasePool {
public:
    static DatabasePool& getInstance();
    
    void initialize(const std::string& conninfo, int poolSize);
    std::shared_ptr<DatabaseConnection> getConnection();
    void releaseConnection(std::shared_ptr<DatabaseConnection> conn);

private:
    DatabasePool() = default;
    ~DatabasePool() = default;
    DatabasePool(const DatabasePool&) = delete;
    DatabasePool& operator=(const DatabasePool&) = delete;

    std::vector<std::shared_ptr<DatabaseConnection>> pool_;
    std::mutex pool_mutex_;
    std::string conninfo_;
    int maxPoolSize_;
};