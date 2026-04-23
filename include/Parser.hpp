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

    // Named string values. Expected keys per command type:
    //   SELECT: table, whereField, whereOp, whereValue, orderField, limit
    //   INSERT: table
    //   CREATE: table
    //   DROP:   table
    std::map<std::string, std::string> fields;

    // For SELECT: field list (may include "*"); for INSERT: column names
    std::vector<std::string> items;

    // For CREATE: (columnName, columnType) pairs
    // Note: column types (int, float, varchar, boolean) are tokenized as Keywords
    std::vector<std::pair<std::string, std::string>> typedFields;

    // For CREATE: constraints per column (columnName -> "NOT NULL PRIMARY KEY" etc.)
    // Constraints appear in order: NOT NULL before PRIMARY KEY
    std::map<std::string, std::string> constraints;

    // For INSERT: value rows (each row has one value per column)
    std::vector<std::vector<std::string>> valueRows;

    // Helpers for building results inside ParseSequence steps
    void set(const std::string& key, const std::string& val) { fields[key] = val; }
    std::string get(const std::string& key) const {
        auto it = fields.find(key);
        return it != fields.end() ? it->second : "";
    }
    void addItem(const std::string& item) { items.push_back(item); }
    void addTypedField(const std::string& name, const std::string& type) {
        typedFields.emplace_back(name, type);
    }
    void addConstraint(const std::string& field, const std::string& constraint) {
        auto& existing = constraints[field];
        if (!existing.empty()) existing += " ";
        existing += constraint;
    }
    void addValueRow(const std::vector<std::string>& row) { valueRows.push_back(row); }
    void fail(const std::string& msg) { ok = false; error = msg; }
};

class Parser {
public:
    virtual ~Parser() = default;
    virtual bool recognizes(Tokenizer& tok) = 0;
    virtual ParsedCommand parse(Tokenizer& tok) = 0;
};

} // namespace lab
