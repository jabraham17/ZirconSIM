#include "parser.h"

#include "common/debug.h"
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
Parser::Control_ptr Parser::parse(std::string input) {
    common::debug::log(common::debug::DebugType::PARSER, "parse()\n");
    lexer = Lexer(input);
    auto c = parse_control();
    expect(TokenType::END_OF_FILE);
    return c;
}
std::vector<Parser::Control_ptr> Parser::parseAll(std::string input) {
    common::debug::log(common::debug::DebugType::PARSER, "parseAll()\n");
    lexer = Lexer(input);
    std::vector<Parser::Control_ptr> c_list;
    while(lexer.peek().token_type != TokenType::END_OF_FILE) {
        auto c = parse_control();
        c_list.push_back(c);
    }
    expect(TokenType::END_OF_FILE);
    return c_list;
}

Parser::Control_ptr Parser::parse_control() {
    common::debug::log(common::debug::DebugType::PARSER, "parse_control()\n");
    if(lexer.peek().token_type == TokenType::WATCH) {
        return parse_watch_command();
    } else {
        return parse_action_command();
    }
}
Parser::Watch_ptr Parser::parse_watch_command() {
    common::debug::log(
        common::debug::DebugType::PARSER,
        "parse_watch_command()\n");
    expect(TokenType::WATCH);
    Watch_ptr watch;
    if(lexer.peek().token_type == TokenType::REGISTER) {
        auto reg_tok = expect(TokenType::REGISTER);
        if(auto reg = isa::rf::parseRegister(reg_tok.lexeme)) {
            watch = std::make_shared<command::WatchRegister>(*reg);
        } else throw new ParseException("Invalid Register: " + reg_tok.lexeme);
    } else {
        auto expr = parse_expr();
        watch = std::make_shared<command::WatchMemoryAddress>(expr);
    }
    auto actions = parse_action_list();
    actions = make_action_group(actions);
    watch->setActions(actions);

    return watch;
}
Parser::Command_ptr Parser::parse_action_command() {
    common::debug::log(
        common::debug::DebugType::PARSER,
        "parse_action_command()\n");
    auto actions = parse_action_list();
    auto condition = parse_if_statement();
    auto event_list = parse_on_statement();

    actions = make_action_group(actions);

    return std::make_shared<command::Command>(actions, condition, event_list);
}
command::ExprPtr Parser::parse_if_statement() {
    common::debug::log(
        common::debug::DebugType::PARSER,
        "parse_if_statement()\n");
    if(lexer.peek().token_type == TokenType::IF) {
        expect(TokenType::IF);
        auto cond_expr = parse_expr();
        return cond_expr;
    }
    return nullptr;
}
std::vector<event::EventType> Parser::parse_on_statement() {
    common::debug::log(
        common::debug::DebugType::PARSER,
        "parse_on_statement()\n");
    if(lexer.peek().token_type == TokenType::ON) {
        expect(TokenType::ON);
        return parse_event_list();
    }
    return {};
}
std::vector<Parser::Action_ptr> Parser::parse_action_list() {
    common::debug::log(
        common::debug::DebugType::PARSER,
        "parse_action_list()\n");
    auto action = parse_action();
    if(lexer.peek().token_type == TokenType::COMMA) {
        expect(TokenType::COMMA);
        auto action_list = parse_action_list();
        action_list.insert(action_list.begin(), action);
        return action_list;
    } else return {action};
}
std::vector<event::EventType> Parser::parse_event_list() {
    common::debug::log(
        common::debug::DebugType::PARSER,
        "parse_event_list()\n");
    auto event = parse_event();
    if(lexer.peek().token_type == TokenType::COMMA) {
        expect(TokenType::COMMA);
        auto event_list = parse_event_list();
        event_list.insert(event_list.begin(), event);
        return event_list;
    } else return {event};
}
event::EventType Parser::parse_event() {
    common::debug::log(common::debug::DebugType::PARSER, "parse_event()\n");
    auto sub_lexeme = expect(TokenType::SUBSYSTEM).lexeme;
    expect(TokenType::COLON);
    auto ev_lexeme = expect(TokenType::EVENT).lexeme;
    auto sub_ev = event::getEventType(sub_lexeme + "_" + ev_lexeme);
    if(sub_ev == event::EventType::NONE)
        throw ParseException("Unknown event: " + sub_lexeme + ":" + ev_lexeme);
    return sub_ev;
}
Parser::Action_ptr Parser::parse_action() {
    common::debug::log(common::debug::DebugType::PARSER, "parse_action()\n");
    if(lexer.peek().token_type == TokenType::STOP) {
        expect(TokenType::STOP);
        return std::make_shared<command::action::Stop>();
    } else if(lexer.peek().token_type == TokenType::PAUSE) {
        expect(TokenType::PAUSE);
        return std::make_shared<command::action::Pause>();
    } else if(lexer.peek().token_type == TokenType::RESUME) {
        expect(TokenType::RESUME);
        return std::make_shared<command::action::Resume>();
    } else if(lexer.peek().token_type == TokenType::DISASM) {
        expect(TokenType::DISASM);
        auto expr = parse_expr();
        return std::make_shared<command::action::Disasm>(expr);
    } else if(lexer.peek().token_type == TokenType::DUMP) {
        expect(TokenType::DUMP);
        auto expr = parse_expr();
        return std::make_shared<command::action::Dump>(expr);
    } else throw ParseException("Unknown action: " + lexer.peek().getString());
}

command::ExprPtr Parser::parse_expr() {
    common::debug::log(common::debug::DebugType::PARSER, "parse_expr()\n");
    std::vector<Token> input;
    while(ExprParser::isExprToken(lexer.peek())) {
        input.push_back(lexer.getToken());
    }
    ExprParser ep(input);
    auto expr = ep.parse();
    return expr;
}
std::vector<Parser::Action_ptr>
Parser::make_action_group(const std::vector<Action_ptr>& actions) {
    // only make an action group if there is more than 1 action
    if(actions.size() > 1) {
        auto action_group =
            std::make_shared<command::action::ActionGroup>(actions);
        return {action_group};
    } else return actions;
}

} // namespace parser
} // namespace ishell
