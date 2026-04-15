#pragma once
#include <string>
#include <algorithm>

namespace lab {

enum class TokenType {
    Keyword, Identifier, Number, String, Operator, Punctuation, Boolean, Eof
};

struct Token {
    TokenType type = TokenType::Eof;
    std::string data;

    bool isKeyword(const std::string& kw) const {
        if (type != TokenType::Keyword) return false;
        std::string a = data, b = kw;
        std::transform(a.begin(), a.end(), a.begin(), ::tolower);
        std::transform(b.begin(), b.end(), b.begin(), ::tolower);
        return a == b;
    }

    bool isPunct(char ch) const {
        return type == TokenType::Punctuation && data.size() == 1 && data[0] == ch;
    }

    bool isOp() const {
        return type == TokenType::Operator;
    }
};

} // namespace lab
