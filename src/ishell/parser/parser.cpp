#include "parser.h"

#include "event/event.h"

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
    auto controls = parse_controls();
    expect(TokenType::END_OF_FILE);
    return command::ControlList(controls);
}

// controls -> control | control controls
std::vector<std::shared_ptr<command::ControlBase>> Parser::parse_controls() {
    auto control = parse_control();
    if(lexer.peek().token_type == TokenType::SUBSYSTEM ||
       lexer.peek().token_type == TokenType::WATCH) {
        auto control_list = parse_controls();
        control_list.insert(control_list.begin(), control);
        return control_list;
    } else {
        return {control};
    }
}

// control -> event_action | event_cond_action | watch_stmt
std::shared_ptr<command::ControlBase> Parser::parse_control() {
    // first of event_action and event_cond_action is both SUBSYSTEM COLON EVENT
    // then first of cond is either 1 thing then equals or 4 things then equals
    if(lexer.peek().token_type == TokenType::WATCH) {
        return parse_watch_stmt();
    } else if(
        is_cond_op(lexer.peek(5).token_type) ||
        is_cond_op(lexer.peek(8).token_type)) {
        return parse_event_cond_action();
    } else {
        return parse_event_action();
    }
}

// event_action -> event action_list
std::shared_ptr<command::Command> Parser::parse_event_action() {
    auto ev = parse_event();
    auto act = parse_action_list();
    // keep these as a an action list, not group
    // because they are unconditionally run
    return std::make_shared<command::Command>(ev, act);
}

// event_cond_action -> event cond_list action_list
std::shared_ptr<command::ConditionalCommand> Parser::parse_event_cond_action() {
    auto ev = parse_event();
    auto cond = parse_cond_list();
    auto act = parse_action_list();
    // turn actions into action group if more than 1
    if(act.size() > 1) {
        std::vector<std::shared_ptr<command::action::ActionInterface>>
            action_group = {
                std::make_shared<command::action::ActionGroup>(act)};
        return std::make_shared<command::ConditionalCommand>(
            ev,
            cond,
            action_group);
    } else return std::make_shared<command::ConditionalCommand>(ev, cond, act);
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
std::vector<std::shared_ptr<command::action::ActionInterface>>
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

// action -> REGISTER_CLASS | register | pc | mem | STOP | PAUSE
std::shared_ptr<command::action::ActionInterface> Parser::parse_action() {
    if(lexer.peek().token_type == TokenType::REGISTER_CLASS) {
        if(lexer.peek(2).token_type == TokenType::LBRACK) {
            auto [reg_class, idx] = parse_register();
            return std::make_shared<command::action::DumpRegister>(
                reg_class,
                idx);
        } else {
            auto reg_class = expect(TokenType::REGISTER_CLASS);
            auto reg_class_type =
                isa::rf::getRegisterClassType(reg_class.lexeme);
            if(reg_class_type == isa::rf::RegisterClassType::NONE)
                throw ParseException("Invalid Register Class Type");
            return std::make_shared<command::action::DumpRegisterClass>(
                reg_class_type);
        }
    } else if(lexer.peek().token_type == TokenType::PC) {
        auto pc_offset = parse_pc();
        return std::make_shared<command::action::DumpPC>(pc_offset);
    } else if(lexer.peek().token_type == TokenType::MEM) {
        auto addr = parse_mem();
        return std::make_shared<command::action::DumpMemoryAddress>(addr);
    } else if(lexer.peek().token_type == TokenType::STOP) {
        expect(TokenType::STOP);
        return std::make_shared<command::action::Stop>();
    } else if(lexer.peek().token_type == TokenType::PAUSE) {
        expect(TokenType::PAUSE);
        return std::make_shared<command::action::Pause>();
    } else throw ParseException("Unknown Action");
}

// cond_list -> cond | cond COMMA cond_list
std::vector<std::shared_ptr<command::condition::ConditionInterface>>
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

// cond -> register EQUALS NUM | mem EQUALS NUM | pc EQUALS NUM
std::shared_ptr<command::condition::ConditionInterface> Parser::parse_cond() {
    if(lexer.peek().token_type == TokenType::REGISTER_CLASS) {
        auto [reg_class, reg_idx] = parse_register();
        auto ct = parse_cond_op();
        auto num = expect(TokenType::NUM);
        return std::make_shared<command::condition::RegisterCompare>(
            reg_class,
            reg_idx,
            types::strToUnsignedInteger(num.lexeme),
            ct);

    } else if(lexer.peek().token_type == TokenType::MEM) {
        auto addr = parse_mem();
        auto ct = parse_cond_op();
        auto num = expect(TokenType::NUM);
        return std::make_shared<command::condition::MemAddrCompare>(
            addr,
            types::strToUnsignedInteger(num.lexeme),
            ct);

    } else if(lexer.peek().token_type == TokenType::PC) {
        auto pc_offset = parse_pc();
        auto ct = parse_cond_op();
        auto num = expect(TokenType::NUM);
        return std::make_shared<command::condition::PCCompare>(
            pc_offset,
            types::strToAddress(num.lexeme),
            ct);

    } else throw ParseException("Unknown Condition");
}

// cond_op -> EQUALS | NOTEQUAL
// cond_op -> LESSTHAN | LESSTHAN_EQUALTO | GREATERTHAN_EQUALTO
command::condition::ComparisonType Parser::parse_cond_op() {
    auto t = lexer.getToken();
    switch(t.token_type) {
        case TokenType::EQUALS: return command::condition::ComparisonType::EQ;
        case TokenType::NOTEQUAL:
            return command::condition::ComparisonType::NEQ;
        case TokenType::LESSTHAN: return command::condition::ComparisonType::LT;
        case TokenType::LESSTHAN_EQUALTO:
            return command::condition::ComparisonType::LTE;
        case TokenType::GREATERTHAN:
            return command::condition::ComparisonType::GT;
        case TokenType::GREATERTHAN_EQUALTO:
            return command::condition::ComparisonType::GTE;
        default: throw ParseException("Unknown Comparison Op");
    }
}
bool Parser::is_cond_op(TokenType tt) {
    return tt == TokenType::EQUALS || tt == TokenType::NOTEQUAL ||
           tt == TokenType::LESSTHAN || tt == TokenType::LESSTHAN_EQUALTO ||
           tt == TokenType::GREATERTHAN || tt == TokenType::GREATERTHAN_EQUALTO;
}

// register -> REGISTER_CLASS LBRACK NUM RBRACK
std::pair<isa::rf::RegisterClassType, types::RegisterIndex>
Parser::parse_register() {
    auto reg_file = expect(TokenType::REGISTER_CLASS);
    expect(TokenType::LBRACK);
    auto num = expect(TokenType::NUM);
    expect(TokenType::RBRACK);

    auto reg_class = isa::rf::getRegisterClassType(reg_file.lexeme);
    if(reg_class == isa::rf::RegisterClassType::NONE)
        throw ParseException("Invalid Register Class Type");
    return std::make_pair(reg_class, types::strToRegisterIndex(num.lexeme));
}

// mem -> MEM LBRACK NUM RBRACK
types::Address Parser::parse_mem() {
    expect(TokenType::MEM);
    expect(TokenType::LBRACK);
    auto num = expect(TokenType::NUM);
    expect(TokenType::RBRACK);

    return types::strToAddress(num.lexeme);
}

// watch_stmt -> WATCH register | WATCH mem | WATCH register action_list | WATCH
// mem action_list
std::shared_ptr<command::Watch> Parser::parse_watch_stmt() {
    expect(TokenType::WATCH);
    std::shared_ptr<command::Watch> watch;
    if(lexer.peek().token_type == TokenType::REGISTER_CLASS &&
       lexer.peek(2).token_type == TokenType::LBRACK) {
        auto [reg_class, idx] = parse_register();
        watch = std::make_shared<command::WatchRegister>(reg_class, idx);

    } else if(lexer.peek().token_type == TokenType::MEM) {
        auto addr = parse_mem();
        watch = std::make_shared<command::WatchMemoryAddress>(addr);
    } else {
        throw ParseException("Unknown Watch Statement");
    }
    std::vector<std::shared_ptr<command::action::ActionInterface>> actions;
    if(lexer.peek().token_type == TokenType::REGISTER_CLASS ||
       lexer.peek().token_type == TokenType::PC ||
       lexer.peek().token_type == TokenType::MEM ||
       lexer.peek().token_type == TokenType::STOP) {
        actions = parse_action_list();
    }
    if(watch) {
        // turn actions into action group if more than 1
        if(actions.size() > 1) {
            auto action_group =
                std::make_shared<command::action::ActionGroup>(actions);
            watch->setActions({action_group});
        } else {
            watch->setActions(actions);
        }
    }

    return watch;
}

// pc -> PC | PC PLUS NUM | PC MINUS NUM
types::SignedInteger Parser::parse_pc() {
    expect(TokenType::PC);
    if(lexer.peek().token_type == TokenType::PLUS) {
        expect(TokenType::PLUS);
        auto num = expect(TokenType::NUM);
        return types::strToSignedInteger(num.lexeme);
    } else if(lexer.peek().token_type == TokenType::MINUS) {
        expect(TokenType::MINUS);
        auto num = expect(TokenType::NUM);
        return -types::strToSignedInteger(num.lexeme);
    } else {
        // offset of 0 to pc
        return 0;
    }
}

} // namespace parser
} // namespace ishell
