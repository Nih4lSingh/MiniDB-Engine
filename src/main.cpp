#include <minidb/tokenizer.hpp>
#include <minidb/parser.hpp>

#include <iostream>
#include <string>
#include <map>

using namespace std;

map<minidb::TokenKind, string> tokenKindToString = {
    {minidb::TokenKind::Identifier, "Identifier"},
    {minidb::TokenKind::Number, "Number"},
    {minidb::TokenKind::String, "String"},
    {minidb::TokenKind::LeftParen, "LeftParen"},
    {minidb::TokenKind::RightParen, "RightParen"},
    {minidb::TokenKind::Comma, "Comma"},
    {minidb::TokenKind::Star, "Star"},
    {minidb::TokenKind::Semicolon, "Semicolon"},
    {minidb::TokenKind::Plus, "Plus"},
    {minidb::TokenKind::Minus, "Minus"}
};

string dataTypeToString(minidb::DataType type) {
    switch (type) {
        case minidb::DataType::Integer: return "INT";
        case minidb::DataType::Float:   return "FLOAT";
        case minidb::DataType::Text:    return "TEXT";
    }
    return "Unknown";
}

int main() {
    string sql;

    while (cout << "miniDB> " && getline(cin, sql)) {
        try {
            auto tokens = minidb::tokenize(sql);

            for (const auto& token : tokens) {
                cout << "Token: " << tokenKindToString[token.kind]
                     << ", Text: " << token.text
                     << ", Offset: " << token.offset << endl;
            }

            auto statement = minidb::parseStatement(tokens);
            cout << "Parse successful:\n";

            switch (statement.kind) {
                case minidb::Statement::Kind::CreateTable:
                    cout << "CREATE TABLE " << statement.createTable.tableName << '\n';
                    for (const auto& column : statement.createTable.columns) {
                        cout << "  " << column.name << " "
                             << dataTypeToString(column.type) << '\n';
                    }
                    break;

                case minidb::Statement::Kind::Insert:
                    cout << "INSERT INTO " << statement.insert.tableName << '\n';
                    cout << "  Values:";
                    for (const auto& value : statement.insert.values) {
                        cout << " " << value.text;
                    }
                    cout << '\n';
                    break;

                case minidb::Statement::Kind::Select:
                    cout << "SELECT ";
                    if (statement.select.columns.empty()) {
                        cout << "*";
                    } else {
                        for (size_t i = 0; i < statement.select.columns.size(); ++i) {
                            if (i > 0) cout << ", ";
                            cout << statement.select.columns[i];
                        }
                    }
                    cout << " FROM " << statement.select.tableName << '\n';
                    break;
            }
        } catch (const minidb::TokenizeError& e) {
            cerr << "Tokenize error: " << e.what() << '\n';
        } catch (const minidb::ParseError& e) {
            cerr << "Parse error: " << e.what() << '\n';
        }
    }

    return 0;
}