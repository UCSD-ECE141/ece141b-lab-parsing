# SQL Parsing Challenge

Parse SQL commands. 220 randomized tests. Make them pass.

`DropParser` is provided as an example. Implement `SelectParser`, `InsertParser`, and `CreateParser`.

Use the `ParseSequence` class in `include/ParseSequence.hpp` to describe your grammars declaratively instead of writing manual token-walking loops. You may need to extend `ParseSequence` with your own primitives.

## Commands

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

## Build and run

```
cmake -B build
cmake --build build
./build/lab-parsing
```

## Files

Edit `include/Parsers.hpp`. You may also add primitives to `include/ParseSequence.hpp`.
