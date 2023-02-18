#ifndef ZIRCON_ISHELL_PARSER_LEXER_H_
#define ZIRCON_ISHELL_PARSER_LEXER_H_

#include <string>
#include <vector>

#define LEXER_TOKENS(F)                                                        \
    /*MISC*/                                                                   \
    F(END_OF_FILE)                                                             \
    F(NONE)                                                                    \
    F(ERROR)                                                                   \
    /*Event selection*/                                                        \
    F(SUBSYSTEM)                                                               \
    F(EVENT)                                                                   \
    F(COLON)                                                                   \
    /*keywords*/                                                               \
    F(STOP)                                                                    \
    F(PAUSE)                                                                   \
    F(RESUME)                                                                  \
    F(WATCH)                                                                   \
    F(DUMP)                                                                    \
    F(DISASM)                                                                  \
    F(IF)                                                                      \
    F(ON)                                                                      \
    /*primaries, most are prefixed with $*/                                    \
    F(NUM)                                                                     \
    F(PC)                                                                      \
    F(MEM)                                                                     \
    F(REGISTER)                                                                \
    F(COMMA)                                                                   \
    /*grouping*/                                                               \
    F(LPAREN)                                                                  \
    F(RPAREN)                                                                  \
    F(LBRACK)                                                                  \
    F(RBRACK)                                                                  \
    /*expr ops*/                                                               \
    F(MULTIPLY)                                                                \
    F(DIVIDE)                                                                  \
    F(PLUS)                                                                    \
    F(MINUS)                                                                   \
    F(LSHIFT)                                                                  \
    F(RSHIFT)                                                                  \
    F(LT)                                                                      \
    F(LTEQ)                                                                    \
    F(GT)                                                                      \
    F(GTEQ)                                                                    \
    F(EQ)                                                                      \
    F(NEQ)                                                                     \
    F(BW_AND)                                                                  \
    F(BW_OR)                                                                   \
    F(AND)                                                                     \
    F(OR)                                                                      \
    F(NEGATE)                                                                  \
    F(BW_NOT)                                                                  \
    F(NOT)

namespace ishell {

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
    operator ValueType() const { return value_; }
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
    bool operator==(const TokenType& other) const {
        return this->value_ == other.value_;
    }
    bool operator==(const ValueType& other) const {
        return this->value_ == other;
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
    Lexer(std::string input);
    Token getToken();
    Token peek(unsigned ahead = 1);
    TokenType ungetToken(Token);

  private:
    std::vector<Token> tokens;
    std::vector<char> input_buffer;

    // returns NUM
    Token getNUM();
    bool isNUM();
    bool isDigit();
    bool isHexDigit();
    bool isBinaryDigit();
    bool isHexStart();
    bool isBinaryStart();

    Token getKeyword();

    Token getPrefixedPrimary();
    bool isPrefixedPrimaryStart();

    Token getSymbol();
    bool isSymbol();

    // helpers
    std::string getWord();
    void skipWhitespace();

    // input buffer code
    bool endOfInput();
    char peekChar();
    void ungetChar(char);
    char getChar();
};

} // namespace parser
} // namespace ishell
#endif
