#include "parser.h"

#include "event/event.h"

#include <utility>

#include "expr_parser.h"

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
    // auto controls = parse_command();
    parse_command();
    // TODO: implement
    expect(TokenType::END_OF_FILE);
    return command::ControlList();
}

void Parser::parse_command() {
    if(lexer.peek().token_type == TokenType::WATCH) {
        parse_watch_command();
    } else {
        parse_action_command();
    }
}
void Parser::parse_watch_command() {
    expect(TokenType::WATCH);
    if(lexer.peek().token_type == TokenType::REGISTER) {
        expect(TokenType::REGISTER);
    } else {
        expect(TokenType::MEM);
    }
    parse_action_list();
}
void Parser::parse_action_command() {
    parse_action_list();
    parse_if_statement();
    parse_on_statement();
}
void Parser::parse_if_statement() {
    if(lexer.peek().token_type == TokenType::IF) {
        expect(TokenType::IF);
        parse_expr();
    }
}
void Parser::parse_on_statement() {
    if(lexer.peek().token_type == TokenType::ON) {
        expect(TokenType::ON);
        parse_event_list();
    }
}
void Parser::parse_action_list() {
    parse_action();
    if(lexer.peek().token_type == TokenType::COMMA) {
        expect(TokenType::COMMA);
        parse_action_list();
    }
}
void Parser::parse_event_list() {
    parse_event();
    if(lexer.peek().token_type == TokenType::COMMA) {
        expect(TokenType::COMMA);
        parse_event_list();
    }
}
void Parser::parse_event() {
    expect(TokenType::SUBSYSTEM);
    expect(TokenType::COLON);
    expect(TokenType::EVENT);
}
void Parser::parse_action() {
    if(lexer.peek().token_type == TokenType::STOP) {
        expect(TokenType::STOP);
    } else if(lexer.peek().token_type == TokenType::PAUSE) {
        expect(TokenType::PAUSE);
    } else if(lexer.peek().token_type == TokenType::RESUME) {
        expect(TokenType::RESUME);
    } else if(lexer.peek().token_type == TokenType::DISASM) {
        expect(TokenType::DISASM);
        parse_expr();
    } else if(lexer.peek().token_type == TokenType::DUMP) {
        expect(TokenType::DUMP);
        parse_expr();
    }
}

void Parser::parse_expr() {
    std::vector<Token> input;
    // TODO: should not slurp all input, should slurp until an end of expr token
    // this could be passed in? or maybe deduced by context
    while(lexer.peek().token_type != TokenType::END_OF_FILE &&
          lexer.peek().token_type != TokenType::ERROR) {
        input.push_back(lexer.getToken());
    }
    ExprParser ep(input);
    ep.parse();
    std::cout << "success\n";
}

} // namespace parser
} // namespace ishell
