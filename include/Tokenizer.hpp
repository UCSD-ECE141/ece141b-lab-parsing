#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <set>
#include "Token.hpp"

namespace lab {

class Tokenizer {
public:
    explicit Tokenizer(const std::string& input) {
        tokenize(input);
        pos_ = 0;
    }

    bool more() const { return pos_ < tokens_.size(); }

    const Token& current() const {
        static Token eof{TokenType::Eof, ""};
        return pos_ < tokens_.size() ? tokens_[pos_] : eof;
    }

    const Token& peek(int offset = 1) const {
        static Token eof{TokenType::Eof, ""};
        size_t idx = pos_ + offset;
        return idx < tokens_.size() ? tokens_[idx] : eof;
    }

    bool next(int count = 1) {
        for (int i = 0; i < count && pos_ < tokens_.size(); ++i) ++pos_;
        return pos_ < tokens_.size();
    }

    size_t getIndex() const { return pos_; }
    void setIndex(size_t idx) { pos_ = idx; }
    size_t remaining() const { return pos_ < tokens_.size() ? tokens_.size() - pos_ : 0; }

private:
    std::vector<Token> tokens_;
    size_t pos_ = 0;

    static const std::set<std::string>& keywords() {
        static std::set<std::string> kw = {
            "select", "from", "where", "order", "by", "limit",
            "insert", "into", "values", "create", "table", "drop",
            "not", "null", "primary", "key",
            "int", "float", "varchar", "boolean",
            "true", "false", "and", "or"
        };
        return kw;
    }

    void tokenize(const std::string& input) {
        size_t i = 0;
        while (i < input.size()) {
            // skip whitespace
            if (std::isspace(input[i])) { ++i; continue; }

            // two-char operators
            if (i + 1 < input.size()) {
                std::string two = input.substr(i, 2);
                if (two == ">=" || two == "<=" || two == "!=") {
                    tokens_.push_back({TokenType::Operator, two});
                    i += 2;
                    continue;
                }
            }

            // single-char operators
            if (input[i] == '=' || input[i] == '>' || input[i] == '<') {
                tokens_.push_back({TokenType::Operator, std::string(1, input[i])});
                ++i;
                continue;
            }

            // punctuation
            if (input[i] == '(' || input[i] == ')' || input[i] == ',' ||
                input[i] == ';' || input[i] == '*') {
                tokens_.push_back({TokenType::Punctuation, std::string(1, input[i])});
                ++i;
                continue;
            }

            // quoted string
            if (input[i] == '"' || input[i] == '\'') {
                char quote = input[i];
                ++i;
                std::string val;
                while (i < input.size() && input[i] != quote) {
                    val += input[i++];
                }
                if (i < input.size()) ++i; // skip closing quote
                tokens_.push_back({TokenType::String, val});
                continue;
            }

            // number
            if (std::isdigit(input[i])) {
                std::string num;
                while (i < input.size() && (std::isdigit(input[i]) || input[i] == '.')) {
                    num += input[i++];
                }
                tokens_.push_back({TokenType::Number, num});
                continue;
            }

            // identifier or keyword
            if (std::isalpha(input[i]) || input[i] == '_') {
                std::string word;
                while (i < input.size() && (std::isalnum(input[i]) || input[i] == '_')) {
                    word += input[i++];
                }
                std::string lower = word;
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

                if (lower == "true" || lower == "false") {
                    tokens_.push_back({TokenType::Boolean, lower});
                } else if (keywords().count(lower)) {
                    tokens_.push_back({TokenType::Keyword, lower});
                } else {
                    tokens_.push_back({TokenType::Identifier, word});
                }
                continue;
            }

            // unknown char, skip
            ++i;
        }
    }
};

} // namespace lab
