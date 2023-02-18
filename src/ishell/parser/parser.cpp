#include "parser.h"

#include "event/event.h"
#include "common/debug.h"

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
Parser::Control_ptr Parser::parse() {
    common::debug::log(common::debug::DebugType::PARSER, __PRETTY_FUNCTION__, "\n");
    auto c = parse_control();
    expect(TokenType::END_OF_FILE);
    return c;
}

Parser::Control_ptr Parser::parse_control() {
    if(lexer.peek().token_type == TokenType::WATCH) {
        return parse_watch_command();
    } else {
        return parse_action_command();
    }
}
Parser::Watch_ptr Parser::parse_watch_command() {
    expect(TokenType::WATCH);
    Watch_ptr watch;
    if(lexer.peek().token_type == TokenType::REGISTER) {
        auto reg_tok = expect(TokenType::REGISTER);
        if(auto reg = isa::rf::parseRegister(reg_tok.lexeme)) {
            auto [regtype, idx] = *reg;
            watch = std::make_shared<command::WatchRegister>(regtype, idx);
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
    auto actions = parse_action_list();
    auto condition = parse_if_statement();
    auto event_list = parse_on_statement();

    actions = make_action_group(actions);
    // if no condition (ie a nullptr), make condition_list empty
    auto condition_list =
        condition ? (std::initializer_list<Condition_ptr>){condition}
                  : (std::initializer_list<Condition_ptr>());

    return std::make_shared<command::Command>(
        actions,
        condition_list,
        event_list);
}
Parser::Condition_ptr Parser::parse_if_statement() {
    if(lexer.peek().token_type == TokenType::IF) {
        expect(TokenType::IF);
        auto cond_expr = parse_expr();
        return std::make_shared<command::Condition>(cond_expr);
    }
    return nullptr;
}
std::vector<event::EventType> Parser::parse_on_statement() {
    if(lexer.peek().token_type == TokenType::ON) {
        expect(TokenType::ON);
        return parse_event_list();
    }
    return {};
}
std::vector<Parser::Action_ptr> Parser::parse_action_list() {
    // std::cerr << "parse action\n";
    auto action = parse_action();
    if(lexer.peek().token_type == TokenType::COMMA) {
        expect(TokenType::COMMA);
        auto action_list = parse_action_list();
        action_list.insert(action_list.begin(), action);
        return action_list;
    } else return {action};
}
std::vector<event::EventType> Parser::parse_event_list() {
    auto event = parse_event();
    if(lexer.peek().token_type == TokenType::COMMA) {
        expect(TokenType::COMMA);
        auto event_list = parse_event_list();
        event_list.insert(event_list.begin(), event);
        return event_list;
    } else return {event};
}
event::EventType Parser::parse_event() {
    auto sub_lexeme = expect(TokenType::SUBSYSTEM).lexeme;
    expect(TokenType::COLON);
    auto ev_lexeme = expect(TokenType::EVENT).lexeme;
    auto sub_ev = event::getEventType(sub_lexeme + "_" + ev_lexeme);
    if(sub_ev == event::EventType::NONE)
        throw ParseException("Unknown event: " + sub_lexeme + ":" + ev_lexeme);
    return sub_ev;
}
Parser::Action_ptr Parser::parse_action() {
    // std::cerr << lexer.peek().getString() << "\n";
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

Parser::Expr_ptr Parser::parse_expr() {
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
