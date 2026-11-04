#include "tokenizer.hpp"
#include <cctype>

using namespace std;

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
            char c = sql[i];

            if(isspace(static_cast<unsigned char>(c))){
                i++;
                continue;
            }

            if(isdigit(static_cast<unsigned char>(c))){
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

            if(isalpha(static_cast<unsigned char>(c)) || c == '_'){
                size_t start = i;

                while(i < sql.size() &&
                    (isalpha(static_cast<unsigned char>(sql[i])) ||
                    sql[i] == '_' ||
                    isdigit(static_cast<unsigned char>(sql[i])))){

                    currentToken += sql[i];
                    i++;
                }

                if(!currentToken.empty()){
                    tokens.push_back({TokenKind::Identifier, currentToken, start});
                    currentToken.clear();
                }

                continue;
            }

            if(c == '\''){
                size_t start = i;
                currentToken += c;
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

            if(c == ','){
                tokens.push_back({TokenKind::Comma, string(1, c), i});
                i++;
                continue;
            }

            if(c == '*'){
                tokens.push_back({TokenKind::Star, string(1, c), i});
                i++;
                continue;
            }

            if(c == ';'){
                tokens.push_back({TokenKind::Semicolon, string(1, c), i});
                i++;
                continue;
            }

            if(c == '('){
                tokens.push_back({TokenKind::LeftParen, string(1, c), i});
                i++;
                continue;
            }

            if(c == ')'){
                tokens.push_back({TokenKind::RightParen, string(1, c), i});
                i++;
                continue;
            }

            if(c == '+'){
                tokens.push_back({TokenKind::Plus, string(1, c), i});
                i++;
                continue;
            }

            if(c == '-'){
                tokens.push_back({TokenKind::Minus, string(1, c), i});
                i++;
                continue;
            }

            throw TokenizeError(i, "Invalid character");
        }

        return tokens;
    }

    }
