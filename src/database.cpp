#include <minidb/database.hpp>

namespace minidb {

    void Database::createTable(const CreateTableStatement& statement) {
        if (tables_.find(statement.tableName) != tables_.end()) {
            throw ExecutionError("Table already exists: " + statement.tableName);
        }

        tables_.emplace(statement.tableName, Table{statement.columns});
    }

}