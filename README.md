# MiniDB-Engine

MiniDB-Engine is a C++17 learning project for building a small SQL database engine from scratch. It currently supports basic schema creation, row insertion, and selection with a single comparison-based `WHERE` condition. Tables are held in memory while the program runs and saved to a binary database file for use across sessions.

## Implemented features

- **Tokenizer:** Recognizes identifiers, numeric literals, single-quoted strings, SQL punctuation, comparison operators, and the `NULL` value. Unquoted identifiers are normalized to lowercase; quoted string contents are preserved.
- **Recursive-descent parser:** Parses one SQL statement at a time:
  - `CREATE TABLE`
  - `INSERT INTO ... VALUES`
  - `SELECT *` or selected columns
  - Optional single-condition `WHERE operand operator operand`
- **Supported comparison operators:** `=`, `!=`, `<`, `<=`, `>`, and `>=`.
- **In-memory database:** Supports `INT`, `FLOAT`, and `TEXT` columns. It checks table and column existence, duplicate column names, value counts, and compatible values during insertion.
- **INSERT behavior:** Supports optional column lists. Columns omitted from an insert are initialized to `NULL`. Integer-looking values inserted into `FLOAT` columns are displayed with a decimal part, such as `23.0`.
- **SELECT behavior:** Reads all rows or selected columns, preserves requested column order, and applies the single `WHERE` comparison to filter rows. Numeric columns can be compared with other numeric columns, including `INT`/`FLOAT` pairs.
- **NULL behavior:** A row is skipped when a column referenced by the `WHERE` comparison contains a stored `NULL`. The parser currently rejects `NULL` as a `WHERE` operand; `IS NULL`, `AND`, and `OR` are not implemented.
- **Persistence and serialization:** `save()` serializes table names, column definitions, and row values to a binary file. `load()` restores them into the in-memory database at startup. The CLI saves after successful `CREATE TABLE` and `INSERT` statements. The save/reload round trip has been manually verified.
- **CLI:** Accepts multiline SQL input and processes statements sequentially in one session. Each statement must end with a semicolon. Multiple statements in one input query are not supported.
- **Errors:** Tokenization, parsing, and execution errors are reported separately.

## Build and run

The Windows build below uses CMake with the MinGW Makefiles generator. CMake and MinGW GCC/Make must be installed. Run these commands from the project root:

```powershell
& "C:\Program Files\CMake\bin\cmake.exe" -G "MinGW Makefiles" -S . -B build-mingw
& "C:\Program Files\CMake\bin\cmake.exe" --build build-mingw
.\build-mingw\minidb.exe
```

If CMake is on your `PATH`, you can use `cmake` instead of the full executable path.

Alternatively, compile the source files directly:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/main.cpp src/tokenizer.cpp src/parser.cpp src/database.cpp -o minidb.exe
.\minidb.exe
```

## Example

```sql
CREATE TABLE students (id INT, name TEXT, score FLOAT);
INSERT INTO students VALUES (1, 'Asha', 82.5);
INSERT INTO students VALUES (2, 'Ben', 67);
SELECT name, score FROM students WHERE score > 80;
```

The final query returns Asha's row.

## Current status and next steps

The basic in-memory SQL workflow, single-condition `WHERE` filtering, and binary persistence have been implemented and manually exercised. Table schemas and row data are serialized and restored across program sessions. Week 3 work remaining includes B+ tree indexing. Further planned work includes automated tests, `AND`/`OR` conditions, `IS NULL`, more robust error handling, and benchmarking.

Further planned work includes automated tests, `AND`/`OR` conditions, `IS NULL`, more robust error handling, and benchmarking.
