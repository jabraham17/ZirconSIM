#ifndef ZIRCON_COMMAND_PARSER_PARSER_H_
#define ZIRCON_COMMAND_PARSER_PARSER_H_

#include "lexer.h"

#include "command/command.h"
#include "command/expr.h"
#include "hart/isa/rf.h"

#include <optional>
#include <ostream>
#include <stdexcept>
#include <type_traits>

namespace ishell {

namespace parser {
struct ParseException : public std::runtime_error {
    ParseException(std::string message)
        : std::runtime_error("Parse Exception: " + message) {}
};

class Parser {
  private:
    using Control_ptr = std::shared_ptr<command::ControlBase>;
    using Watch_ptr = std::shared_ptr<command::Watch>;
    using Command_ptr = std::shared_ptr<command::Command>;
    using Condition_ptr = std::shared_ptr<command::Condition>;
    using Expr_ptr = std::shared_ptr<command::Expr>;
    using Action_ptr = std::shared_ptr<command::action::ActionInterface>;

  public:
    Lexer lexer;
    std::ostream* log;
    Parser(std::string input, std::ostream& log = std::cout)
        : lexer(input), log(&log) {}
    Control_ptr parse();

    /*
    control         -> action_command | watch_command
    watch_command   -> WATCH REGISTER action_list
    watch_command   -> WATCH MEM LBRACK expr RBRACK action_list
    action_command  -> action_list if_statement on_staptement
    if_statement    -> IF expr | EPSILON
    on_statement    -> ON event_list | EPSILON
    action_list     -> action | action COMMA action_list
    event_list      -> event | event COMMA event_list
    event           -> SUBSYSTEM COLON EVENT
    action          -> STOP | PAUSE | RESUME | DISASM expr | DUMP expr
    */

  private:
    Token expect(TokenType tt);

    Control_ptr parse_control();
    Watch_ptr parse_watch_command();
    Command_ptr parse_action_command();
    Condition_ptr parse_if_statement();
    std::vector<event::EventType> parse_on_statement();
    std::vector<Action_ptr> parse_action_list();
    std::vector<event::EventType> parse_event_list();
    event::EventType parse_event();
    Action_ptr parse_action();
    Expr_ptr parse_expr();

    // helpers

    // builds an action group if it makes sense to do it
    std::vector<Action_ptr> make_action_group(const std::vector<Action_ptr>&);
};

} // namespace parser
} // namespace ishell

#endif
