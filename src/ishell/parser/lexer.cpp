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

Lexer::Lexer(std::string input) {
    input_buffer.insert(input_buffer.end(), input.rbegin(), input.rend());
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
    else if(isPrefixedPrimaryStart()) return getPrefixedPrimary();
    else return getKeyword();
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

static std::string toupper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
    return str;
}

Token Lexer::getKeyword() {
    Token t;
    auto word = getWord();
    t.lexeme = toupper(word);
    if(t.lexeme == "STOP") t.token_type = TokenType::STOP;
    else if(t.lexeme == "PAUSE") t.token_type = TokenType::PAUSE;
    else if(t.lexeme == "RESUME") t.token_type = TokenType::RESUME;
    else if(t.lexeme == "WATCH") t.token_type = TokenType::WATCH;
    else if(t.lexeme == "DUMP") t.token_type = TokenType::DUMP;
    else if(t.lexeme == "DISASM") t.token_type = TokenType::DISASM;
    else if(t.lexeme == "IF") t.token_type = TokenType::IF;
    else if(t.lexeme == "ON") t.token_type = TokenType::ON;
    else if(event::isEventSubsystemType(t.lexeme))
        t.token_type = TokenType::SUBSYSTEM;
    else if(event::isEventType(t.lexeme)) t.token_type = TokenType::EVENT;

    return t;
}

Token Lexer::getPrefixedPrimary() {
    Token t;
    if(peekChar() == '$') {
        getChar();
        auto word = getWord();
        t.lexeme = toupper(word);
        if(t.lexeme == "PC") t.token_type = TokenType::PC;
        else if(t.lexeme == "M") t.token_type = TokenType::MEM;
        else t.token_type = TokenType::REGISTER;
    }
    return t;
}
bool Lexer::isPrefixedPrimaryStart() { return peekChar() == '$'; }

Token Lexer::getSymbol() {
    Token t;

    // get the char
    if(isSymbol()) {
        char c = getChar();
        switch(c) {
            case ':': t.token_type = TokenType::COLON; break;
            case ',': t.token_type = TokenType::COMMA; break;
            case '[': t.token_type = TokenType::LBRACK; break;
            case ']': t.token_type = TokenType::RBRACK; break;
            case '(': t.token_type = TokenType::LPAREN; break;
            case ')': t.token_type = TokenType::RPAREN; break;
            case '*': t.token_type = TokenType::MULTIPLY; break;
            case '/': t.token_type = TokenType::DIVIDE; break;
            case '+': t.token_type = TokenType::PLUS; break;
            case '-': t.token_type = TokenType::MINUS; break;
            case '<': {
                if(peekChar() == '=') {
                    getChar();
                    t.token_type = TokenType::LTEQ;
                } else if(peekChar() == '<') {
                    getChar();
                    t.token_type = TokenType::LSHIFT;
                } else t.token_type = TokenType::LT;
                break;
            }
            case '>': {
                if(peekChar() == '=') {
                    getChar();
                    t.token_type = TokenType::GTEQ;
                } else if(peekChar() == '>') {
                    getChar();
                    t.token_type = TokenType::RSHIFT;
                } else t.token_type = TokenType::GT;
                break;
            }
            case '=': {
                if(peekChar() == '=') {
                    getChar();
                    t.token_type = TokenType::EQ;
                }
                break;
            }
            case '!': {
                if(peekChar() == '=') {
                    getChar();
                    t.token_type = TokenType::NEQ;
                } else {
                    t.token_type = TokenType::NOT;
                }
                break;
            }
            case '&': {
                if(peekChar() == '&') {
                    getChar();
                    t.token_type = TokenType::AND;
                } else t.token_type = TokenType::BW_AND;
                break;
            }
            case '|': {
                if(peekChar() == '|') {
                    getChar();
                    t.token_type = TokenType::OR;
                } else t.token_type = TokenType::BW_OR;
                break;
            }
            case '~':
                t.token_type = TokenType::BW_NOT;
                break;
                // TODO: NEGATE, which is a prefixed MINUS, is not implemented

            default: break;
        }
    }
    return t;
}
bool Lexer::isSymbol() {
    char c = peekChar();
    return c == ':' || c == ',' || c == '[' || c == ']' || c == '(' ||
           c == ')' || c == '*' || c == '/' || c == '+' || c == '-' ||
           c == '<' || c == '>' || c == '=' || c == '!' || c == '&' ||
           c == '|' || c == '~';
}

std::string Lexer::getWord() {
    std::string s;
    while(!endOfInput() && (std::isalnum(peekChar()) || peekChar() == '_'))
        s += getChar();
    return s;
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
