# SQL Parsing Challenge

## What to do

1. Open `include/Parsers.hpp`. `DropParser` is already implemented as an example. Implement `SelectParser`, `InsertParser`, and `CreateParser`.
2. For each parser, implement `recognizes()` to check whether the first token matches the command type (e.g., `select`, `insert`, `create`).
3. For each parser, implement `parse()` using `ParseSequence` to describe the grammar declaratively. Populate the `ParsedCommand` struct with the parsed data.
4. See `include/Parser.hpp` for the exact fields each command type must populate.
5. Build and run after each parser to check your progress.

Use the `ParseSequence` class in `include/ParseSequence.hpp` to chain together grammar rules instead of writing manual if/else token-walking loops. You may need to extend `ParseSequence` with your own primitives.

## Commands to support

```sql
SELECT * FROM users;
SELECT name, age FROM users WHERE age > 25;
SELECT id, email FROM orders ORDER BY id;
SELECT name FROM products WHERE price >= 10 LIMIT 5;
SELECT title, price FROM products WHERE price != 0 ORDER BY title LIMIT 20;

INSERT INTO users (name, age) VALUES ("Alice", 30);
INSERT INTO orders (product, qty, price) VALUES ("Widget", 5, 9.99), ("Gadget", 2, 24.50);

CREATE TABLE users (id int PRIMARY KEY, name varchar NOT NULL, age int);
CREATE TABLE orders (id int PRIMARY KEY NOT NULL, total float, active boolean);

DROP TABLE users;
DROP TABLE orders;
```

## Files

- `include/Parsers.hpp` — your code goes here
- `include/ParseSequence.hpp` — declarative parsing toolkit (you may add primitives)
- `include/Parser.hpp` — defines `ParsedCommand` with the fields you need to populate
- `include/Token.hpp` — token types and helpers
- `include/Tokenizer.hpp` — splits SQL strings into tokens
- `include/TestHarness.hpp` — test runner (read-only)
