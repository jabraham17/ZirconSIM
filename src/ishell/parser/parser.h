#ifndef ZIRCON_COMMAND_PARSER_PARSER_H_
#define ZIRCON_COMMAND_PARSER_PARSER_H_

#include "lexer.h"

#include "command/command.h"
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
  public:
    Lexer lexer;
    std::ostream* log;
    Parser(std::string input, std::ostream& log = std::cout)
        : lexer(input), log(&log) {}
    command::ControlList parse();

    /*
    command         -> action_command | watch_command
    watch_command   -> WATCH REGISTER action_list | WATCH MEM action_list
    action_command  -> action_list if_statement on_statement
    if_statement    -> IF cond | EPSILON
    on_statement    -> ON event_list | EPSILON
    action_list     -> action | action COMMA action_list
    event_list      -> event | event COMMA event_list
    cond            -> expr cond_op expr
    cond_op         -> EQUALS | NOTEQUAL | LESSTHAN | LESSTHAN_EQUALTO
    cond_op         -> GREATERTHAN | GREATERTHAN_EQUALTO
    cond_op         -> BOOLEAN_AND | BOOLEAN_OR
    event           -> SUBSYSTEM COLON EVENT
    action          -> STOP | PAUSE | RESUME | DISASM expr | DUMP expr


    */

  private:
    Token expect(TokenType tt);

    void parse_expr();


};

} // namespace parser
} // namespace ishell

#endif
