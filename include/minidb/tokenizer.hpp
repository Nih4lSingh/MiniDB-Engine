#pragma once 

#include <cstddef> // for the size_t type
#include <stdexcept> // for exception handling
#include <string>
#include <vector>


namespace minidb{ // creating a namespace for the project
    enum class TokenKind { 
        Identifier,
        Number,
        String,
        LeftParen,
        RightParen,
        Comma,
        Star,
        Semicolon,
        Plus,
        Minus,
        Null
    }; // Types of tokens that can be recognized

    struct Token{ // A structure to represent a token
        TokenKind kind; // The type of token
        std::string text; // The actual text of the token
        std::size_t offset; // The position of the token in the given string
    };

    class TokenizeError: public std::runtime_error{ // Excetion that occurs when we encounter an error while tokentizing
    public:
        TokenizeError(std::size_t offset, const std::string& message);
        std::size_t offset() const noexcept; // Returns the position of the error in the string
    private:
        std::size_t offset_; // The position of the error in the string
    };

    std::vector<Token> tokenize(const std::string& sql); //Turns a SQL query to vector of tokens, Defined in tokenizer.cpp
}