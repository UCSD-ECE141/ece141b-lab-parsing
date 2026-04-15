#pragma once
#include "Parser.hpp"

namespace lab {

class SelectParser : public Parser {
public:
    bool recognizes(Tokenizer& tok) override { return false; }
    ParsedCommand parse(Tokenizer& tok) override {
        return {"SELECT", false, "not implemented"};
    }
};

class InsertParser : public Parser {
public:
    bool recognizes(Tokenizer& tok) override { return false; }
    ParsedCommand parse(Tokenizer& tok) override {
        return {"INSERT", false, "not implemented"};
    }
};

class CreateParser : public Parser {
public:
    bool recognizes(Tokenizer& tok) override { return false; }
    ParsedCommand parse(Tokenizer& tok) override {
        return {"CREATE", false, "not implemented"};
    }
};

class DropParser : public Parser {
public:
    bool recognizes(Tokenizer& tok) override { return false; }
    ParsedCommand parse(Tokenizer& tok) override {
        return {"DROP", false, "not implemented"};
    }
};

} // namespace lab
