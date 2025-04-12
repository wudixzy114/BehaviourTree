#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include <string>
#include <unordered_map>
#include <any>
#include <optional>
#include <stdexcept>
#include <iostream>

class Blackboard {
public:
    Blackboard() = default;

    template<typename T>
    void set(const std::string &key, T value) {
        data[key] = std::make_any<T>(std::move(value));
    }

    template<typename T>
    std::optional<T> get(const std::string &key) const {
        auto it = data.find(key);
        if (it == data.end()) {
            return std::nullopt;
        }
        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast &e) {
            std::cerr << "Blackboard::get type mismatch for key '" << key << "': " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    template<typename T>
    T getRequired(const std::string &key) const {
        auto it = data.find(key);
        if (it == data.end()) {
            throw std::runtime_error("Blackboard key not found: " + key);
        }
        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast &e) {
            throw std::runtime_error("Blackboard type mismatch for key '" + key + "': " + e.what());
        }
    }

    bool contains(const std::string &key) const {
        return data.count(key);
    }

    void remove(const std::string &key) {
        data.erase(key);
    }

    void clear() {
        data.clear();
    }

private:
    std::unordered_map<std::string, std::any> data;
};

#endif //BLACKBOARD_H
