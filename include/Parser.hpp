#pragma once
#include <map>
#include <string>
#include <vector>
#include "Tokenizer.hpp"

namespace lab {

struct ParsedCommand {
    std::string type;   // "SELECT", "INSERT", "CREATE", "DROP"
    bool ok = false;
    std::string error;

    // Named string values (table, whereField, whereOp, whereValue, orderField, limit)
    std::map<std::string, std::string> fields;

    // For SELECT: field list; for INSERT: field names
    std::vector<std::string> items;

    // For CREATE: (fieldName, fieldType) pairs
    std::vector<std::pair<std::string, std::string>> typedFields;

    // For CREATE: constraints per field (fieldName -> "NOT NULL PRIMARY KEY" etc.)
    std::map<std::string, std::string> constraints;

    // For INSERT: value rows
    std::vector<std::vector<std::string>> valueRows;
};

class Parser {
public:
    virtual ~Parser() = default;
    virtual bool recognizes(Tokenizer& tok) = 0;
    virtual ParsedCommand parse(Tokenizer& tok) = 0;
};

} // namespace lab
