#pragma once

#include <minidb/parser.hpp>

#include <vector>
#include <map>
#include <string>
#include <stdexcept>

namespace minidb{
    struct Table {
        std::vector<ColumnDefinition> columns;
        std::vector<std::vector<Token>> rows;
    };
    class ExecutionError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    class Database {
    public:
        void createTable(const CreateStatement& statement);
        void insertTable(const InsertStatement& statement);
        void selectTable(const SelectStatement& statement);
    private:
        std::map<std::string, Table> tables_;
    };

}