#include "parser.h"

#include "event/event.h"
#include "expr_parser.h"
#include <utility>

namespace ishell {
namespace parser {

Token Parser::expect(TokenType tt) {
    auto t = lexer.getToken();
    if(t.token_type == tt) {
        return t;
    }
    throw ParseException(
        "Expected '" + std::string(tt) + "' got '" + std::string(t.token_type) +
        "'");
}
command::ControlList Parser::parse() {
    parse_expr();
    // auto controls = parse_controls();
    expect(TokenType::END_OF_FILE);
    // return command::ControlList(controls);
    return command::ControlList();
}

void Parser::parse_expr() {
    std::vector<Token> input;
    while(lexer.peek().token_type != TokenType::END_OF_FILE && lexer.peek().token_type != TokenType::ERROR) {
        input.push_back(lexer.getToken());
    }
    ExprParser ep(input);
    ep.parse();
    std::cout << "success\n";
}

} // namespace parser
} // namespace ishell
