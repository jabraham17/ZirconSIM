#include "parser.h"
#include "event/event.h"
#include <utility>

namespace controller {
namespace parser {

Address strToAddress(std::string s) { return std::stoull(s); }
RegisterIndex strToRegisterIndex(std::string s) { return std::stoull(s); }

Token Parser::expect(TokenType tt) {
    auto t = lexer.getToken();
    if(t.token_type == tt) {
        return t;
    }
    throw ParseException(
        "Expected '" + std::string(tt) + "' got '" + std::string(t.token_type) +
        "'");
}
CommandList Parser::parse() {
    try {
        auto controls = parse_controls();
        expect(TokenType::END_OF_FILE);
        return CommandList(controls);
    } catch(const ParseException& e) {
        *log << e.what() << "\n";
        return CommandList({});
    }
}

// ./build/bin/zircon test/main-musl-debug.out -control mem:read pc

// controls -> control | control controls
std::vector<std::shared_ptr<Command>> Parser::parse_controls() {
    auto control = parse_control();
    if(lexer.peek().token_type == TokenType::SUBSYSTEM) {
        auto control_list = parse_controls();
        control_list.insert(control_list.begin(), control);
        return control_list;
    } else {
        return {control};
    }
}

// control -> event_action | event_cond_action
std::shared_ptr<Command> Parser::parse_control() {
    // first of event_action and event_cond_action is both SUBSYSTEM COLON EVENT
    // then first of cond is either 1 thing then equals or 4 things then equals
    if(lexer.peek(5).token_type == TokenType::EQUALS ||
       lexer.peek(8).token_type == TokenType::EQUALS) {
        return parse_event_cond_action();
    } else {
        return parse_event_action();
    }
}

// event_action -> event action_list
std::shared_ptr<Command> Parser::parse_event_action() {
    auto ev = parse_event();
    auto act = parse_action_list();
    return std::make_shared<Command>(ev, act);
}

// event_cond_action -> event cond_list action_list
std::shared_ptr<ConditionalCommand> Parser::parse_event_cond_action() {
    auto ev = parse_event();
    auto cond = parse_cond_list();
    auto act = parse_action_list();
    return std::make_shared<ConditionalCommand>(ev, cond, act);
}

// event -> SUBSYSTEM COLON EVENT
event::EventType Parser::parse_event() {
    auto sub = expect(TokenType::SUBSYSTEM);
    expect(TokenType::COLON);
    auto ev = expect(TokenType::EVENT);
    auto sub_ev = event::getEventType(sub.lexeme + "_" + ev.lexeme);
    if(sub_ev == event::EventType::NONE) throw ParseException("Unknown event");
    return sub_ev;
}

// action_list -> action | action COMMA action_list
std::vector<std::shared_ptr<action::ActionInterface>>
Parser::parse_action_list() {
    auto action = parse_action();
    if(lexer.peek().token_type == TokenType::COMMA) {
        expect(TokenType::COMMA);
        auto action_list = parse_action_list();
        action_list.insert(action_list.begin(), action);
        return action_list;
    } else {
        return {action};
    }
}

// action -> REGISTER_CLASS | register | PC | mem | STOP
std::shared_ptr<action::ActionInterface> Parser::parse_action() {
    if(lexer.peek().token_type == TokenType::REGISTER_CLASS) {
        if(lexer.peek(2).token_type == TokenType::LBRACK) {
            auto reg = parse_register();
            return nullptr;
            // TODO:
            // return std::make_shared<action::DumpRegister>(
            //     nullptr,
            //     reg->second);
        } else {
            // TODO
            auto reg_class = expect(TokenType::REGISTER_CLASS);
            auto reg_class_type =
                isa::rf::getRegisterClassType(reg_class.lexeme);
            if(reg_class_type == isa::rf::RegisterClassType::NONE)
                throw ParseException("Invalid Register Class Type");
            // return nullptr;
            return std::make_shared<action::DumpRegisterClass>(
                nullptr,
                reg_class_type);
        }
    } else if(lexer.peek().token_type == TokenType::PC) {
        expect(TokenType::PC);
        return std::make_shared<action::DumpPC>(nullptr);
    } else if(lexer.peek().token_type == TokenType::MEM) {
        auto addr = parse_mem();
        return std::make_shared<action::DumpMemoryAddress>(nullptr, addr);

    } else if(lexer.peek().token_type == TokenType::STOP) {
        expect(TokenType::STOP);
        return std::make_shared<action::Stop>(nullptr);
    } else throw ParseException("Unknown Action");
}

// cond_list -> cond | cond COMMA cond_list
std::vector<std::shared_ptr<condition::ConditionInterface>>
Parser::parse_cond_list() {
    auto cond = parse_cond();
    if(lexer.peek().token_type == TokenType::COMMA) {
        expect(TokenType::COMMA);
        auto cond_list = parse_cond_list();
        cond_list.insert(cond_list.begin(), cond);
        return cond_list;
    } else {
        return {cond};
    }
}

// cond -> register EQUALS NUM | mem EQUALS NUM | PC EQUALS NUM
std::shared_ptr<condition::ConditionInterface> Parser::parse_cond() {
    if(lexer.peek().token_type == TokenType::REGISTER_CLASS) {
        auto reg = parse_register();
        expect(TokenType::EQUALS);
        auto num = expect(TokenType::NUM);
        // TODO:
        return std::make_shared<condition::AlwaysTrue>();

    } else if(lexer.peek().token_type == TokenType::MEM) {
        auto addr = parse_mem();
        expect(TokenType::EQUALS);
        auto num = expect(TokenType::NUM);

        // TODO:
        return std::make_shared<condition::AlwaysTrue>();

    } else if(lexer.peek().token_type == TokenType::PC) {
        expect(TokenType::PC);
        expect(TokenType::EQUALS);
        auto num = expect(TokenType::NUM);
        return std::make_shared<condition::PCEquals>(
            nullptr,
            strToAddress(num.lexeme));

    } else throw ParseException("Unknown Condition");
}

// register -> REGISTER_CLASS LBRACK NUM RBRACK
std::pair<isa::rf::RegisterClassType, RegisterIndex> Parser::parse_register() {
    auto reg_file = expect(TokenType::REGISTER_CLASS);
    expect(TokenType::LBRACK);
    auto num = expect(TokenType::NUM);
    expect(TokenType::RBRACK);

    // TODO: need to do error checking here
    return std::make_pair(
        isa::rf::getRegisterClassType(reg_file.lexeme),
        strToRegisterIndex(num.lexeme));
}

// mem -> MEM LBRACK NUM RBRACK
Address Parser::parse_mem() {
    expect(TokenType::MEM);
    expect(TokenType::LBRACK);
    auto num = expect(TokenType::NUM);
    expect(TokenType::RBRACK);

    return strToAddress(num.lexeme);
}
} // namespace parser
} // namespace controller
