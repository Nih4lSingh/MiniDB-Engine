#include <minidb/parser.hpp>

#include <cctype>
#include <vector>
#include <string>

using namespace std;

namespace minidb {
    ParseError::ParseError(size_t tokenIndex, const string& message)
    : runtime_error("at token " + to_string(tokenIndex) + ": " + message),
      tokenIndex_(tokenIndex) {}

    size_t ParseError::tokenIndex() const noexcept {
        return tokenIndex_;
    }
    Statement parseStatement(const vector<Token>& tokens) {
        
    }
}