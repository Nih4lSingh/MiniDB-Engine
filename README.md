# MiniDB-Engine

MiniDB-Engine is a small C++17 learning project focused on building an in-memory SQL database engine from scratch. The project is being developed incrementally to understand tokenization, parsing, query execution, and database internals.

## Implemented so far

- **Tokenizer:** Tokenizes identifiers, numeric literals, single-quoted strings, punctuation, and the SQL `NULL` literal. `NULL` is recognized without regard to case; quoted `'NULL'` remains a string.
- **Parser:** Uses a recursive-descent parsing approach to parse one SQL statement at a time:
  - `CREATE TABLE`
  - `INSERT INTO ... VALUES`
  - `SELECT * FROM ...`
  - `SELECT column1, column2 FROM ...`
- **Table creation:** Creates in-memory tables supporting `INT`, `FLOAT`, and `TEXT` column types. Table and column names are normalized to lowercase.
- **Data insertion:** Inserts rows into existing tables, supports insertion with or without explicitly specifying column names, validates column existence and data types, and initializes omitted columns to `NULL`.
- **SELECT execution:** Retrieves all columns or selected columns from a table. Supports arbitrary column ordering and duplicate column selection. Validates table and column existence.
- **Multiline CLI input:** Supports SQL statements spanning multiple lines. The CLI accumulates input until it encounters a semicolon outside a single-quoted string.
- **String handling:** The CLI recognizes escaped single quotes (`''`) when detecting the end of a query.
- **Error handling:** Reports tokenization, parsing, and execution errors separately.

## Current limitations

- Data is stored entirely in memory and is lost when the program exits. File persistence and indexing are not implemented.
- Only one SQL statement is processed per query. Multiple statements separated by semicolons on the same line are not yet supported.
- `WHERE`, `ORDER BY`, `LIMIT`, and other advanced SQL clauses are not implemented.
- There is no `NOT NULL` constraint; every column currently permits `NULL`.
- Numeric type checking is basic: both `INT` and `FLOAT` accept numeric tokens.
- Query results are currently printed directly to the console rather than returned through a separate result-set interface.
- The CLI displays query results but does not yet provide formatted, column-aligned table output.

## Build and run

From the project root, compile the source files:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/main.cpp src/tokenizer.cpp src/parser.cpp src/database.cpp -o minidb.exe
.\minidb.exe
```

## Usage

MiniDB supports single-line and multiline SQL statements.

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

MiniDB currently supports the basic SQL workflow of creating tables, inserting records, and retrieving selected columns. The next development milestone is implementing the `WHERE` clause, followed by additional query capabilities.