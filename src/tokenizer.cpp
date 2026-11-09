#include <minidb/tokenizer.hpp>
#include <cctype>
#include <algorithm>

using namespace std;

auto toLower = [](unsigned char ch) {
        return static_cast<char>(tolower(ch));
    };

namespace minidb {
    TokenizeError::TokenizeError(size_t offset, const string& message)
    : runtime_error("at byte " + to_string(offset) + ": " + message),
      offset_(offset) {}

    size_t TokenizeError::offset() const noexcept {
        return offset_;
    }

    vector<Token> tokenize(const string& sql){
        vector<Token> tokens;
        size_t i = 0;
        string currentToken;

        for (i = 0; i < sql.size();){
            char character = sql[i];

            if(isspace(static_cast<unsigned char>(character))){
                i++;
                continue;
            }

            if(isdigit(static_cast<unsigned char>(character))){
                size_t start = i;

                while(i < sql.size() && isdigit(static_cast<unsigned char>(sql[i]))){
                    currentToken += sql[i];
                    i++;
                }

                if(i < sql.size() && sql[i] == '.'){
                    currentToken += sql[i];
                    i++;

                    while(i < sql.size() && isdigit(static_cast<unsigned char>(sql[i]))){
                        currentToken += sql[i];
                        i++;
                    }

                    if(!isdigit(static_cast<unsigned char>(sql[i-1])))
                        throw TokenizeError(i, "Invalid character in number");

                    if(i < sql.size() && sql[i] == '.')
                        throw TokenizeError(i, "Invalid character in number");
                }

                tokens.push_back({TokenKind::Number, currentToken, start});
                currentToken.clear();
                continue;
            }

            if(isalpha(static_cast<unsigned char>(character)) || character == '_'){
                size_t start = i;

                while(i < sql.size() &&
                    (isalpha(static_cast<unsigned char>(sql[i])) ||
                    sql[i] == '_' ||
                    isdigit(static_cast<unsigned char>(sql[i])))){

                    currentToken += sql[i];
                    i++;
                }
                string currentTokencopy = currentToken;
                transform(currentTokencopy.begin(), currentTokencopy.end(), currentTokencopy.begin(), toLower);
                if(currentTokencopy == "null"){
                    tokens.push_back({TokenKind::Null, currentTokencopy, start});
                    currentToken.clear();
                }
                if(!currentToken.empty()){
                    tokens.push_back({TokenKind::Identifier, currentToken, start});
                    currentToken.clear();
                }

                continue;
            }

            if(character == '\''){
                size_t start = i;
                currentToken += character;
                i++;

                bool closed = false;

                while(i < sql.size()){
                    if(sql[i] == '\''){
                        if(i + 1 < sql.size() && sql[i+1] == '\''){
                            currentToken += '\'';
                            currentToken += '\'';
                            i += 2;
                            continue;
                        }

                        currentToken += sql[i];
                        i++;
                        closed = true;
                        break;
                    }

                    currentToken += sql[i];
                    i++;
                }

                if(!closed)
                    throw TokenizeError(i, "Unterminated string literal");

                tokens.push_back({TokenKind::String, currentToken, start});
                currentToken.clear();
                continue;
            }

            TokenKind punctuation;
            switch (character) {
                case ',': punctuation = TokenKind::Comma; break;
                case '*': punctuation = TokenKind::Star; break;
                case ';': punctuation = TokenKind::Semicolon; break;
                case '(': punctuation = TokenKind::LeftParen; break;
                case ')': punctuation = TokenKind::RightParen; break;
                case '+': punctuation = TokenKind::Plus; break;
                case '-': punctuation = TokenKind::Minus; break;
                default: throw TokenizeError(i, "Invalid character");
            }
            tokens.push_back({punctuation, string(1, character), i});
            i++;
            continue;
        }

        return tokens;
    }

    }
