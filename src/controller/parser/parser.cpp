#include "parser.h"
#include "event/event.h"
#include <utility>

namespace controller {
namespace parser {

Address strToAddress(std::string s) { return std::stoull(s); }
RegisterIndex strToRegisterIndex(std::string s) { return std::stoull(s); }
Integer strToInteger(std::string s) { return std::stoull(s); }

Token Parser::expect(TokenType tt) {
    auto t = lexer.getToken();
    if(t.token_type == tt) {
        return t;
    }
    throw ParseException(
        "Expected '" + std::string(tt) + "' got '" + std::string(t.token_type) +
        "'");
}
ControlList Parser::parse() {
    try {
        auto controls = parse_controls();
        expect(TokenType::END_OF_FILE);
        return ControlList(controls);
    } catch(const ParseException& e) {
        *log << e.what() << "\n";
        return ControlList({});
    }
}

// controls -> control | control controls
std::vector<std::shared_ptr<ControlBase>> Parser::parse_controls() {
    auto control = parse_control();
    if(lexer.peek().token_type == TokenType::SUBSYSTEM || lexer.peek().token_type == TokenType::WATCH) {
        auto control_list = parse_controls();
        control_list.insert(control_list.begin(), control);
        return control_list;
    } else {
        return {control};
    }
}

// control -> event_action | event_cond_action | watch_stmt
std::shared_ptr<ControlBase> Parser::parse_control() {
    // first of event_action and event_cond_action is both SUBSYSTEM COLON EVENT
    // then first of cond is either 1 thing then equals or 4 things then equals
    if(lexer.peek().token_type == TokenType::WATCH) {
        return parse_watch_stmt();
    }
    else if(is_cond_op(lexer.peek(5).token_type) ||
       is_cond_op(lexer.peek(8).token_type)) {
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
            auto [reg_class, idx] = parse_register();
            return std::make_shared<action::DumpRegister>(reg_class, idx);
        } else {
            auto reg_class = expect(TokenType::REGISTER_CLASS);
            auto reg_class_type =
                isa::rf::getRegisterClassType(reg_class.lexeme);
            if(reg_class_type == isa::rf::RegisterClassType::NONE)
                throw ParseException("Invalid Register Class Type");
            return std::make_shared<action::DumpRegisterClass>(reg_class_type);
        }
    } else if(lexer.peek().token_type == TokenType::PC) {
        expect(TokenType::PC);
        return std::make_shared<action::DumpPC>();
    } else if(lexer.peek().token_type == TokenType::MEM) {
        auto addr = parse_mem();
        return std::make_shared<action::DumpMemoryAddress>(addr);
    } else if(lexer.peek().token_type == TokenType::STOP) {
        expect(TokenType::STOP);
        return std::make_shared<action::Stop>();
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
        auto [reg_class, reg_idx] = parse_register();
        auto ct = parse_cond_op();
        auto num = expect(TokenType::NUM);
        return std::make_shared<condition::RegisterCompare>(
            reg_class,
            reg_idx,
            strToInteger(num.lexeme),
            ct);

    } else if(lexer.peek().token_type == TokenType::MEM) {
        auto addr = parse_mem();
        auto ct = parse_cond_op();
        auto num = expect(TokenType::NUM);
        return std::make_shared<condition::MemAddrCompare>(
            addr,
            strToInteger(num.lexeme),
            ct);

    } else if(lexer.peek().token_type == TokenType::PC) {
        expect(TokenType::PC);
        auto ct = parse_cond_op();
        auto num = expect(TokenType::NUM);
        return std::make_shared<condition::PCCompare>(
            strToAddress(num.lexeme),
            ct);

    } else throw ParseException("Unknown Condition");
}

// cond_op -> EQUALS | NOTEQUAL
// cond_op -> LESSTHAN | LESSTHAN_EQUALTO | GREATERTHAN_EQUALTO
condition::ComparisonType Parser::parse_cond_op() {
    auto t = lexer.getToken();
    switch(t.token_type) {
        case TokenType::EQUALS: return condition::ComparisonType::EQ;
        case TokenType::NOTEQUAL: return condition::ComparisonType::NEQ;
        case TokenType::LESSTHAN: return condition::ComparisonType::LT;
        case TokenType::LESSTHAN_EQUALTO: return condition::ComparisonType::LTE;
        case TokenType::GREATERTHAN: return condition::ComparisonType::GT;
        case TokenType::GREATERTHAN_EQUALTO:
            return condition::ComparisonType::GTE;
        default: throw ParseException("Unknown Comparison Op");
    }
}
bool Parser::is_cond_op(TokenType tt) {
    return tt == TokenType::EQUALS || tt == TokenType::NOTEQUAL ||
           tt == TokenType::LESSTHAN || tt == TokenType::LESSTHAN_EQUALTO ||
           tt == TokenType::GREATERTHAN || tt == TokenType::GREATERTHAN_EQUALTO;
}

// register -> REGISTER_CLASS LBRACK NUM RBRACK
std::pair<isa::rf::RegisterClassType, RegisterIndex> Parser::parse_register() {
    auto reg_file = expect(TokenType::REGISTER_CLASS);
    expect(TokenType::LBRACK);
    auto num = expect(TokenType::NUM);
    expect(TokenType::RBRACK);

    auto reg_class = isa::rf::getRegisterClassType(reg_file.lexeme);
    if(reg_class == isa::rf::RegisterClassType::NONE)
        throw ParseException("Invalid Register Class Type");
    return std::make_pair(reg_class, strToRegisterIndex(num.lexeme));
}

// mem -> MEM LBRACK NUM RBRACK
Address Parser::parse_mem() {
    expect(TokenType::MEM);
    expect(TokenType::LBRACK);
    auto num = expect(TokenType::NUM);
    expect(TokenType::RBRACK);

    return strToAddress(num.lexeme);
}

// watch_stmt -> WATCH register | WATCH mem
std::shared_ptr<Watch> Parser::parse_watch_stmt() {
    expect(TokenType::WATCH);
    if(lexer.peek().token_type == TokenType::REGISTER_CLASS && lexer.peek(2).token_type == TokenType::LBRACK) {
        auto [reg_class, idx] = parse_register();
        return std::make_shared<WatchRegister>(reg_class, idx);
        
    } else if(lexer.peek().token_type == TokenType::MEM) {
        auto addr = parse_mem();
        return std::make_shared<WatchMemoryAddress>(addr);
    } 
    else {
        throw ParseException("Unknown Watch Statement");
    }
}

} // namespace parser
} // namespace controller
