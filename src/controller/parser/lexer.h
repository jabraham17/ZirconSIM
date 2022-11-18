#ifndef ZIRCON_CONTROLLER_PARSER_LEXER_H_
#define ZIRCON_CONTROLLER_PARSER_LEXER_H_

#include <string>
#include <vector>

#define LEXER_TOKENS(F)                                                        \
    F(END_OF_FILE)                                                             \
    F(NONE)                                                                    \
    F(ERROR)                                                                   \
    F(SUBSYSTEM)                                                               \
    F(EVENT)                                                                   \
    F(REGISTER_CLASS)                                                          \
    F(MEM)                                                                     \
    F(STOP)                                                                    \
    F(PC)                                                                      \
    F(COLON)                                                                   \
    F(COMMA)                                                                   \
    F(EQUALS)                                                                  \
    F(LESSTHAN)                                                                \
    F(LESSTHAN_EQUALTO)                                                                \
    F(GREATERTHAN)                                                             \
    F(GREATERTHAN_EQUALTO)                                                             \
    F(NOTEQUAL)                                                                \
    F(LBRACK)                                                                  \
    F(RBRACK)                                                                  \
    F(NUM)

namespace controller {

namespace parser {

struct TokenType {
    using ValueType = uint64_t;

  private:
    ValueType value_;

  public:
#define TOKEN(t) static const ValueType t = __COUNTER__;
    LEXER_TOKENS(TOKEN)
#undef TOKEN

    TokenType(ValueType v = ERROR) : value_(v) {}
    operator ValueType() { return value_; }
    explicit operator std::string() const {
#define LEXER_CASE(tt)                                                         \
    if(this->value_ == tt) return #tt;
        LEXER_TOKENS(LEXER_CASE)
#undef LEXER_CASE
        return "ERROR";
    }
    explicit operator std::string() {
        return std::string(*const_cast<const TokenType*>(this));
    }
    TokenType operator=(ValueType v) {
        this->value_ = v;
        return *this;
    }
};

class Token {
  public:
    std::string lexeme;
    TokenType token_type;
    Token(std::string lexeme, TokenType token_type)
        : lexeme(lexeme), token_type(token_type) {}
    Token() : lexeme(""), token_type(TokenType::ERROR) {}
    Token(TokenType token_type) : lexeme(""), token_type(token_type) {}

    std::string getString() const;
    std::string getString();
};

class Lexer {
  public:
    Lexer(std::vector<std::string> input);
    Token getToken();
    Token peek(unsigned ahead = 1);
    TokenType ungetToken(Token);

  private:
    std::vector<Token> tokens;
    std::vector<char> input_buffer;

    Token getID();

    // get word
    std::string getWord();

    // returns NUM
    Token getNUM();
    bool isNUM();
    bool isDigit();
    bool isHexDigit();
    bool isBinaryDigit();
    bool isHexStart();
    bool isBinaryStart();

    Token getSymbol();
    bool isSymbol();

    void skipWhitespace();

    // input buffer code
    bool endOfInput();
    char peekChar();
    void ungetChar(char);
    char getChar();
};

} // namespace parser
} // namespace controller
#endif
