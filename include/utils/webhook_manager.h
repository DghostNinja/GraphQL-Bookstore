#pragma once

#include <string>
#include <functional>
#include <map>
#include <mutex>

class WebhookManager {
public:
    static WebhookManager& getInstance();
    
    std::string sendWebhook(const std::string& url, const std::string& eventType, 
                           const std::string& payload, const std::string& headers = "");
    std::string testWebhook(const std::string& url);
    std::string fetchExternalResource(const std::string& url);
    std::string validateUrl(const std::string& url);
    
private:
    WebhookManager() = default;
    ~WebhookManager() = default;
    WebhookManager(const WebhookManager&) = delete;
    WebhookManager& operator=(const WebhookManager&) = delete;
    
    std::string performHttpRequest(const std::string& url, const std::string& method = "GET",
                                   const std::string& body = "", const std::string& headers = "");
    bool isInternalUrl(const std::string& url);
};