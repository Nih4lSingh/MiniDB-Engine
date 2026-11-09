#include <minidb/parser.hpp>

#include <cctype>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <array>
#include <set>

using namespace std;

bool equalsIgnoreCase(string left, string right) {
    auto toLower = [](unsigned char ch) {
        return static_cast<char>(tolower(ch));
    };

    transform(left.begin(), left.end(), left.begin(), toLower);
    return left == right;
}

auto toLower = [](unsigned char ch) {
        return static_cast<char>(tolower(ch));
    };

namespace minidb {
    ParseError::ParseError(size_t tokenIndex, const string& message)
    : runtime_error("at token " + to_string(tokenIndex) + ": " + message),
      tokenIndex_(tokenIndex) {}

    size_t ParseError::tokenIndex() const noexcept {
        return tokenIndex_;
    }
    Statement parseStatement(const vector<Token>& tokens) {
        
        string type;
        size_t i = 1;
        if(tokens.size()<2){
            throw ParseError(0, "Expected statement type (CREATE, INSERT, SELECT)");
        }
        if(tokens[0].kind != TokenKind::Identifier){
            throw ParseError(0, "Expected statement type (CREATE, INSERT, SELECT)");
        }
        for (const auto& statementType : array<string, 3>{"create", "insert", "select"}) {
            if (equalsIgnoreCase(tokens[0].text, statementType)) {
                type = statementType;
                break;
            }
        }
        if (type.empty()) {
            throw ParseError(0, "Expected statement type (CREATE, INSERT, SELECT)");
        }
        if (tokens.back().kind != TokenKind::Semicolon) {
            throw ParseError(tokens.size(), "Expected ; at the end of statement");
        }
        size_t end = tokens.size() - 1;


        if (type == "insert") {
            vector<size_t> insertColumnTokenIndexes;
            if (i >= end || tokens[i].kind != TokenKind::Identifier ||
                 !equalsIgnoreCase(tokens[i++].text, "into")) {
                throw ParseError(i == 1 ? i : i - 1, "Expected INTO after INSERT");
            }
            if (i >= end || tokens[i].kind != TokenKind::Identifier) {
                throw ParseError(i, "Expected table name after INTO");
            }

            InsertStatement insert;
            string tableName = tokens[i].text;
            transform(tableName.begin(), tableName.end(), tableName.begin(), toLower);
            insert.tableName = tableName;
            ++i;

            if (i < end && tokens[i].kind == TokenKind::LeftParen) {
                ++i;
                if (i >= end || tokens[i].kind != TokenKind::Identifier) {
                    throw ParseError(i, "Expected column name");
                }
                size_t columnTokenIndex = i;
                string Colname=tokens[i].text;
                transform(Colname.begin(), Colname.end(), Colname.begin(), toLower);
                insert.columns.push_back(Colname);
                insertColumnTokenIndexes.push_back(columnTokenIndex);
                ++i;
                while (i < end && tokens[i].kind == TokenKind::Comma) {
                    ++i;
                    if (i >= end || tokens[i].kind != TokenKind::Identifier) {
                        throw ParseError(i, "Expected column name after comma");
                    }
                    columnTokenIndex = i;
                    Colname=tokens[i].text;
                    transform(Colname.begin(), Colname.end(), Colname.begin(), toLower);
                    insert.columns.push_back(Colname);
                    insertColumnTokenIndexes.push_back(columnTokenIndex);
                    ++i;
                }
                if (i >= end || tokens[i++].kind != TokenKind::RightParen) {
                    throw ParseError(i - 1, "Expected ) after column list");
                }
            }
            set <string> uniqueColumns;
            for (const auto& column : insert.columns) {
                if (!uniqueColumns.insert(column).second) {
                    size_t duplicateIndex = static_cast<size_t>(&column - insert.columns.data());
                    throw ParseError(insertColumnTokenIndexes[duplicateIndex], "Duplicate column name: " + column);
                }
            }
            size_t expectedValuesIndex = i;
            if (i >= end || tokens[i].kind != TokenKind::Identifier ||
                !equalsIgnoreCase(tokens[i++].text, "values")) {
                throw ParseError(i == expectedValuesIndex ? i : i - 1, "Expected VALUES after table name or column list");
            }
            size_t expectedValuesParenIndex = i;
            if (i >= end || tokens[i++].kind != TokenKind::LeftParen) {
                throw ParseError(i == expectedValuesParenIndex ? i : i - 1, "Expected ( after VALUES");
            }

            while (true) {
                if (i >= end) throw ParseError(i, "Expected a value");

                Token value;
                if (tokens[i].kind == TokenKind::Number || tokens[i].kind == TokenKind::String) {
                    value = tokens[i++];
                } else if ((tokens[i].kind == TokenKind::Plus || tokens[i].kind == TokenKind::Minus) &&
                           i + 1 < end && tokens[i + 1].kind == TokenKind::Number) {
                    value = tokens[i++];
                    value.kind = TokenKind::Number;
                    value.text += tokens[i++].text;
                } else {
                    throw ParseError(i, "Expected a number or string literal");
                }
                insert.values.push_back(value);

                if (i >= end || tokens[i].kind != TokenKind::Comma) break;
                ++i;
            }
            
            size_t expectedValuesCloseIndex = i;
            if (i >= end || tokens[i++].kind != TokenKind::RightParen) {
                throw ParseError(i == expectedValuesCloseIndex ? i : i - 1, "Expected ) after values");
            }
            if(insert.values.size()!= insert.columns.size() && !insert.columns.empty()){
                throw ParseError(i - 1, "Number of values does not match number of columns");
            }
            if (i != end) throw ParseError(i, "Unexpected token after INSERT statement");

            Statement result{};
            result.kind = Statement::Kind::Insert;
            result.insert = insert;
            return result;
        }


        if(type == "create"){
            string tableName;
            if(i >= end || !equalsIgnoreCase(tokens[i].text, "table") || 
            tokens[i].kind != TokenKind::Identifier){
                throw ParseError(i, "Expected TABLE after CREATE");
            }
            i++;
            if(i >= end || tokens[i].kind != TokenKind::Identifier){
                throw ParseError(i, "Expected table name after CREATE TABLE");
            }
            tableName = tokens[i++].text;
            transform(tableName.begin(), tableName.end(), tableName.begin(), toLower);
            if(i >= end || tokens[i].kind != TokenKind::LeftParen){
                throw ParseError(i, "Expected ( after table name");
            }
            i++;
            CreateTableStatement create;
            create.tableName = tableName;
            map<string, int> columnNames;
            while(i < end && tokens[i].kind != TokenKind::RightParen){
                if(tokens[i].kind != TokenKind::Identifier){
                    throw ParseError(i, "Expected column name");
                }
                ColumnDefinition colDef;
                string ColName = tokens[i++].text;
                transform(ColName.begin(), ColName.end(), ColName.begin(), toLower);
                colDef.name = ColName;
                if(columnNames[ColName] > 0){
                    throw ParseError(i - 1, "Duplicate column name: " + ColName);
                }
                columnNames[ColName]++;
                if(i >= end || tokens[i].kind != TokenKind::Identifier){
                    throw ParseError(i, "Expected data type after column name");
                }
                string datatype = tokens[i++].text;
                transform(datatype.begin(), datatype.end(), datatype.begin(), toLower);

                colDef.type = (datatype == "int") ? DataType::Integer :
                              (datatype == "float") ? DataType::Float :
                              (datatype == "text") ? DataType::Text : throw ParseError(i - 1, "Unknown data type: " + datatype);
                create.columns.push_back(colDef);
                if (i < end && tokens[i].kind == TokenKind::Comma) {
                    ++i;
                    if (i >= end || tokens[i].kind == TokenKind::RightParen) {
                        throw ParseError(i, "Expected column definition after comma");
                    }
                    continue;
                }
                if (i < end && tokens[i].kind == TokenKind::RightParen) {
                    break;
                }
                if(i < end && tokens[i].kind != TokenKind::Comma && tokens[i].kind != TokenKind::RightParen){
                    throw ParseError(i, "Expected , or ) after column definition");
                }
            }
            size_t expectedColumnCloseIndex = i;
            if(i >= end || tokens[i++].kind != TokenKind::RightParen){
                throw ParseError(i == expectedColumnCloseIndex ? i : i - 1, "Expected ) after column definitions");
            }
            if(create.columns.empty()){
                throw ParseError(i - 1, "Expected at least one column definition");
            }
            if(i != end){
                throw ParseError(i, "Unexpected token after CREATE TABLE statement");
            }
            Statement result{};
            result.kind = Statement::Kind::CreateTable;
            result.createTable = create;
            return result;
        }


        if(type == "select"){
            bool Star = false;
            SelectStatement select;
            if(i < end && tokens[i].kind == TokenKind::Identifier &&
               equalsIgnoreCase(tokens[i].text, "from")){
                throw ParseError(i, "Expected * or column name after SELECT");
            }
            if(i>=end || (tokens[i].kind != TokenKind::Star && tokens[i].kind != TokenKind::Identifier)){
                throw ParseError(i, "Expected * or column name after SELECT");
            }
            if(tokens[i].kind == TokenKind::Star){
                Star = true;
                i++;
            }
            if(!Star){
                string Colname = tokens[i++].text;
                transform(Colname.begin(), Colname.end(), Colname.begin(), toLower);
                select.columns.push_back(Colname);
                while(i < end && tokens[i].kind == TokenKind::Comma){
                    i++;
                    if(i>=end || tokens[i].kind != TokenKind::Identifier ||
                       equalsIgnoreCase(tokens[i].text, "from")){
                        throw ParseError(i, "Expected column name after comma");
                    }
                    Colname = tokens[i++].text;
                    transform(Colname.begin(), Colname.end(), Colname.begin(), toLower);
                    select.columns.push_back(Colname);
                }
            }
            if(i>=end ||tokens[i].kind != TokenKind::Identifier || !equalsIgnoreCase(tokens[i].text, "from")){
                throw ParseError(i, "Expected FROM keyword after selected columns");
            }
            i++;
            if(i>=end || tokens[i].kind != TokenKind::Identifier){
                throw ParseError(i, "Expected table name after FROM");
            }
            string tableName = tokens[i].text;
            transform(tableName.begin(), tableName.end(), tableName.begin(), toLower);
            select.tableName = tableName;
            i++;
            if(i != end){
                throw ParseError(i, "Unexpected token after SELECT statement");
            }
            Statement result{};
            result.kind = Statement::Kind::Select; 
            result.select = select;
            return result;
        }
        
        Statement result{};
        return result;
    }
}
