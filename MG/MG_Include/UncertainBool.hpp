//
// Created by BZLZHH on 2025/5/3.
//

#pragma once
#include <initializer_list>
#include <stdexcept>
#include <utility>

class UncertainBool {
public:
    enum Value { False = 0, True = 1, Unknown = 2 };

    UncertainBool() : value_(False), default_(false) {}

    explicit UncertainBool(bool b) : value_(b ? True : False), default_(false) {}

    UncertainBool(Value val, bool def) : value_(val), default_(def) {
        validateValue();
    }
    
    UncertainBool(bool val, bool def) : value_(val ? True : False), default_(def) {
        validateValue();
    }
    
    UncertainBool& operator=(Value val) {
        value_ = val;
        validateValue();
        return *this;
    }
    
    UncertainBool& operator=(bool b) {
        value_ = b ? True : False;
        return *this;
    }
    
    UncertainBool& operator=(std::initializer_list<std::pair<bool, bool>> init) {
        if (init.size() != 1) throw std::invalid_argument("Invalid initializer");
        value_ = init.begin()->first ? True : False;
        default_ = init.begin()->second;
        return *this;
    }
    
    explicit operator bool() const {
        return toBool();
    }
    
    bool toBool() const {
        if (value_ == True)       return true;
        else if (value_ == False) return false;
        else                      return default_;
    }

    UncertainBool operator!() const {
        return {static_cast<bool>(*this) ? False : True, false};
    }

    bool operator==(const UncertainBool& rhs) const {
        return static_cast<bool>(*this) == static_cast<bool>(rhs);
    }
    bool operator!=(const UncertainBool& rhs) const { return !(*this == rhs); }

    friend UncertainBool operator&&(const UncertainBool& lhs, const UncertainBool& rhs) {
        bool actualDefault = lhs.default_ && rhs.default_;
        if(lhs.value_ == False || rhs.value_ == False) return {False,actualDefault};
        else if(lhs.value_ == True && rhs.value_ == True) return {True,actualDefault};
        else return {Unknown, actualDefault};
    }

    friend UncertainBool operator||(const UncertainBool& lhs, const UncertainBool& rhs) {
        bool actualDefault = lhs.default_ || rhs.default_;
        if(lhs.value_ == True || rhs.value_ == True) return {True,actualDefault};
        else if(lhs.value_ == False && rhs.value_ == False) return {False,actualDefault};
        else return {Unknown, actualDefault};
    }
    
    friend UncertainBool operator&&(bool lhs,const UncertainBool& rhs){
        return UncertainBool(lhs, lhs) && rhs;
    }

    friend UncertainBool operator||(bool lhs,const UncertainBool& rhs){
        return UncertainBool(lhs, lhs) || rhs;
    }

    UncertainBool& operator&=(const UncertainBool& rhs) {
        *this = *this && rhs;
        return *this;
    }

    UncertainBool& operator|=(const UncertainBool& rhs) {
        *this = *this || rhs;
        return *this;
    }

    UncertainBool& operator=(std::initializer_list<std::pair<Value, bool>> init) {
        if (init.size() != 1) throw std::invalid_argument("Invalid initializer");
        value_ = init.begin()->first;
        default_ = init.begin()->second;
        validateValue();
        return *this;
    }

    Value getValue() const { return value_; }
    bool getDefault() const { return default_; }

private:
    Value value_;
    bool default_;

    void validateValue() {
        if (value_ != False && value_ != True && value_ != Unknown) {
            throw std::invalid_argument("Invalid value");
        }
    }
};

inline UncertainBool operator!(const UncertainBool& lhs) {
    return lhs.operator!();
}

inline bool operator==(bool lhs, const UncertainBool& rhs) {
    return static_cast<bool>(rhs) == lhs;
}

inline bool operator!=(bool lhs, const UncertainBool& rhs) {
    return !(lhs == rhs);
}