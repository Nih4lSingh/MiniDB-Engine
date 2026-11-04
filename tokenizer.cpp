#include "tokenizer.hpp"
#include <cctype> // for character classification functions

using namespace std;
using namespace minidb;
vector<Token> tokenize(const string& sql){
    vector<Token> tokens; //vector of tokens
    size_t i=0;
    string currentToken; //string to hold the current token being processed
    for (i=0;i<sql.size();){
        char c = sql[i];
        if(isspace(static_cast<unsigned char>(c))){ // Skip whitespace
            i++;
            continue;
        }
        if(isdigit(c)){
            while(i<sql.size() && isdigit(static_cast<unsigned char>(sql[i]))){
                currentToken += sql[i];
                i++;
            }
            if(i< sql.size() && sql[i]=='.'){ // Handle decimal numbers
                currentToken += sql[i];
                i++;
                while(i<sql.size() && isdigit(static_cast<unsigned char>(sql[i]))){
                    currentToken += sql[i];
                    i++;
                }
                if(!isdigit(static_cast<unsigned char>(sql[i-1]))) throw TokenizeError(i, "Invalid character in number");
                if(i<sql.size() && sql[i]=='.') throw TokenizeError(i, "Invalid character in number");
            }
            tokens.push_back({TokenKind::Number, currentToken, i - currentToken.size()});
            currentToken.clear();
            continue;
        }
        if(isalpha(c) || c=='_'){
            while(i<sql.size() && (isalpha(static_cast<unsigned char>(sql[i])) || sql[i]=='_' || isdigit(static_cast<unsigned char>(sql[i])))){
                currentToken += sql[i];
                i++;
            }
            if(!currentToken.empty()){
                tokens.push_back({TokenKind::Identifier, currentToken, i - currentToken.size()});
                currentToken.clear();
            }
            continue;
        }
        if(c=='"'){
            currentToken += c;
            i++;
            while(i<sql.size() && sql[i]!='"'){
                currentToken += sql[i];
                i++;
            }
            if(i<sql.size() && sql[i]=='"'){
                currentToken += sql[i];
                i++;
            }
            else{
                throw TokenizeError(i, "Unterminated string literal");
            }
            tokens.push_back({TokenKind::String, currentToken, i - currentToken.size()});
            currentToken.clear();
            continue;
        }
        if(c=='\''){
            currentToken += c;
            i++;
            while(i<sql.size() && sql[i]!='\''){
                currentToken += sql[i];
                i++;
            }
            if(i<sql.size() && sql[i]=='\''){
                currentToken += sql[i];
                i++;
            }
            else{
                throw TokenizeError(i, "Unterminated string literal");
            }
            tokens.push_back({TokenKind::String, currentToken, i - currentToken.size()});
            currentToken.clear();
            continue;
        }
        if(c==','){
            tokens.push_back({TokenKind::Comma, string(1, c), i});
            i++;
            continue;
        
        }
        if(c=='*'){
            tokens.push_back({TokenKind::Star, string(1, c), i});
            i++;
            continue;
        }
        if(c==';'){
            tokens.push_back({TokenKind::Semicolon, string(1, c), i});
            i++;
            continue;
        }
        if(c=='('){
            tokens.push_back({TokenKind::LeftParen, string(1, c), i});
            i++;
            continue;
        }
        if(c==')'){
            tokens.push_back({TokenKind::RightParen, string(1, c), i});
            i++;
            continue;
        }
        if(c=='+'){
            tokens.push_back({TokenKind::Plus, string(1, c), i});
            i++;
            continue;
        }
        if(c=='-'){
            tokens.push_back({TokenKind::Minus, string(1, c), i});
            i++;
            continue;
        }
        throw TokenizeError(i, "Invalid character");
    }
    return tokens;
}