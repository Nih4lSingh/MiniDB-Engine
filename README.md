# MiniDB-Engine

MiniDB-Engine is a small C++17 learning project focused on building an in-memory SQL database engine from scratch. The project is being developed incrementally to understand tokenization, parsing, query execution, and database internals.

## Implemented so far

- **Tokenizer:** Tokenizes identifiers, numeric literals, single-quoted strings, punctuation, comparison operators, and the SQL `NULL` literal. Identifiers are normalized to lowercase; `NULL` is recognized without regard to case, while quoted `'NULL'` remains a string.
- **Parser:** Uses recursive-descent parsing for one SQL statement at a time:
  - `CREATE TABLE`
  - `INSERT INTO ... VALUES`
  - `SELECT * FROM ...`
  - `SELECT column1, column2 FROM ...`
  - An optional, single-condition `WHERE operand operator operand` clause. Operands may be identifiers, numeric literals (including signed numbers), or strings. Supported operators are `=`, `!=`, `<`, `<=`, `>`, and `>=`.
- **Table creation:** Creates in-memory tables supporting `INT`, `FLOAT`, and `TEXT` column types. Table and column names are normalized to lowercase.
- **Data insertion:** Inserts rows into existing tables, supports insertion with or without explicitly specifying column names, validates column existence and data types, and initializes omitted columns to `NULL`.
- **Numeric handling:** Decimal values are rejected for `INT` columns. Integer-looking values inserted into `FLOAT` columns are normalized for display (for example, `23` becomes `23.0`). Values are currently stored as token text.
- **SELECT execution:** Displays all rows or selected columns from a table. Supports arbitrary column ordering and duplicate column selection, and validates table and column existence. Although the parser accepts a `WHERE` condition, SELECT execution does not yet use it to filter rows.
- **Multiline CLI input:** Supports SQL statements spanning multiple lines. The CLI accumulates input until it encounters a semicolon outside a single-quoted string.
- **String handling:** The CLI recognizes escaped single quotes (`''`) when detecting the end of a query.
- **Error handling:** Reports tokenization, parsing, and execution errors separately.

## Current limitations

- Data is stored entirely in memory and is lost when the program exits. File persistence and indexing are not implemented.
- Only one SQL statement is processed per query. Multiple statements separated by semicolons on the same line are not yet supported.
- WHERE syntax is parsed but its condition is not yet applied to SELECT results. Multiple conditions (`AND`/`OR`), arithmetic expressions, and NULL comparisons are not supported.
- There is no `NOT NULL` constraint; every column currently permits `NULL`.
- Numeric values are stored as text tokens rather than native numeric values.
- Query results are printed directly to the console rather than returned through a separate result-set interface.
- The CLI displays query results but does not yet provide formatted, column-aligned table output.

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

## Usage

MiniDB supports single-line and multiline SQL statements. Each statement must end with a semicolon.

### Create a table

```sql
CREATE TABLE students (
    id INT,
    name TEXT,
    age INT,
    marks FLOAT
);
```

### Insert records

```sql
INSERT INTO students VALUES (1, 'Nihal', 20, 89.5);

INSERT INTO students (id, name)
VALUES (2, 'Rahul');
```

Unspecified columns are automatically initialized to `NULL`.

### Select records

Retrieve all columns:

```sql
SELECT * FROM students;
```

Retrieve specific columns:

```sql
SELECT name, age FROM students;
```

Retrieve columns in a different order:

```sql
SELECT marks, name, id FROM students;
```

The parser accepts a single comparison condition such as `SELECT * FROM students WHERE age >= -1;`, but SELECT currently displays all rows without applying that condition.

### Multiline input

The CLI accepts statements across multiple lines and processes them once a terminating semicolon is entered outside a quoted string.

```sql
CREATE TABLE employees (
    emp_id INT,
    name TEXT,
    salary FLOAT
);
```

Semicolons inside string literals are supported:

```sql
INSERT INTO students VALUES (3, 'Hello; World', 21, 90.0);
```

## Project status

MiniDB supports the basic SQL workflow of creating tables, inserting records, and retrieving selected columns. The parser also accepts a basic WHERE condition; the next development milestone is applying that condition to filter SELECT results, followed by additional query capabilities.

