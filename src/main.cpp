#include <minidb/tokenizer.hpp>
#include <minidb/parser.hpp>
#include <minidb/database.hpp>

#include <iostream>
#include <string>

using namespace std;

int main() {
    string sql;
    minidb::Database database;
    while (cout << "miniDB> " && getline(cin, sql)) {
        try {
            auto tokens = minidb::tokenize(sql);
            auto statement = minidb::parseStatement(tokens);
            cout << "Parse successful:\n";

            switch (statement.kind) {
                case minidb::Statement::Kind::CreateTable:
                    database.createTable(statement.createTable);
                    cout<<"Table created successfully\n";
                    break;

                case minidb::Statement::Kind::Insert:
                    database.insertTable(statement.insert);
                    cout<<"Added a new row successfully\n";
                    break;

                case minidb::Statement::Kind::Select:
                    database.selectTable(statement.select);
                    break;
            }

        } catch (const minidb::TokenizeError& e) {
            cerr << "Tokenize error: " << e.what() << '\n';
        } catch (const minidb::ParseError& e) {
            cerr << "Parse error: " << e.what() << '\n';
        } catch (const minidb::ExecutionError & e){
            cerr << "Execution error: " << e.what() << '\n';
        }
    }

    return 0;
}