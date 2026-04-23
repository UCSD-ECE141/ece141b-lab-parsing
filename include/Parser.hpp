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

    // Named string values, keyed by slot name. Each command type uses:
    //
    //   SELECT: fields["table"]      = table name
    //           fields["whereField"] = column in WHERE clause
    //           fields["whereOp"]    = operator (=, >, <, >=, <=, !=)
    //           fields["whereValue"] = value to compare against
    //           fields["orderField"] = column in ORDER BY clause
    //           fields["limit"]      = row limit as a string
    //           items                = column names being selected (or "*")
    //
    //   INSERT: fields["table"]      = table name
    //           items                = column names
    //           valueRows            = one vector<string> per VALUES tuple
    //
    //   CREATE: fields["table"]      = table name
    //           typedFields          = (columnName, columnType) pairs
    //           constraints          = columnName -> "NOT NULL", "PRIMARY KEY",
    //                                  or "NOT NULL PRIMARY KEY"
    //
    //   DROP:   fields["table"]      = table name
    //
    std::map<std::string, std::string> fields;

    std::vector<std::string> items;

    // Note: column types (int, float, varchar, boolean) are tokenized as Keywords
    std::vector<std::pair<std::string, std::string>> typedFields;

    // Constraints appear in order: NOT NULL before PRIMARY KEY
    std::map<std::string, std::string> constraints;

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
