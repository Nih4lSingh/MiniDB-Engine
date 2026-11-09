#include <minidb/database.hpp>

#include <cctype>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

namespace minidb {

    bool valueMatchesType(const Token& value, DataType type) {
        switch (type) {
            case DataType::Integer:
                return value.kind == TokenKind::Number;
            case DataType::Float:
                return value.kind == TokenKind::Number;
            case DataType::Text:
                return value.kind == TokenKind::String;
        }
        return false;
    }
    string dataTypeName(DataType type) {
        switch (type) {
            case DataType::Integer: return "INT";
            case DataType::Float:   return "FLOAT";
            case DataType::Text:    return "TEXT";
        }
        return "";
    }

    string valueTypeName(TokenKind kind) {
        switch (kind) {
            case TokenKind::Number: return "number";
            case TokenKind::String: return "text";
            default:                return "non-literal";
        }
    }

    void Database::createTable(const CreateTableStatement& statement) {
        if (tables_.find(statement.tableName) != tables_.end()) {
            throw ExecutionError("Table already exists: " + statement.tableName);
        }

        tables_.emplace(statement.tableName, Table{statement.columns});
    }

    void Database::insertTable(const InsertStatement& statement){
        auto tableIt = tables_.find(statement.tableName);
        // After checking it isn't tables_.end():
        
        if(tableIt == tables_.end()){
            throw ExecutionError("Table doesn't exist: " + statement.tableName);
        }
        Table& table = tableIt->second;
        vector<Token>values = statement.values;
        vector<ColumnDefinition> targetColumns;

        if(statement.columns.empty()){
            targetColumns = table.columns;
        }
        else{
            for(const auto& name : statement.columns){
                auto columnIt = find_if(table.columns.begin(), table.columns.end(),
                    [&](const ColumnDefinition& column) {
                        return column.name == name;
                    });
                if(columnIt == table.columns.end()){
                    throw ExecutionError("Non exisiting column: " + name);
                }
                targetColumns.push_back(*columnIt);
            }
        }
        if (values.size() != targetColumns.size()) {
            throw ExecutionError("Number of values does not match number of columns");
        }

        vector <Token> row(table.columns.size());

        for (size_t i = 0; i < values.size(); i++){
            if(!valueMatchesType(values[i],targetColumns[i].type)){
                throw ExecutionError(
                    "Datatype mismatch for column '" + targetColumns[i].name +
                    "': expected " + dataTypeName(targetColumns[i].type) +
                    ", got " + valueTypeName(values[i].kind));
            }
            size_t idx = (find_if(table.columns.begin(), table.columns.end(),
                [&](const ColumnDefinition& column) {
                    return column.name == targetColumns[i].name;
                    })) - table.columns.begin();
            row[idx] = values[i];
        }
        table.rows.push_back(row);
    }

}