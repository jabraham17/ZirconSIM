#ifndef ZIRCON_ISHELL_PARSER_LEXER_H_
#define ZIRCON_ISHELL_PARSER_LEXER_H_

#include <array>
#include <string>
#include <vector>

// miscellaneous tokens
#define LEXER_TOKENS_MISC(F)                                                   \
    F(END_OF_FILE)                                                             \
    F(NONE)                                                                    \
    F(ERROR)

// event tokens
#define LEXER_TOKENS_EVENT(F)                                                  \
    F(SUBSYSTEM)                                                               \
    F(EVENT)                                                                   \
    F(COLON)

// keyword tokens
#define LEXER_TOKENS_KEYWORD(F)                                                \
    F(STOP)                                                                    \
    F(PAUSE)                                                                   \
    F(RESUME)                                                                  \
    F(WATCH)                                                                   \
    F(DUMP)                                                                    \
    F(DISASM)                                                                  \
    F(SET)                                                                     \
    F(IF)                                                                      \
    F(ON)

// keyword tokens
#define LEXER_TOKENS_PRIMARY(F)                                                \
    F(NUM)                                                                     \
    F(PC)                                                                      \
    F(MEM)                                                                     \
    F(REGISTER)                                                                \
    F(SYMBOL)

// grouping tokens
#define LEXER_TOKENS_GROUPING(F)                                               \
    F(LPAREN)                                                                  \
    F(RPAREN)                                                                  \
    F(LBRACK)                                                                  \
    F(RBRACK)

// operator tokens
#define LEXER_TOKENS_OPERATOR(F)                                               \
    F(EQUALS)                                                                  \
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

#define LEXER_TOKENS(F)                                                        \
    LEXER_TOKENS_MISC(F)                                                       \
    LEXER_TOKENS_EVENT(F)                                                      \
    LEXER_TOKENS_KEYWORD(F)                                                    \
    LEXER_TOKENS_PRIMARY(F)                                                    \
    LEXER_TOKENS_GROUPING(F)                                                   \
    LEXER_TOKENS_OPERATOR(F)                                                   \
    /*others*/                                                                 \
    F(COMMA)                                                                   \
    F(SEMICOLON)                                                               \
    F(STRING)                                                                  \
    F(MODIFIER)

#define COUNT_LIST(elm) +1
#define MAKE_LIST(elm) elm,
#define MAKE_STR_LIST(elm) std::string(#elm),

namespace ishell {

namespace parser {

struct TokenType {
    using ValueType = uint64_t;

  private:
    ValueType value_;

  public:
#define TOKEN(tt) static const ValueType tt = __COUNTER__;
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
    // DO NOT PUT IN THESE EQUALITY FUNCTIONS
    // Having just the first one causes ambiguous compiler errors
    // Having just the second one or both causes all static const TokenType
    // to be removed without warning. But only when compiled with -O0. Compiled
    // with any level of optimization causes this to work properly and as
    // expected bool operator==(const TokenType& other) const {
    //     return this->value_ == other.value_;
    // }
    // bool operator==(const ValueType& other) const {
    //     return this->value_ == other;
    // }

    bool isKeyword() {
#define LEXER_CASE(tt) this->value_ == tt ||
        return LEXER_TOKENS_KEYWORD(LEXER_CASE) false;
#undef LEXER_CASE
    }
    static auto validKeywords() {
        std::array elms = {LEXER_TOKENS_KEYWORD(MAKE_STR_LIST)};
        return elms;
    }
    bool isOperator() {
#define LEXER_CASE(tt) this->value_ == tt ||
        return LEXER_TOKENS_OPERATOR(LEXER_CASE) false;
#undef LEXER_CASE
    }
    bool isPrimary() {
#define LEXER_CASE(tt) this->value_ == tt ||
        return LEXER_TOKENS_PRIMARY(LEXER_CASE) false;
#undef LEXER_CASE
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
    Lexer(std::string input = "");
    Token getToken();
    Token peek(unsigned ahead = 1);
    TokenType ungetToken(Token);
    void addInput(std::string);

  private:
    std::vector<Token> tokens;
    std::vector<char> input_buffer;
    Token last_token;

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

    Token getModifier();
    bool isModifierStart();

    Token getStringToken();
    bool isStringTokenStart();

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
