#include "lexer.h"

#include "event/event.h"
#include "hart/isa/rf.h"

#include <algorithm>
#include <iostream>
#include <sstream>
namespace ishell {
namespace parser {

std::string Token::getString() const {
    std::stringstream str;
    str << "{" << std::string(token_type) << ", \"" << lexeme << "\"}";
    return str.str();
}
std::string Token::getString() {
    return const_cast<const Token*>(this)->getString();
}

Lexer::Lexer(std::vector<std::string> input) {
    char c = 0;
    for(auto s : input) {
        if(c) input_buffer.push_back(c);
        c = ' ';
        input_buffer.insert(input_buffer.end(), s.begin(), s.end());
    }
    std::reverse(input_buffer.begin(), input_buffer.end());
}

Token Lexer::getToken() {
    // if we have tokens, get them,
    if(!tokens.empty()) {
        auto t = tokens.back();
        tokens.pop_back();
        return t;
    }
    skipWhitespace();

    if(endOfInput()) return Token(TokenType::END_OF_FILE);

    if(isSymbol()) return getSymbol();
    else if(isNUM()) return getNUM();
    else return getID();
}

Token Lexer::peek(unsigned ahead) {
    if(ahead == 0) return Token();
    std::vector<Token> peeked_buffer;
    for(unsigned i = 0; i < ahead; i++) {
        peeked_buffer.push_back(getToken());
    }
    if(peeked_buffer.empty()) return Token();

    Token peeked = peeked_buffer.back();
    for(auto it = peeked_buffer.rbegin(); it != peeked_buffer.rend(); it++) {
        ungetToken(*it);
    }

    return peeked;
}

TokenType Lexer::ungetToken(Token t) {
    tokens.push_back(t);
    return t.token_type;
}

Token Lexer::getID() {
    Token t;
    auto word = getWord();
    auto toupper = [](std::string str) {
        std::transform(
            str.begin(),
            str.end(),
            str.begin(),
            [](unsigned char c) { return std::toupper(c); });
        return str;
    };
    t.lexeme = toupper(word);
    if(t.lexeme == "PC") t.token_type = TokenType::PC;
    else if(t.lexeme == "M") t.token_type = TokenType::MEM;
    else if(t.lexeme == "STOP") t.token_type = TokenType::STOP;
    else if(t.lexeme == "PAUSE") t.token_type = TokenType::PAUSE;
    else if(t.lexeme == "WATCH") t.token_type = TokenType::WATCH;
    else if(event::isEventSubsystemType(t.lexeme))
        t.token_type = TokenType::SUBSYSTEM;
    else if(event::isEventType(t.lexeme)) t.token_type = TokenType::EVENT;
    else if(isa::rf::isRegisterClassType(t.lexeme))
        t.token_type = TokenType::REGISTER_CLASS;

    return t;
}

std::string Lexer::getWord() {
    std::string s;
    while(!endOfInput() && (std::isalnum(peekChar()) || peekChar() == '_'))
        s += getChar();
    return s;
}

// returns NUM
Token Lexer::getNUM() {
    Token t;

    if(isHexStart()) {
        // consume hex start
        getChar();
        getChar();
        while(!endOfInput() && isHexDigit())
            t.lexeme += getChar();
        t.token_type = TokenType::NUM;
        t.lexeme = std::to_string(std::stoull(t.lexeme, nullptr, 16));
    } else if(isBinaryStart()) {
        // consume bin start
        getChar();
        getChar();
        while(!endOfInput() && isBinaryDigit())
            t.lexeme += getChar();
        t.token_type = TokenType::NUM;
        t.lexeme = std::to_string(std::stoull(t.lexeme, nullptr, 2));
    } else if(isDigit()) {
        while(!endOfInput() && isDigit())
            t.lexeme += getChar();
        t.token_type = TokenType::NUM;
    }
    // if no valid num sequence, then error is already set
    return t;
}
bool Lexer::isNUM() { return isDigit() || isHexStart() || isBinaryStart(); }
bool Lexer::isDigit() {
    char c = peekChar();
    return c == '0' || c == '1' || c == '2' || c == '3' || c == '4' ||
           c == '5' || c == '6' || c == '7' || c == '8' || c == '9';
}
bool Lexer::isHexDigit() {
    char c = peekChar();
    return isDigit() || c == 'A' || c == 'B' || c == 'C' || c == 'D' ||
           c == 'E' || c == 'F' || c == 'a' || c == 'b' || c == 'c' ||
           c == 'd' || c == 'e' || c == 'f';
}
bool Lexer::isBinaryDigit() {
    char c = peekChar();
    return c == '0' || c == '1';
}
bool Lexer::isHexStart() {
    char c1 = getChar();
    char c2 = peekChar();
    ungetChar(c1);
    return c1 == '0' && c2 == 'x';
}
bool Lexer::isBinaryStart() {
    char c1 = getChar();
    char c2 = peekChar();
    ungetChar(c1);
    return c1 == '0' && c2 == 'b';
}

Token Lexer::getSymbol() {
    Token t;

    // get the char
    if(isSymbol()) {
        char c = getChar();
        switch(c) {
            case ':': t.token_type = TokenType::COLON; break;
            case ',': t.token_type = TokenType::COMMA; break;
            case '=': t.token_type = TokenType::EQUALS; break;
            case '!': {
                c = getChar();
                if(c == '=') t.token_type = TokenType::NOTEQUAL;
                break;
            }
            case '<': {
                if(peekChar() == '=') {
                    getChar();
                    t.token_type = TokenType::LESSTHAN_EQUALTO;
                } else t.token_type = TokenType::LESSTHAN;
                break;
            }
            case '>': {
                if(peekChar() == '=') {
                    getChar();
                    t.token_type = TokenType::GREATERTHAN_EQUALTO;
                } else t.token_type = TokenType::GREATERTHAN;
                break;
            }
            case '[': t.token_type = TokenType::LBRACK; break;
            case ']': t.token_type = TokenType::RBRACK; break;
            case '+': t.token_type = TokenType::PLUS; break;
            case '-': t.token_type = TokenType::MINUS; break;
            default: break;
        }
    }
    return t;
}

// just need to check first one
bool Lexer::isSymbol() {
    char c = peekChar();
    return c == ':' || c == ',' || c == '=' || c == '<' || c == '>' ||
           c == '!' || c == '[' || c == ']' || c == '+' || c == '-';
}

void Lexer::skipWhitespace() {
    while(!endOfInput() && isspace(peekChar())) {
        getChar();
    }
}

bool Lexer::endOfInput() { return input_buffer.empty(); }
char Lexer::peekChar() {
    char c = getChar();
    ungetChar(c);
    return c;
}
void Lexer::ungetChar(char c) {
    if(c != 0) input_buffer.push_back(c);
}
char Lexer::getChar() {
    if(!input_buffer.empty()) {
        char c = input_buffer.back();
        input_buffer.pop_back();
        return c;
    } else {
        return 0;
    }
}

} // namespace parser
} // namespace ishell
