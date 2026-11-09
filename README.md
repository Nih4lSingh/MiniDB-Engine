# MiniDB-Engine

MiniDB-Engine is a small C++17 learning project for building an in-memory SQL database engine step by step.

## Implemented so far

- Tokenizes identifiers, numeric literals, single-quoted strings, punctuation, and the SQL `NULL` literal. `NULL` is recognized without regard to case; quoted `'NULL'` remains a string.
- Parses one SQL statement at a time: `CREATE TABLE`, `INSERT INTO ... VALUES`, and basic `SELECT` forms (`*` or a list of columns).
- Creates in-memory tables with `INT`, `FLOAT`, and `TEXT` columns. Table and column names are normalized to lowercase.
- Inserts rows into existing tables, checks that named columns exist and that values match the column type, and stores explicit or omitted values as `NULL`.
- Reports tokenization, parsing, and execution errors in the command-line interface.

## Current limitations

- Data is kept in memory and is lost when the program exits. There is no file persistence or index yet.
- The parser recognizes `SELECT`, but the CLI does not execute it or display table rows yet.
- There is no `NOT NULL` constraint; every column currently permits `NULL`.
- Numeric type checking is basic: both `INT` and `FLOAT` accept a numeric token.
- The CMake target currently lists only `main.cpp` and `tokenizer.cpp`. Until `parser.cpp` and `database.cpp` are added to `CMakeLists.txt`, build the full current program with the direct compile command below.

## Build and run

From the project root, compile all current source files:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/main.cpp src/tokenizer.cpp src/parser.cpp src/database.cpp -o minidb.exe
.\minidb.exe
```

Enter one SQL statement per line. For example:

```sql
CREATE TABLE students (id INT, name TEXT);
INSERT INTO students (id, name) VALUES (1, NULL);
INSERT INTO students (name) VALUES ('Aman');
```

The current CLI confirms successful table creation and inserts. It does not yet print stored rows for a `SELECT` statement.
