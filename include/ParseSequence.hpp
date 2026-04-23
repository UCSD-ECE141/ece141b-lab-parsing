#pragma once
#include <string>
#include <vector>
#include <functional>
#include "Tokenizer.hpp"
#include "Parser.hpp"

namespace lab {

// General-purpose declarative parser. Chain primitives to describe a grammar.
//
// Primitives:
//   expect(keyword)      - consume a keyword token
//   expectPunct(char)    - consume a punctuation token
//   capture(slot)        - store current token data in a named slot, advance
//   captureIf(slot, fn)  - store if predicate passes, advance
//   collectIf(fn)        - append current token data to items list if predicate passes, advance
//   skip()               - advance one token
//   optional(builder)    - try inner steps, backtrack if they fail
//   group(open, close, builder) - run inner steps wrapped in open/close punctuation
//   repeat(sep, builder) - run inner steps once, then repeat while separator found
//   run(tok, result)     - execute all steps
//
// You compose these into grammars. The class knows nothing about SQL.
//
// Tokenizer notes:
//   - "*" is tokenized as Punctuation, not Identifier
//   - Type names (int, float, varchar, boolean) are tokenized as Keywords
//   - Operators: =, >, <, >=, <=, !=
//
// Example — parsing DROP TABLE <name>;
//
//   ParseSequence seq;
//   seq.expect("drop")
//      .expect("table")
//      .captureIf("table", isIdent)
//      .expectPunct(';');
//
//   ParsedCommand cmd{"DROP"};
//   if (seq.run(tok, cmd)) cmd.ok = true;

class ParseSequence {
public:
    ParseSequence() {}

    // Expect a specific keyword. Fails if current token is not that keyword.
    ParseSequence& expect(const std::string& kw) {
        steps_.push_back([=](Tokenizer& tok, ParsedCommand& r) -> bool {
            if (!tok.more() || !tok.current().isKeyword(kw)) {
                r.fail("expected '" + kw + "'");
                return false;
            }
            tok.next();
            return true;
        });
        return *this;
    }

    // Expect a specific punctuation character.
    ParseSequence& expectPunct(char ch) {
        steps_.push_back([=](Tokenizer& tok, ParsedCommand& r) -> bool {
            if (!tok.more() || !tok.current().isPunct(ch)) {
                r.fail(std::string("expected '") + ch + "'");
                return false;
            }
            tok.next();
            return true;
        });
        return *this;
    }

    // Capture current token data into a named slot. Advances.
    ParseSequence& capture(const std::string& slot) {
        steps_.push_back([=](Tokenizer& tok, ParsedCommand& r) -> bool {
            if (!tok.more()) {
                r.fail("expected token for '" + slot + "'");
                return false;
            }
            r.set(slot, tok.current().data);
            tok.next();
            return true;
        });
        return *this;
    }

    // Capture current token into a named slot only if predicate passes.
    ParseSequence& captureIf(const std::string& slot,
                              std::function<bool(const Token&)> pred) {
        steps_.push_back([=](Tokenizer& tok, ParsedCommand& r) -> bool {
            if (!tok.more() || !pred(tok.current())) {
                r.fail("predicate failed for '" + slot + "'");
                return false;
            }
            r.set(slot, tok.current().data);
            tok.next();
            return true;
        });
        return *this;
    }

    // Append current token data to items list if predicate passes. Advances.
    ParseSequence& collectIf(std::function<bool(const Token&)> pred) {
        steps_.push_back([=](Tokenizer& tok, ParsedCommand& r) -> bool {
            if (!tok.more() || !pred(tok.current())) {
                r.fail("expected matching token");
                return false;
            }
            r.addItem(tok.current().data);
            tok.next();
            return true;
        });
        return *this;
    }

    // Skip one token unconditionally.
    ParseSequence& skip() {
        steps_.push_back([](Tokenizer& tok, ParsedCommand&) -> bool {
            if (tok.more()) tok.next();
            return true;
        });
        return *this;
    }

    // Try inner steps. If they fail, backtrack and continue (no failure).
    ParseSequence& optional(std::function<void(ParseSequence&)> builder) {
        ParseSequence inner;
        builder(inner);
        steps_.push_back([inner](Tokenizer& tok, ParsedCommand& r) mutable -> bool {
            size_t save = tok.getIndex();
            ParsedCommand backup = r;
            for (auto& step : inner.steps_) {
                if (!step(tok, r)) {
                    tok.setIndex(save);
                    r = backup;
                    return true;
                }
            }
            return true;
        });
        return *this;
    }

    // Run inner steps once, wrapped in open/close punctuation.
    ParseSequence& group(char open, char close,
                         std::function<void(ParseSequence&)> builder) {
        ParseSequence inner;
        builder(inner);
        steps_.push_back([=](Tokenizer& tok, ParsedCommand& r) mutable -> bool {
            if (!tok.more() || !tok.current().isPunct(open)) {
                r.fail(std::string("expected '") + open + "'");
                return false;
            }
            tok.next();
            for (auto& step : inner.steps_) {
                if (!step(tok, r)) return false;
            }
            if (!tok.more() || !tok.current().isPunct(close)) {
                r.fail(std::string("expected '") + close + "'");
                return false;
            }
            tok.next();
            return true;
        });
        return *this;
    }

    // Run inner steps repeatedly, separated by a delimiter character.
    // Executes at least once, then repeats while separator is found.
    ParseSequence& repeat(char separator,
                          std::function<void(ParseSequence&)> builder) {
        ParseSequence inner;
        builder(inner);
        steps_.push_back([=](Tokenizer& tok, ParsedCommand& r) mutable -> bool {
            auto runInner = [&]() -> bool {
                for (auto& step : inner.steps_) {
                    if (!step(tok, r)) return false;
                }
                return true;
            };
            if (!runInner()) return false;
            while (tok.more() && tok.current().isPunct(separator)) {
                tok.next();
                if (!runInner()) return false;
            }
            return true;
        });
        return *this;
    }

    // Execute all steps against the tokenizer. Returns true if all passed.
    bool run(Tokenizer& tok, ParsedCommand& r) {
        for (auto& step : steps_) {
            if (!step(tok, r)) return false;
        }
        return true;
    }

private:
    using Step = std::function<bool(Tokenizer&, ParsedCommand&)>;
    std::vector<Step> steps_;
};

} // namespace lab
