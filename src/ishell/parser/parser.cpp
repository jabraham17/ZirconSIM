#include "parser.h"

#include "common/debug.h"
#include "event/event.h"

#include <deque>
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
command::CommandPtr
Parser::parse(std::string input, command::CommandContext context) {
    common::debug::log(common::debug::DebugType::PARSER, "parse()\n");
    lexer = Lexer(input);
    auto c = parse_command(context);
    expect(TokenType::END_OF_FILE);
    return c;
}
std::vector<command::CommandPtr>
Parser::parseAll(std::string input, command::CommandContext context) {
    common::debug::log(common::debug::DebugType::PARSER, "parseAll()\n");
    lexer = Lexer(input);
    std::vector<command::CommandPtr> c_list;
    while(lexer.peek().token_type != TokenType::END_OF_FILE) {
        auto c = parse_command(context);
        c_list.push_back(c);
    }
    expect(TokenType::END_OF_FILE);
    return c_list;
}

template <class ForwardIT>
static bool actionsContainWatch(ForwardIT begin, ForwardIT end) {
    using VT = typename std::iterator_traits<ForwardIT>::value_type;
    static_assert(
        std::is_same<VT, action::ActionPtr>(),
        "Iterator must be to an action");
    std::deque<VT> q(begin, end);
    while(!q.empty()) {
        auto v = q.back();
        q.pop_back();
        if(v->template isa<action::Watch>()) return true;
        if(v->template isa<action::ActionGroup>()) {
            auto more_actions =
                v->template cast<action::ActionGroup>()->getActions();
            q.insert(q.begin(), more_actions.begin(), more_actions.end());
        }
    }
    return false;
}

command::CommandPtr Parser::parse_command(command::CommandContext context) {
    common::debug::log(common::debug::DebugType::PARSER, "parse_command()\n");
    auto actions = parse_action_list(TokenType::SEMICOLON);
    auto condition = parse_if_statement();
    auto event_list = parse_on_statement();

    actions = make_action_group(actions);

    // no events and CLI context, use a callback
    // no events and REPL context, use an immediate, UNLESS there is a watch
    // command otherwise regardless of context if there are events use callback
    if(event_list.empty()) {
        if(context == command::CommandContext::CLI) {
            common::debug::logln(
                common::debug::DebugType::PARSER,
                "Returning a callback from CLI with no events");
            return std::make_shared<command::CallbackCommand>(
                context,
                actions,
                condition);
        } else if(
            context == command::CommandContext::REPL &&
            actionsContainWatch(actions.begin(), actions.end())) {
            common::debug::logln(
                common::debug::DebugType::PARSER,
                "Returning a callback from REPL with WATCH command");
            return std::make_shared<command::CallbackCommand>(
                context,
                actions,
                condition);
        } else if(context == command::CommandContext::REPL) {
            common::debug::logln(
                common::debug::DebugType::PARSER,
                "Returning an immediate command from REPL");
            return std::make_shared<command::Command>(
                context,
                actions,
                condition);
        } else {
            // probably a really cryptic parse exception, but shouldn't occur
            // unless you really mess up
            throw ParseException("Cannot infer events with unknown context\n");
        }
    } else {
        common::debug::logln(
            common::debug::DebugType::PARSER,
            "Returning a callback with user events");
        return std::make_shared<command::CallbackCommand>(
            context,
            actions,
            condition,
            std::set(event_list.begin(), event_list.end()));
    }
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

std::vector<event::EventType> Parser::parse_event_list() {
    common::debug::log(
        common::debug::DebugType::PARSER,
        "parse_event_list()\n");
    auto event = parse_event();
    if(lexer.peek().token_type == TokenType::SEMICOLON) {
        expect(TokenType::SEMICOLON);
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

Parser::ActionPtrList Parser::parse_action_list(TokenType SEP) {
    common::debug::log(
        common::debug::DebugType::PARSER,
        "parse_action_list()\n");
    auto action = parse_action();
    if(lexer.peek().token_type == SEP) {
        expect(SEP);
        auto action_list = parse_action_list(SEP);
        action_list.insert(action_list.begin(), action);
        return action_list;
    } else return {action};
}

action::ActionPtr Parser::parse_action() {
    common::debug::log(common::debug::DebugType::PARSER, "parse_action()\n");
    if(lexer.peek().token_type == TokenType::STOP) {
        expect(TokenType::STOP);
        ParenParserRAII ppRAII(this);
        return std::make_shared<action::Stop>();
    } else if(lexer.peek().token_type == TokenType::PAUSE) {
        expect(TokenType::PAUSE);
        ParenParserRAII ppRAII(this);
        return std::make_shared<action::Pause>();
    } else if(lexer.peek().token_type == TokenType::RESUME) {
        expect(TokenType::RESUME);
        ParenParserRAII ppRAII(this);
        return std::make_shared<action::Resume>();
    } else if(lexer.peek().token_type == TokenType::DISASM) {
        expect(TokenType::DISASM);
        ParenParserRAII ppRAII(this);
        auto expr = parse_expr();
        return std::make_shared<action::Disasm>(expr);
    } else if(lexer.peek().token_type == TokenType::DUMP) {
        expect(TokenType::DUMP);
        ParenParserRAII ppRAII(this);
        auto dump_args = parse_dump_arg_list();
        return std::make_shared<action::Dump>(
            dump_args.begin(),
            dump_args.end());
    } else if(lexer.peek().token_type == TokenType::WATCH) {
        expect(TokenType::WATCH);
        ParenParserRAII ppRAII(this);
        auto [watch_expr, watch_args] = parse_watch_args();
        return std::make_shared<action::Watch>(
            watch_expr,
            watch_args.begin(),
            watch_args.end());
    } else if(lexer.peek().token_type == TokenType::SET) {
        expect(TokenType::SET);
        ParenParserRAII ppRAII(this);
        auto lhs = parse_lvalue_expr();
        expect(TokenType::EQUALS);
        auto rhs = parse_expr();
        return std::make_shared<action::Set>(lhs, rhs);
    } else throw ParseException("Unknown action: " + lexer.peek().getString());
}

action::Dump::DumpArg Parser::parse_dump_arg() {
    common::debug::log(common::debug::DebugType::PARSER, "parse_dump_arg()\n");
    if(lexer.peek().token_type == TokenType::STRING) {
        auto s_tok = expect(TokenType::STRING);
        return s_tok.lexeme;
    } else {
        auto e = parse_expr();
        return e;
    }
}
std::vector<action::Dump::DumpArg> Parser::parse_dump_arg_list() {
    common::debug::log(
        common::debug::DebugType::PARSER,
        "parse_dump_arg_list()\n");
    auto arg = parse_dump_arg();
    if(lexer.peek().token_type == TokenType::COMMA) {
        expect(TokenType::COMMA);
        auto arg_list = parse_dump_arg_list();
        arg_list.insert(arg_list.begin(), arg);
        return arg_list;
    } else return {arg};
}

std::pair<command::ExprPtr, Parser::ActionPtrList> Parser::parse_watch_args() {
    common::debug::log(
        common::debug::DebugType::PARSER,
        "parse_watch_args()\n");
    auto e = parse_lvalue_expr();
    expect(TokenType::COMMA);
    auto actions = parse_action_list(TokenType::COMMA);
    return {e, actions};
}

command::ExprPtr Parser::parse_lvalue_expr() {
    common::debug::log(
        common::debug::DebugType::PARSER,
        "parse_lvalue_expr()\n");
    auto e = parse_expr();
    if(!e->isLValue())
        throw ParseException("'" + e->getString() + "' is not an lvalue");
    return e;
}

command::ExprPtr Parser::parse_expr() {
    common::debug::log(common::debug::DebugType::PARSER, "parse_expr()\n");
    std::vector<Token> input;
    long parenCount = 0;
    while(ExprParser::isExprToken(lexer.peek())) {
        auto t = lexer.getToken();
        if(t.token_type == TokenType::LPAREN) parenCount++;
        if(t.token_type == TokenType::RPAREN) parenCount--;
        input.push_back(t);
    }
    // while parenCount is lt 0 and last elm in 'input' is an RPAREN
    // it should not be included in the 'input'
    while(parenCount < 0 && input.back().token_type == TokenType::RPAREN) {
        auto t = input.back();
        input.pop_back();
        lexer.ungetToken(t); // put it back
        parenCount++;
    }
    ExprParser ep(input);
    auto expr = ep.parse();
    return expr;
}

Parser::ActionPtrList Parser::make_action_group(const ActionPtrList& actions) {
    // only make an action group if there is more than 1 action
    if(actions.size() > 1) {
        auto action_group = std::make_shared<action::ActionGroup>(actions);
        return {action_group};
    } else return actions;
}

} // namespace parser
} // namespace ishell
