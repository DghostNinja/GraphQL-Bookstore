#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>

namespace SimpleJson {
    
class JsonValue {
public:
    enum Type { Null, Boolean, Number, String, Array, Object };
    
    JsonValue() : type_(Null), boolVal_(false), numberVal_(0) {}
    explicit JsonValue(bool val) : type_(Boolean), boolVal_(val) {}
    explicit JsonValue(int val) : type_(Number), numberVal_(val) {}
    explicit JsonValue(double val) : type_(Number), numberVal_(val) {}
    explicit JsonValue(const std::string& val) : type_(String), stringVal_(val) {}
    explicit JsonValue(const char* val) : type_(String), stringVal_(val ? val : "") {}
    
    Type type() const { return type_; }
    bool isBool() const { return type_ == Boolean; }
    bool isNumber() const { return type_ == Number; }
    bool isString() const { return type_ == String; }
    bool isArray() const { return type_ == Array; }
    bool isObject() const { return type_ == Object; }
    bool isNull() const { return type_ == Null; }
    
    bool asBool() const { return boolVal_; }
    double asNumber() const { return numberVal_; }
    std::string asString() const { return stringVal_; }
    
    std::vector<JsonValue>& asArray() { return arrayVal_; }
    const std::vector<JsonValue>& asArray() const { return arrayVal_; }
    
    std::map<std::string, JsonValue>& asObject() { return objectVal_; }
    const std::map<std::string, JsonValue>& asObject() const { return objectVal_; }
    
    JsonValue& operator[](const std::string& key) {
        return objectVal_[key];
    }
    
    const JsonValue& operator[](const std::string& key) const {
        static JsonValue nullVal;
        auto it = objectVal_.find(key);
        if (it != objectVal_.end()) {
            return it->second;
        }
        return nullVal;
    }
    
    bool has(const std::string& key) const {
        return objectVal_.count(key) > 0;
    }
    
    std::string toString() const {
        std::ostringstream ss;
        switch (type_) {
            case Null: ss << "null"; break;
            case Boolean: ss << (boolVal_ ? "true" : "false"); break;
            case Number: ss << numberVal_; break;
            case String: ss << "\"" << escapeString(stringVal_) << "\""; break;
            case Array: {
                ss << "[";
                for (size_t i = 0; i < arrayVal_.size(); i++) {
                    if (i > 0) ss << ",";
                    ss << arrayVal_[i].toString();
                }
                ss << "]";
                break;
            }
            case Object: {
                ss << "{";
                bool first = true;
                for (const auto& pair : objectVal_) {
                    if (!first) ss << ",";
                    first = false;
                    ss << "\"" << pair.first << "\":" << pair.second.toString();
                }
                ss << "}";
                break;
            }
        }
        return ss.str();
    }
    
    std::string dump() const {
        return toString();
    }
    
    void push(const JsonValue& val) {
        type_ = Array;
        arrayVal_.push_back(val);
    }
    
    void set(const std::string& key, const JsonValue& val) {
        type_ = Object;
        objectVal_[key] = val;
    }
    
    void setType(Type t) { type_ = t; }

private:
    Type type_;
    bool boolVal_;
    double numberVal_;
    std::string stringVal_;
    std::vector<JsonValue> arrayVal_;
    std::map<std::string, JsonValue> objectVal_;
    
    static std::string escapeString(const std::string& str) {
        std::string result;
        for (char c : str) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c;
            }
        }
        return result;
    }
};

class Json {
public:
    static JsonValue parse(const std::string& str) {
        JsonValue result;
        result.setType(JsonValue::Null);
        return result;
    }
    
    static JsonValue array() {
        JsonValue val;
        val.setType(JsonValue::Array);
        return val;
    }
    
    static JsonValue object() {
        JsonValue val;
        val.setType(JsonValue::Object);
        return val;
    }
    
    static JsonValue null() {
        return JsonValue();
    }
    
    static JsonValue value(const std::string& val) {
        return JsonValue(val);
    }
    
    static JsonValue value(int val) {
        return JsonValue(val);
    }
    
    static JsonValue value(double val) {
        return JsonValue(val);
    }
    
    static JsonValue value(bool val) {
        return JsonValue(val);
    }
};

inline JsonValue Json() { return JsonValue(); }
inline JsonValue Json(bool val) { return JsonValue(val); }
inline JsonValue Json(int val) { return JsonValue(val); }
inline JsonValue Json(double val) { return JsonValue(val); }
inline JsonValue Json(const std::string& val) { return JsonValue(val); }
inline JsonValue Json(const char* val) { return JsonValue(val); }

}