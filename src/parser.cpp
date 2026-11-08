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
        size_t end = tokens.size();
        if (end > 0 && tokens[end - 1].kind == TokenKind::Semicolon) {
            --end;
        }
        if (type != "insert" && end == tokens.size()) {
            throw ParseError(tokens.size() - 1, "Expected ; at the end of statement");
        }

        if (type == "insert") {
            if (i >= end || tokens[i].kind != TokenKind::Identifier ||
                 !equalsIgnoreCase(tokens[i++].text, "into")) {
                throw ParseError(i - 1, "Expected INTO after INSERT");
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
                string Colname=tokens[i].text;
                transform(Colname.begin(), Colname.end(), Colname.begin(), toLower);
                insert.columns.push_back(Colname);
                ++i;
                while (i < end && tokens[i].kind == TokenKind::Comma) {
                    ++i;
                    if (i >= end || tokens[i].kind != TokenKind::Identifier) {
                        throw ParseError(i, "Expected column name after comma");
                    }
                    Colname=tokens[i].text;
                    transform(Colname.begin(), Colname.end(), Colname.begin(), toLower);
                    insert.columns.push_back(Colname);
                    ++i;
                }
                if (i >= end || tokens[i++].kind != TokenKind::RightParen) {
                    throw ParseError(i - 1, "Expected ) after column list");
                }
            }
            set <string> uniqueColumns;
            for (const auto& column : insert.columns) {
                if (!uniqueColumns.insert(column).second) {
                    throw ParseError(i, "Duplicate column name: " + column);
                }
            }
            if (i >= end || tokens[i].kind != TokenKind::Identifier ||
                !equalsIgnoreCase(tokens[i++].text, "values")) {
                throw ParseError(i - 1, "Expected VALUES after table name or column list");
            }
            if (i >= end || tokens[i++].kind != TokenKind::LeftParen) {
                throw ParseError(i - 1, "Expected ( after VALUES");
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
            
            if (i >= end || tokens[i++].kind != TokenKind::RightParen) {
                throw ParseError(i - 1, "Expected ) after values");
            }
            if(insert.values.size()!= insert.columns.size() && !insert.columns.empty()){
                throw ParseError(i, "Number of values does not match number of columns");
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
            if(i + 1 >= end || tokens[i++].kind != TokenKind::Identifier){
                throw ParseError(i - 1, "Expected table name after CREATE TABLE");
            }
            tableName = tokens[i - 1].text;
            transform(tableName.begin(), tableName.end(), tableName.begin(), toLower);
            if(tokens[i++].kind != TokenKind::LeftParen){
                throw ParseError(i - 1, "Expected ( after table name");
            }
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
            if(i >= end || tokens[i++].kind != TokenKind::RightParen){
                throw ParseError(i - 1, "Expected ) after column definitions");
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
            if(i>=end || (!equalsIgnoreCase(tokens[i++].text, "*") && tokens[i].kind != TokenKind::Identifier)){
                throw ParseError(i - 1, "Expected * or column name after SELECT");
            }
            if(i>=end || !equalsIgnoreCase(tokens[i++].text, "from")){
                throw ParseError(i - 1, "Expected FROM keyword after SELECT");
            }
            if(tokens[i].kind != TokenKind::Identifier){
                throw ParseError(i, "Expected table name after FROM");
            }
        }
        
        Statement result{};
        return result;
    }
}
