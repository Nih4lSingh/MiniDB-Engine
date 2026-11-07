#include <minidb/parser.hpp>

#include <cctype>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <array>

using namespace std;
bool equalsIgnoreCase(string left, string right) {
    auto toLower = [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    };

    transform(left.begin(), left.end(), left.begin(), toLower);
    return left == right;
}
namespace minidb {
    ParseError::ParseError(size_t tokenIndex, const string& message)
    : runtime_error("at token " + to_string(tokenIndex) + ": " + message),
      tokenIndex_(tokenIndex) {}

    size_t ParseError::tokenIndex() const noexcept {
        return tokenIndex_;
    }
    Statement parseStatement(const vector<Token>& tokens) {
        vector<int> parenStack;
        string type;
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
            size_t i = 1;
            if (i >= end || tokens[i].kind != TokenKind::Identifier ||
                 !equalsIgnoreCase(tokens[i++].text, "into")) {
                throw ParseError(i - 1, "Expected INTO after INSERT");
            }
            if (i >= end || tokens[i].kind != TokenKind::Identifier) {
                throw ParseError(i, "Expected table name after INTO");
            }

            InsertStatement insert;
            insert.tableName = tokens[i++].text;

            if (i < end && tokens[i].kind == TokenKind::LeftParen) {
                ++i;
                if (i >= end || tokens[i].kind != TokenKind::Identifier) {
                    throw ParseError(i, "Expected column name");
                }
                insert.columns.push_back(tokens[i].text);
                ++i;
                while (i < end && tokens[i].kind == TokenKind::Comma) {
                    ++i;
                    if (i >= end || tokens[i].kind != TokenKind::Identifier) {
                        throw ParseError(i, "Expected column name after comma");
                    }
                    insert.columns.push_back(tokens[i].text);
                    ++i;
                }
                if (i >= end || tokens[i++].kind != TokenKind::RightParen) {
                    throw ParseError(i - 1, "Expected ) after column list");
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
            if (i != end) throw ParseError(i, "Unexpected token after INSERT statement");

            Statement result{};
            result.kind = Statement::Kind::Insert;
            result.insert = insert;
            return result;
        }

        if(type == "create" && !equalsIgnoreCase(tokens[1].text, "table")){
            throw ParseError(1, "Expected TABLE after CREATE");
        }
        if(type == "select" && !equalsIgnoreCase(tokens[1].text, "*") && tokens[1].kind != TokenKind::Identifier){
            throw ParseError(1, "Expected * or column name after SELECT");
        }
        if(type == "select" && !equalsIgnoreCase(tokens[2].text, "from")){
            throw ParseError(2, "Expected FROM keyword after SELECT");
        }
        if(type == "select" && tokens[3].kind != TokenKind::Identifier){
            throw ParseError(3, "Expected table name after FROM");
        }
        if(type == "create" && tokens[2].kind != TokenKind::Identifier){
            throw ParseError(2, "Expected table name after CREATE TABLE");
        }
        if(type == "create"){
            if(tokens[3].kind != TokenKind::LeftParen){
                throw ParseError(3, "Expected ( after table name");
            }
            parenStack.push_back(3);
        }
        if(type == "create"){
            if(tokens[tokens.size()-2].kind != TokenKind::RightParen){
                throw ParseError(tokens.size() - 2, "Expected ) at the end of statement");
            }
            parenStack.pop_back();
        }
        Statement result{};
        if (type == "create") {
            result.kind = Statement::Kind::CreateTable;
            result.createTable = CreateTableStatement{tokens[2].text, {}};
        } else if (type == "select") {
            result.kind = Statement::Kind::Select;
            result.select = SelectStatement{{}, tokens[3].text};
        }
        return result;
    }
}
