#pragma once

#include <minidb/tokenizer.hpp>
#include <stdexcept>
#include <vector>
#include <string>

namespace minidb {
    enum class DataType {
        Integer,
        Float,
        Text
    };

    struct ColumnDefinition {
        std::string name;
        DataType type;
    };

    struct CreateTableStatement {
        std::string tableName;
        std::vector<ColumnDefinition> columns;
    };

    struct InsertStatement {
        std::string tableName;
        std::vector<std::string> columns; // Optional list of columns to insert into. If empty, all columns are assumed.
        std::vector<Token> values;
    };

    struct Where{
        minidb::Token LHS;
        minidb::Token Operator;
        minidb::Token RHS;
    };

    struct SelectStatement {
        // An empty list will mean SELECT *.
        std::vector<std::string> columns;
        std::string tableName;
        bool condition = false;
        Where expression;
    };

    struct Statement {
        enum class Kind {
            CreateTable,
            Insert,
            Select
        };

        Kind kind;
        CreateTableStatement createTable;
        InsertStatement insert;
        SelectStatement select;
    };

    class ParseError : public std::runtime_error {
    public:
        ParseError(std::size_t tokenIndex, const std::string& message);
        std::size_t tokenIndex() const noexcept;

    private:
        std::size_t tokenIndex_;
    };

    // Parses exactly one SQL statement. A trailing semicolon is optional.
    Statement parseStatement(const std::vector<Token>& tokens);

}
