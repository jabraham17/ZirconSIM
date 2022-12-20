#ifndef ZIRCON_CONTROLLER_PARSER_PARSER_H_
#define ZIRCON_CONTROLLER_PARSER_PARSER_H_

#include "lexer.h"

#include "command/command.h"
#include "hart/isa/rf.h"
#include "hart/types.h"

#include <optional>
#include <ostream>
#include <stdexcept>
#include <type_traits>

namespace controller {

namespace parser {
struct ParseException : public std::runtime_error {
    ParseException(std::string message)
        : std::runtime_error("Parse Exception: " + message) {}
};

class Parser {
  public:
    Lexer lexer;
    std::ostream* log;
    Parser(std::vector<std::string> input, std::ostream& log = std::cout)
        : lexer(input), log(&log) {
        // Token t = lexer.getToken();
        // while(t.token_type != TokenType::END_OF_FILE) {
        //     std::cout << t.getString() << "\n";
        //     t = lexer.getToken();
        // }
    }
    command::ControlList parse();

    /*
    controls          -> control | control controls
    control           -> event_action | event_cond_action | watch_stmt
    event_action      -> event action_list
    event_cond_action -> event cond_list action_list
    event             -> SUBSYSTEM COLON EVENT
    action_list       -> action | action COMMA action_list
    action            -> REGISTER_CLASS | register | pc | mem | STOP | PAUSE
    cond_list         -> cond | cond COMMA cond_list
    cond              -> register EQUALS NUM | mem EQUALS NUM | pc EQUALS NUM
    cond_op           -> EQUALS | NOTEQUAL
    cond_op           -> LESSTHAN | LESSTHAN_EQUALTO | GREATERTHAN_EQUALTO
    register          -> REGISTER_CLASS LBRACK NUM RBRACK
    mem               -> MEM LBRACK NUM RBRACK
    watch_stmt        -> WATCH register | WATCH mem
    watch_stmt        -> WATCH register action_list | WATCH mem action_list
    pc                -> PC | PC PLUS NUM | PC MINUS NUM
    */

  private:
    Token expect(TokenType tt);

    std::vector<std::shared_ptr<command::ControlBase>> parse_controls();
    std::shared_ptr<command::ControlBase> parse_control();
    std::shared_ptr<command::Command> parse_event_action();
    std::shared_ptr<command::ConditionalCommand> parse_event_cond_action();
    event::EventType parse_event();
    std::vector<std::shared_ptr<command::action::ActionInterface>>
    parse_action_list();
    std::shared_ptr<command::action::ActionInterface> parse_action();
    std::vector<std::shared_ptr<command::condition::ConditionInterface>>
    parse_cond_list();
    std::shared_ptr<command::condition::ConditionInterface> parse_cond();
    command::condition::ComparisonType parse_cond_op();
    bool is_cond_op(TokenType tt);
    std::pair<isa::rf::RegisterClassType, types::RegisterIndex>
    parse_register();
    types::Address parse_mem();
    std::shared_ptr<command::Watch> parse_watch_stmt();
    types::SignedInteger parse_pc();
};

} // namespace parser
} // namespace controller

#endif
