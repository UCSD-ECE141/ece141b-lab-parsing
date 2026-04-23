#pragma once
#include "Parser.hpp"

namespace lab {

// Helper predicates for use with captureIf / collectIf
static bool isIdentifier(const Token& t) { return t.type == TokenType::Identifier; }

// DropParser is provided as an example of the declarative pattern.
// Note: recognizes() checks the leading keyword without consuming input.
class DropParser : public Parser {
public:
    bool recognizes(Tokenizer& tok) override {
        if (!tok.more()) return false;
        if (tok.current().isKeyword("drop")) return true;
        if (tok.current().isKeyword("table") && tok.peek().isKeyword("drop")) return true;
        return false;
    }
    ParsedCommand parse(Tokenizer& tok) override {
        ParsedCommand cmd{"DROP"};
        ParseSequence seq;
        seq.expect("drop")
           .expect("table")
           .captureIf("table", isIdentifier)
           .expectPunct(';');
        if (!seq.run(tok, cmd)) return cmd;
        if (tok.more()) return {"DROP", false, "unexpected tokens"};
        cmd.ok = true;
        return cmd;
    }
};

// Implement the remaining parsers below.

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

} // namespace lab
