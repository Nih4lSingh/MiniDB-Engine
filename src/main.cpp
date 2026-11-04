#include <minidb/tokenizer.hpp>

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
int main(){
    string sql;
    while(cout<<"miniDB> " && getline(cin, sql)){
        try{
            auto tokens = minidb::tokenize(sql);
            for(const auto& token: tokens){
                cout<< "Token: "<< tokenKindToString[token.kind]<< ", Text: "<< token.text<< ", Offset: "<< token.offset<< endl;
            }
        }catch(const minidb::TokenizeError& e){
            cerr<< "Tokenize error: "<< e.what()<< endl;
        }
    }
    return 0;
}