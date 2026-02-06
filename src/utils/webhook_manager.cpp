#include "utils/webhook_manager.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <regex>

WebhookManager& WebhookManager::getInstance() {
    static WebhookManager instance;
    return instance;
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string WebhookManager::performHttpRequest(const std::string& url, const std::string& method,
                                               const std::string& body, const std::string& headers) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "{\"error\": \"Failed to initialize HTTP client\"}";
    }
    
    std::string responseString;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        struct curl_slist* headerList = nullptr;
        headerList = curl_slist_append(headerList, "Content-Type: application/json");
        
        if (!headers.empty()) {
            std::istringstream headerStream(headers);
            std::string header;
            while (std::getline(headerStream, header)) {
                headerList = curl_slist_append(headerList, header.c_str());
            }
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    }
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        std::string error = curl_easy_strerror(res);
        curl_easy_cleanup(curl);
        return "{\"error\": \"" + error + "\"}";
    }
    
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);
    
    return "{\"statusCode\": " + std::to_string(httpCode) + ", \"body\": " + responseString + "}";
}

bool WebhookManager::isInternalUrl(const std::string& url) {
    std::regex internalPattern(R"(^(localhost|127\.0\.0\.1|0\.0\.0\.0|::1|192\.168\.|10\.|172\.(1[6-9]|2[0-9]|3[0-1])\.))");
    return std::regex_search(url, internalPattern);
}

std::string WebhookManager::sendWebhook(const std::string& url, const std::string& eventType,
                                       const std::string& payload, const std::string& headers) {
    std::string sanitizedUrl = url;
    
    if (url.find("http://") != 0 && url.find("https://") != 0) {
        sanitizedUrl = "http://" + url;
    }
    
    std::string webhookPayload = "{\"event\": \"" + eventType + "\", \"data\": " + payload + "}";
    
    std::string response = performHttpRequest(sanitizedUrl, "POST", webhookPayload, headers);
    
    return response;
}

std::string WebhookManager::testWebhook(const std::string& url) {
    std::string sanitizedUrl = url;
    
    if (url.find("http://") != 0 && url.find("https://") != 0) {
        sanitizedUrl = "http://" + url;
    }
    
    std::string response = performHttpRequest(sanitizedUrl);
    
    return response;
}

std::string WebhookManager::fetchExternalResource(const std::string& url) {
    std::string sanitizedUrl = url;
    
    if (url.find("http://") != 0 && url.find("https://") != 0) {
        sanitizedUrl = "http://" + url;
    }
    
    std::string response = performHttpRequest(sanitizedUrl);
    
    return response;
}

std::string WebhookManager::validateUrl(const std::string& url) {
    if (url.empty()) {
        return "{\"valid\": false, \"reason\": \"URL is empty\"}";
    }
    
    std::regex urlPattern(R"(^https?://[a-zA-Z0-9\-._~:/?#\[\]@!$&'()*+,;=]+$)");
    
    if (std::regex_match(url, urlPattern)) {
        return "{\"valid\": true}";
    }
    
    return "{\"valid\": false, \"reason\": \"Invalid URL format\"}";
}