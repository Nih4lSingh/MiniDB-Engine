#include <minidb/tokenizer.hpp>
#include <minidb/parser.hpp>
#include <minidb/database.hpp>

#include <iostream>
#include <string>
#include <utility>

using namespace std;

bool isQueryComplete(const string& query) {
    bool insideString = false;

    for (size_t i = 0; i < query.size(); i++) {
        if (query[i] == '\'') {
            if (insideString && i + 1 < query.size() && query[i + 1] == '\'') {
                i++;
            }
            else {
                insideString = !insideString;
            }
        }
        else if (query[i] == ';' && !insideString) {
            return true;
        }
    }

    return false;
}

int main() {
    string query;
    string sql;
    minidb::Database database;

    string FILE_PATH = "minidb.db";
    try {
        database.load(FILE_PATH);
    } catch (const minidb::ExecutionError& e) {
        cerr << "Database load error: " << e.what() << '\n';
        return 1;
    }

    while (true) {
        cout << ">> ";

        if (!getline(cin, sql)) {
            if (!query.empty()) {
                cerr << "  Incomplete SQL statement\n";
            }
            break;
        }
        query += sql + " ";

        if (isQueryComplete(query)) {
            string querycopy = move(query);
            query.clear();
            try {
            auto tokens = minidb::tokenize(querycopy);
            auto statement = minidb::parseStatement(tokens);

            switch (statement.kind) {
                case minidb::Statement::Kind::CreateTable:
                    database.createTable(statement.create);
                    database.save(FILE_PATH);
                    cout<<"  Table created successfully\n";
                    break;

                case minidb::Statement::Kind::Insert:
                    database.insertTable(statement.insert);
                    database.save(FILE_PATH);
                    cout<<"  Added a new row successfully\n";
                    break;

                case minidb::Statement::Kind::Select:
                    database.selectTable(statement.select);
                    break;
            }

            } catch (const minidb::TokenizeError& e) {
                cerr << "  Tokenize error: " << e.what() << '\n';
            } catch (const minidb::ParseError& e) {
                cerr << "  Parse error: " << e.what() << '\n';
            } catch (const minidb::ExecutionError & e){
                cerr << "  Execution error: " << e.what() << '\n';
            }
        }
    }

    return 0;
}