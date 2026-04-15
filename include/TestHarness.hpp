#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>
#include <functional>
#include "Parser.hpp"
#include "Tokenizer.hpp"

namespace lab {

class TestHarness {
public:
    explicit TestHarness(std::vector<std::unique_ptr<Parser>> parsers)
        : parsers_(std::move(parsers)), rng_(42) {}

    bool runAll() {
        int totalPass = 0, totalTests = 0;

        auto run = [&](const std::string& label, int validCount, int errorCount,
                       std::function<void(std::vector<TestCase>&)> genValid,
                       std::function<void(std::vector<TestCase>&)> genError) {
            std::vector<TestCase> cases;
            genValid(cases);
            genError(cases);
            int pass = 0;
            std::cout << "\n\033[1m=== " << label << " (" << validCount
                      << " valid + " << errorCount << " error) ===\033[0m\n";
            for (auto& tc : cases) {
                bool ok = runOne(tc);
                if (ok) ++pass;
                totalTests++;
            }
            totalPass += pass;
            std::cout << "  " << pass << "/" << cases.size() << " passed\n";
        };

        run("SELECT", 50, 10,
            [&](auto& c) { generateSelectValid(c, 50); },
            [&](auto& c) { generateSelectError(c, 10); });
        run("INSERT", 50, 10,
            [&](auto& c) { generateInsertValid(c, 50); },
            [&](auto& c) { generateInsertError(c, 10); });
        run("CREATE TABLE", 50, 10,
            [&](auto& c) { generateCreateValid(c, 50); },
            [&](auto& c) { generateCreateError(c, 10); });
        run("DROP TABLE", 30, 10,
            [&](auto& c) { generateDropValid(c, 30); },
            [&](auto& c) { generateDropError(c, 10); });

        std::cout << "\n\033[1m=== TOTAL: " << totalPass << "/" << totalTests
                  << " ===\033[0m\n";
        return totalPass == totalTests;
    }

private:
    struct TestCase {
        std::string sql;
        bool expectValid;
        std::string expectedType;
        // Expected fields for validation
        std::map<std::string, std::string> expectedFields;
        std::vector<std::string> expectedItems;
        std::vector<std::pair<std::string, std::string>> expectedTypedFields;
        std::map<std::string, std::string> expectedConstraints;
        std::vector<std::vector<std::string>> expectedValueRows;
    };

    std::vector<std::unique_ptr<Parser>> parsers_;
    std::mt19937 rng_;

    // -- Pools --
    static const std::vector<std::string>& fieldPool() {
        static std::vector<std::string> p = {
            "id", "name", "age", "email", "price", "status", "qty",
            "title", "rating", "active", "created", "updated",
            "category", "score", "level"
        };
        return p;
    }
    static const std::vector<std::string>& tablePool() {
        static std::vector<std::string> p = {
            "users", "orders", "products", "accounts", "tasks",
            "events", "logs", "items", "teams", "records"
        };
        return p;
    }
    static const std::vector<std::string>& opPool() {
        static std::vector<std::string> p = { "=", ">", "<", ">=", "<=", "!=" };
        return p;
    }
    static const std::vector<std::string>& typePool() {
        static std::vector<std::string> p = { "int", "float", "varchar", "boolean" };
        return p;
    }

    std::string pickOne(const std::vector<std::string>& pool) {
        return pool[rng_() % pool.size()];
    }

    std::vector<std::string> pickN(const std::vector<std::string>& pool, int n) {
        std::vector<std::string> copy = pool;
        std::shuffle(copy.begin(), copy.end(), rng_);
        copy.resize(std::min(n, (int)copy.size()));
        return copy;
    }

    std::string randomValue() {
        int kind = rng_() % 3;
        if (kind == 0) return std::to_string(rng_() % 1000);
        if (kind == 1) return "\"val_" + std::to_string(rng_() % 100) + "\"";
        return (rng_() % 2) ? "true" : "false";
    }

    std::string randomValueData() {
        // The data portion (without quotes for strings)
        int kind = rng_() % 3;
        if (kind == 0) return std::to_string(rng_() % 1000);
        if (kind == 1) return "val_" + std::to_string(rng_() % 100);
        return (rng_() % 2) ? "true" : "false";
    }

    std::string randomWhereValue() {
        if (rng_() % 2) return std::to_string(rng_() % 500);
        return "\"item_" + std::to_string(rng_() % 50) + "\"";
    }

    std::string whereValueData(const std::string& sqlVal) {
        // Strip quotes if present
        if (sqlVal.size() >= 2 && sqlVal.front() == '"' && sqlVal.back() == '"')
            return sqlVal.substr(1, sqlVal.size() - 2);
        return sqlVal;
    }

    // -- SELECT generation --
    void generateSelectValid(std::vector<TestCase>& cases, int n) {
        for (int i = 0; i < n; ++i) {
            TestCase tc;
            tc.expectValid = true;
            tc.expectedType = "SELECT";

            // Fields
            bool useStar = (rng_() % 5 == 0);
            std::vector<std::string> fields;
            if (useStar) {
                fields.push_back("*");
            } else {
                int count = 1 + rng_() % 4;
                fields = pickN(fieldPool(), count);
            }
            tc.expectedItems = fields;

            // Table
            std::string table = pickOne(tablePool());
            tc.expectedFields["table"] = table;

            // Build SQL
            std::ostringstream sql;
            sql << "SELECT ";
            for (size_t j = 0; j < fields.size(); ++j) {
                if (j > 0) sql << ", ";
                sql << fields[j];
            }
            sql << " FROM " << table;

            // WHERE (40% chance)
            if (rng_() % 5 < 2) {
                std::string wf = pickOne(fieldPool());
                std::string wo = pickOne(opPool());
                std::string wv = randomWhereValue();
                sql << " WHERE " << wf << " " << wo << " " << wv;
                tc.expectedFields["whereField"] = wf;
                tc.expectedFields["whereOp"] = wo;
                tc.expectedFields["whereValue"] = whereValueData(wv);
            }

            // ORDER BY (20% chance)
            if (rng_() % 5 == 0) {
                std::string of = pickOne(fieldPool());
                sql << " ORDER BY " << of;
                tc.expectedFields["orderField"] = of;
            }

            // LIMIT (20% chance)
            if (rng_() % 5 == 0) {
                int lim = 1 + rng_() % 100;
                sql << " LIMIT " << lim;
                tc.expectedFields["limit"] = std::to_string(lim);
            }

            sql << ";";
            tc.sql = sql.str();
            cases.push_back(std::move(tc));
        }
    }

    void generateSelectError(std::vector<TestCase>& cases, int n) {
        std::vector<std::string> errs = {
            "SELECT name age FROM users;",             // missing comma
            "SELECT name, age users;",                 // missing FROM
            "SELECT FROM users;",                      // missing fields
            "SELECT name, age FROM;",                  // missing table
            "SELECT name FROM users WHERE;",           // WHERE without condition
            "SELECT name FROM users WHERE age;",       // WHERE without op/value
            "SELECT name FROM users WHERE age >;",     // WHERE without value
            "SELECT name FROM users ORDER name;",      // missing BY
            "SELECT name FROM users LIMIT;",           // LIMIT without number
            "SELECT name FROM users",                  // missing semicolon
        };
        for (int i = 0; i < n && i < (int)errs.size(); ++i) {
            TestCase tc;
            tc.sql = errs[i];
            tc.expectValid = false;
            tc.expectedType = "SELECT";
            cases.push_back(std::move(tc));
        }
    }

    // -- INSERT generation --
    void generateInsertValid(std::vector<TestCase>& cases, int n) {
        for (int i = 0; i < n; ++i) {
            TestCase tc;
            tc.expectValid = true;
            tc.expectedType = "INSERT";

            std::string table = pickOne(tablePool());
            tc.expectedFields["table"] = table;

            int fieldCount = 2 + rng_() % 4;
            auto fields = pickN(fieldPool(), fieldCount);
            tc.expectedItems = fields;

            int rowCount = 1 + rng_() % 4;
            std::ostringstream sql;
            sql << "INSERT INTO " << table << " (";
            for (size_t j = 0; j < fields.size(); ++j) {
                if (j > 0) sql << ", ";
                sql << fields[j];
            }
            sql << ") VALUES ";

            for (int r = 0; r < rowCount; ++r) {
                if (r > 0) sql << ", ";
                sql << "(";
                std::vector<std::string> row;
                for (int c = 0; c < fieldCount; ++c) {
                    if (c > 0) sql << ", ";
                    std::string val = randomValueData();
                    row.push_back(val);
                    // In SQL, strings need quotes
                    bool isNum = !val.empty() && (std::isdigit(val[0]) || val[0] == '-');
                    bool isBool = (val == "true" || val == "false");
                    if (!isNum && !isBool) {
                        sql << "\"" << val << "\"";
                    } else {
                        sql << val;
                    }
                }
                sql << ")";
                tc.expectedValueRows.push_back(std::move(row));
            }
            sql << ";";
            tc.sql = sql.str();
            cases.push_back(std::move(tc));
        }
    }

    void generateInsertError(std::vector<TestCase>& cases, int n) {
        std::vector<std::string> errs = {
            "INSERT INTO users (name, age) VALUES (\"bob\");",       // mismatched count
            "INSERT users (name) VALUES (\"bob\");",                  // missing INTO
            "INSERT INTO users name VALUES (\"bob\");",               // missing parens
            "INSERT INTO users (name) (\"bob\");",                    // missing VALUES
            "INSERT INTO users (name) VALUES \"bob\";",               // missing parens on values
            "INSERT INTO (name) VALUES (\"bob\");",                   // missing table
            "INSERT INTO users () VALUES ();",                        // empty field list
            "INSERT INTO users (name) VALUES (\"bob\")",              // missing semicolon
            "INSERT INTO users (name, age) VALUES (\"bob\", 25), (\"sue\");", // second row mismatch
            "INSERT INTO users (name) VALUES;",                       // VALUES with no rows
        };
        for (int i = 0; i < n && i < (int)errs.size(); ++i) {
            TestCase tc;
            tc.sql = errs[i];
            tc.expectValid = false;
            tc.expectedType = "INSERT";
            cases.push_back(std::move(tc));
        }
    }

    // -- CREATE TABLE generation --
    void generateCreateValid(std::vector<TestCase>& cases, int n) {
        for (int i = 0; i < n; ++i) {
            TestCase tc;
            tc.expectValid = true;
            tc.expectedType = "CREATE";

            std::string table = pickOne(tablePool());
            tc.expectedFields["table"] = table;

            int colCount = 2 + rng_() % 4;
            auto colNames = pickN(fieldPool(), colCount);

            std::ostringstream sql;
            sql << "CREATE TABLE " << table << " (";
            for (int c = 0; c < colCount; ++c) {
                if (c > 0) sql << ", ";
                std::string colType = pickOne(typePool());
                sql << colNames[c] << " " << colType;
                tc.expectedTypedFields.emplace_back(colNames[c], colType);

                std::string constraint;
                // NOT NULL: 30% chance
                if (rng_() % 10 < 3) {
                    sql << " NOT NULL";
                    constraint = "NOT NULL";
                }
                // PRIMARY KEY: first column 50%, others 0%
                if (c == 0 && rng_() % 2 == 0) {
                    sql << " PRIMARY KEY";
                    if (!constraint.empty()) constraint += " ";
                    constraint += "PRIMARY KEY";
                }
                if (!constraint.empty()) {
                    tc.expectedConstraints[colNames[c]] = constraint;
                }
            }
            sql << ");";
            tc.sql = sql.str();
            cases.push_back(std::move(tc));
        }
    }

    void generateCreateError(std::vector<TestCase>& cases, int n) {
        std::vector<std::string> errs = {
            "CREATE TABLE users id int, name varchar);",    // missing open paren
            "CREATE TABLE users (id int, name varchar;",    // missing close paren
            "CREATE TABLE (id int);",                        // missing table name
            "CREATE users (id int);",                        // missing TABLE keyword
            "CREATE TABLE users (id, name varchar);",        // missing type
            "CREATE TABLE users (id integer);",              // unknown type
            "CREATE TABLE users ();",                        // empty columns
            "CREATE TABLE users (id int)",                   // missing semicolon
            "CREATE TABLE users (id int, name);",            // second col missing type
            "CREATE TABLE users (id int name varchar);",     // missing comma
        };
        for (int i = 0; i < n && i < (int)errs.size(); ++i) {
            TestCase tc;
            tc.sql = errs[i];
            tc.expectValid = false;
            tc.expectedType = "CREATE";
            cases.push_back(std::move(tc));
        }
    }

    // -- DROP TABLE generation --
    void generateDropValid(std::vector<TestCase>& cases, int n) {
        for (int i = 0; i < n; ++i) {
            TestCase tc;
            tc.expectValid = true;
            tc.expectedType = "DROP";
            std::string table = pickOne(tablePool());
            tc.expectedFields["table"] = table;
            tc.sql = "DROP TABLE " + table + ";";
            cases.push_back(std::move(tc));
        }
    }

    void generateDropError(std::vector<TestCase>& cases, int n) {
        std::vector<std::string> errs = {
            "DROP users;",                    // missing TABLE
            "DROP TABLE;",                    // missing name
            "DROP TABLE users",               // missing semicolon
            "DROP TABLE users extra;",        // extra token
            "DROP users TABLE;",              // wrong order
            "DROP TABLE 123;",                // number not identifier
            "DROP TABLE users products;",     // two names
            "DROP;",                          // missing everything
            "DROP TABLE users, products;",    // comma separated
            "TABLE DROP users;",              // reversed keywords
        };
        for (int i = 0; i < n && i < (int)errs.size(); ++i) {
            TestCase tc;
            tc.sql = errs[i];
            tc.expectValid = false;
            tc.expectedType = "DROP";
            cases.push_back(std::move(tc));
        }
    }

    // -- Run one test --
    bool runOne(const TestCase& tc) {
        Tokenizer tok(tc.sql);

        // Find a parser that recognizes this input
        Parser* chosen = nullptr;
        for (auto& p : parsers_) {
            size_t saved = tok.getIndex();
            if (p->recognizes(tok)) {
                chosen = p.get();
                tok.setIndex(saved);
                break;
            }
            tok.setIndex(saved);
        }

        if (!tc.expectValid) {
            // Error test: the parser MUST recognize the leading keyword
            // (the SQL starts with SELECT/INSERT/CREATE/DROP) and then
            // reject the malformed input. If no parser recognizes it,
            // that means the student hasn't implemented recognizes() yet.
            if (!chosen) {
                printResult(false, tc.sql, "no parser recognized this input");
                return false;
            }
            tok.setIndex(0);
            auto result = chosen->parse(tok);
            if (!result.ok) {
                printResult(true, tc.sql, "");
                return true;
            }
            // Parser said ok. Check if the result actually validates.
            // If validation fails, the parser produced wrong output,
            // which counts as a correct rejection for error tests.
            std::string reason = validateResult(tc, result);
            if (!reason.empty()) {
                printResult(true, tc.sql, "");
                return true;
            }
            printResult(false, tc.sql, "expected failure but parse succeeded");
            return false;
        }

        // Valid test
        if (!chosen) {
            printResult(false, tc.sql, "no parser recognized this input");
            return false;
        }

        tok.setIndex(0);
        auto result = chosen->parse(tok);
        if (!result.ok) {
            printResult(false, tc.sql, "parse failed: " + result.error);
            return false;
        }

        std::string reason = validateResult(tc, result);
        if (!reason.empty()) {
            printResult(false, tc.sql, reason);
            return false;
        }

        printResult(true, tc.sql, "");
        return true;
    }

    std::string validateResult(const TestCase& tc, const ParsedCommand& cmd) {
        // Check type
        if (cmd.type != tc.expectedType) {
            return "expected type=\"" + tc.expectedType + "\", got type=\"" + cmd.type + "\"";
        }

        // Check named fields
        for (auto& [key, val] : tc.expectedFields) {
            auto it = cmd.fields.find(key);
            if (it == cmd.fields.end() || it->second != val) {
                std::string got = (it != cmd.fields.end()) ? it->second : "(missing)";
                return "expected " + key + "=\"" + val + "\", got " + key + "=\"" + got + "\"";
            }
        }

        // Check items
        if (tc.expectedItems != cmd.items) {
            return "items mismatch: expected [" + join(tc.expectedItems) +
                   "], got [" + join(cmd.items) + "]";
        }

        // Check typed fields
        if (tc.expectedTypedFields != cmd.typedFields) {
            return "typedFields mismatch";
        }

        // Check constraints
        if (tc.expectedConstraints != cmd.constraints) {
            std::string exp, got;
            for (auto& [k, v] : tc.expectedConstraints) exp += k + ":" + v + " ";
            for (auto& [k, v] : cmd.constraints) got += k + ":" + v + " ";
            return "constraints mismatch: expected {" + exp + "}, got {" + got + "}";
        }

        // Check value rows
        if (tc.expectedValueRows != cmd.valueRows) {
            return "valueRows mismatch: expected " +
                   std::to_string(tc.expectedValueRows.size()) + " rows, got " +
                   std::to_string(cmd.valueRows.size());
        }

        return "";
    }

    void printResult(bool pass, const std::string& sql, const std::string& reason) {
        std::string truncSql = sql.size() > 72 ? sql.substr(0, 69) + "..." : sql;
        if (pass) {
            std::cout << "  \033[32m[PASS]\033[0m " << truncSql << "\n";
        } else {
            std::cout << "  \033[31m[FAIL]\033[0m " << truncSql << "\n";
            if (!reason.empty()) {
                std::cout << "         " << reason << "\n";
            }
        }
    }

    static std::string join(const std::vector<std::string>& v) {
        std::string r;
        for (size_t i = 0; i < v.size(); ++i) {
            if (i > 0) r += ", ";
            r += v[i];
        }
        return r;
    }
};

} // namespace lab
