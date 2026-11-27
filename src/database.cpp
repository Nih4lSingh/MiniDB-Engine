#include <minidb/database.hpp>

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdint>

using namespace std;

namespace minidb {

    bool valueMatchesType(const Token& value, DataType type) {
        if (value.kind == TokenKind::Null) {
            return true;
        }
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
            case TokenKind::Null: return "null";
            default:                return "non-literal";
        }   
    }

    template <typename T>
    bool check(const T& left, const T& right, TokenKind op) {
        switch (op) {
            case TokenKind::EqualTo:          return left == right;
            case TokenKind::NotEqualTo:       return left != right;
            case TokenKind::GreaterThan:      return left > right;
            case TokenKind::GreaterThanEqual: return left >= right;
            case TokenKind::LesserThan:       return left < right;
            case TokenKind::LesserThanEqual:  return left <= right;
            default: break;
        }
        return false;
    }

    void Database::save(const string& filePath) const {
        ofstream out(filePath, ios::binary);

        if (!out) {
            throw ExecutionError("Could not open database file for writing");
        }

        const char marker[] = {'M', 'D', 'B', '1'};
        out.write(marker, sizeof(marker));

        const uint64_t tableCount = tables_.size();
        out.write(
            reinterpret_cast<const char*>(&tableCount),
            sizeof(tableCount)
        );

        auto writeString = [&out](const std::string& value) {
            const std::uint64_t length = value.size();

            out.write(
                reinterpret_cast<const char*>(&length),
                sizeof(length)
            );
            out.write(
                value.data(),
                static_cast<std::streamsize>(value.size())
            );
        };

        for (const auto& entry : tables_) {
            writeString(entry.first);

            const std::uint64_t columnCount = entry.second.columns.size();
            out.write(
                reinterpret_cast<const char*>(&columnCount),
                sizeof(columnCount)
            );
            for(const auto& column : entry.second.columns){
                writeString(column.name);
                writeString(dataTypeName(column.type));
            }

            const std::uint64_t rowCount = entry.second.rows.size();
            out.write(
                reinterpret_cast<const char*>(&rowCount),
                sizeof(rowCount)
            );
            for(const auto& row : entry.second.rows){
                for(const auto& rowData : row){
                    writeString(valueTypeName(rowData.kind));
                    writeString(rowData.text);
                }
            }
        }

        if (!out) {
            throw ExecutionError("Failed to write database file");
        }
    }

    void Database::createTable(const CreateStatement& statement) {
        if (tables_.find(statement.tableName) != tables_.end()) {
            throw ExecutionError("Table already exists: " + statement.tableName);
        }

        tables_.emplace(statement.tableName, Table{statement.columns,{}});
    }

    void Database::insertTable(const InsertStatement& statement){
        auto tableIt = tables_.find(statement.tableName);
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
        
        Token nullValue{TokenKind::Null, "null", 0};
        vector <Token> row(table.columns.size(), nullValue);

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

            switch (targetColumns[i].type) {
                case DataType::Float:
                    if(values[i].kind != TokenKind::Null && find(values[i].text.begin(),values[i].text.end(),'.') == values[i].text.end()){
                        values[i].text += ".0";
                    }
                    break;
                case DataType::Integer:
                    if(find(values[i].text.begin(),values[i].text.end(),'.') != values[i].text.end()){
                        throw ExecutionError("Excpected Integer, got Float: '" + targetColumns[i].name +"'");
                    }
                    break;
                case DataType::Text:
                    break;
            }
            row[idx] = values[i];
        }
        table.rows.push_back(row);
    }

    void Database::selectTable(const SelectStatement& statement){
        auto tableIt = tables_.find(statement.tableName);
        if(tableIt == tables_.end()){
            throw ExecutionError("Table doesn't exist: " + statement.tableName);
        }
        const Table& table = tableIt->second;

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

        bool decision = false;
        int LcolIndex = -1;
        int RcolIndex = -1;
        if(statement.condition){

            if(statement.expression.LHS.kind != TokenKind::Identifier 
                && statement.expression.LHS.kind != TokenKind::String
                && statement.expression.LHS.kind != TokenKind::Number){
                    throw ExecutionError("Invalid Token at LHS, expected column name or literal");
                };

            if(statement.expression.RHS.kind != TokenKind::Identifier 
                && statement.expression.RHS.kind != TokenKind::String
                && statement.expression.RHS.kind != TokenKind::Number){
                    throw ExecutionError("Invalid Token at RHS, expected column name or literal");
                };
            
            if(statement.expression.LHS.kind != TokenKind::Identifier 
                && statement.expression.RHS.kind != TokenKind::Identifier){

                if(statement.expression.RHS.kind != statement.expression.LHS.kind){
                    throw ExecutionError("Datatype mismatch");
                }

                if(statement.expression.LHS.kind == TokenKind::Number){
                    double L = stod(statement.expression.LHS.text);
                    double R = stod(statement.expression.RHS.text);

                    switch(statement.expression.Operator.kind){
                        case TokenKind::EqualTo: decision = L == R; break;
                        case TokenKind::NotEqualTo: decision = L != R; break;
                        case TokenKind::GreaterThan: decision = L > R; break;
                        case TokenKind::GreaterThanEqual: decision = L >= R; break;
                        case TokenKind::LesserThan: decision = L < R; break;
                        case TokenKind::LesserThanEqual: decision = L <= R; break;
                        default: break;
                    }
                }

                if(statement.expression.LHS.kind == TokenKind::String){
                    string L = statement.expression.LHS.text;
                    string R = statement.expression.RHS.text;

                    switch(statement.expression.Operator.kind){
                        case TokenKind::EqualTo: decision = L == R; break;
                        case TokenKind::NotEqualTo: decision = L != R; break;
                        case TokenKind::GreaterThan: decision = L > R; break;
                        case TokenKind::GreaterThanEqual: decision = L >= R; break;
                        case TokenKind::LesserThan: decision = L < R; break;
                        case TokenKind::LesserThanEqual: decision = L <= R; break;
                        default: break;
                    }
                }
            }

            if(statement.expression.LHS.kind == TokenKind::Identifier){
                string name = statement.expression.LHS.text;
                auto columnIt = find_if(table.columns.begin(), table.columns.end(),
                    [&](const ColumnDefinition& column) {
                        return column.name == name;
                    });
                if(columnIt == table.columns.end()){
                    throw ExecutionError("Non exisiting column on LHS: " + name);
                }
                LcolIndex = columnIt - table.columns.begin();
            }

            if(statement.expression.RHS.kind == TokenKind::Identifier){
                string name = statement.expression.RHS.text;
                auto columnIt = find_if(table.columns.begin(), table.columns.end(),
                    [&](const ColumnDefinition& column) {
                        return column.name == name;
                    });
                if(columnIt == table.columns.end()){
                    throw ExecutionError("Non exisiting column on RHS: " + name);
                }
                RcolIndex = columnIt - table.columns.begin();
            }

            if (statement.expression.LHS.kind == TokenKind::Identifier 
                && statement.expression.RHS.kind == TokenKind::Identifier){
                string Lname = statement.expression.LHS.text, Rname = statement.expression.RHS.text;

                auto Lcolumn = *find_if(table.columns.begin(), table.columns.end(),
                    [&](const ColumnDefinition& column) {
                        return column.name == Lname;
                    });
                auto Rcolumn = *find_if(table.columns.begin(), table.columns.end(),
                    [&](const ColumnDefinition& column) {
                        return column.name == Rname;
                    });
                
                if((Lcolumn.type != Rcolumn.type) 
                    && (Rcolumn.type == DataType::Text 
                    || Lcolumn.type == DataType::Text)){
                    throw ExecutionError("Column datatype mismatch");
                }
            }
            else if(statement.expression.LHS.kind == TokenKind::Identifier){
                string Lname = statement.expression.LHS.text;

                auto Lcolumn = *find_if(table.columns.begin(), table.columns.end(),
                    [&](const ColumnDefinition& column) {
                        return column.name == Lname;
                    });
                if(Lcolumn.type == DataType::Text && statement.expression.RHS.kind != TokenKind::String){
                    throw ExecutionError("Column datatype mismatch");
                }
                if((Lcolumn.type == DataType::Float || Lcolumn.type == DataType::Integer) 
                    && statement.expression.RHS.kind != TokenKind::Number){
                    throw ExecutionError("Column datatype mismatch");
                }
            }
            else if(statement.expression.RHS.kind == TokenKind::Identifier){
                string Rname = statement.expression.RHS.text;

                auto Rcolumn = *find_if(table.columns.begin(), table.columns.end(),
                    [&](const ColumnDefinition& column) {
                        return column.name == Rname;
                    });
                if(Rcolumn.type == DataType::Text && statement.expression.LHS.kind != TokenKind::String){
                    throw ExecutionError("Column datatype mismatch");
                }
                if((Rcolumn.type == DataType::Float || Rcolumn.type == DataType::Integer) 
                    && statement.expression.LHS.kind != TokenKind::Number){
                    throw ExecutionError("Column datatype mismatch");
                }
            }
        }

        for (const auto& column : targetColumns){
            cout<<"| "+column.name+" | ";
        }
        cout<<'\n';

        vector <size_t> colIndex;
        for (size_t i = 0; i < targetColumns.size(); i++){
            size_t idx = (find_if(table.columns.begin(), table.columns.end(),
                [&](const ColumnDefinition& column) {
                    return column.name == targetColumns[i].name;
                    })) - table.columns.begin();
            colIndex.push_back(idx);
        }

        for (const auto& row : table.rows) {
            bool matches = true;

            if (statement.condition) {
                const auto& LHS = statement.expression.LHS;
                const auto& RHS = statement.expression.RHS;
                auto op = statement.expression.Operator.kind;

                if (LHS.kind != TokenKind::Identifier && RHS.kind != TokenKind::Identifier) {
                    matches = decision;
                }
                else if (LHS.kind == TokenKind::Identifier &&
                        RHS.kind == TokenKind::Identifier) {
                    if(row[LcolIndex].kind == TokenKind::Null || row[RcolIndex].kind == TokenKind::Null){
                        continue;
                    }
                    if (table.columns[LcolIndex].type == DataType::Text) {
                        matches = check(row[LcolIndex].text, row[RcolIndex].text, op);
                    } else {
                        double L = stod(row[LcolIndex].text);
                        double R = stod(row[RcolIndex].text);
                        matches = check(L, R, op);
                    }
                }
                else if (LHS.kind == TokenKind::Identifier) {
                    if(row[LcolIndex].kind == TokenKind::Null){
                        continue;
                    }
                    if (RHS.kind == TokenKind::String) {
                        matches = check(row[LcolIndex].text, RHS.text, op);
                    } else {
                        double L = stod(row[LcolIndex].text);
                        double R = stod(RHS.text);
                        matches = check(L, R, op);
                    }
                }
                else {
                    if(row[RcolIndex].kind == TokenKind::Null){
                        continue;
                    }
                    if (LHS.kind == TokenKind::String) {
                        matches = check(LHS.text, row[RcolIndex].text, op);
                    } else {
                        double L = stod(LHS.text);
                        double R = stod(row[RcolIndex].text);
                        matches = check(L, R, op);
                    }
                }
            }

            if (!matches) {
                continue;
            }

            for (size_t idx : colIndex) {
                cout << "| " << row[idx].text << " | ";
            }
            cout << '\n';
        }
    }
}