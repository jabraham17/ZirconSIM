#ifndef ZIRCON_ISHELL_PARSER_PARSER_H_
#define ZIRCON_ISHELL_PARSER_PARSER_H_

#include "lexer.h"

#include "command/action.h"
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
    using ActionPtrList = std::vector<action::ActionPtr>;

  public:
    Lexer lexer;
    Parser() = default;
    command::CommandPtr
    parse(std::string input, command::CommandContext context);
    std::vector<command::CommandPtr>
    parseAll(std::string input, command::CommandContext context);

  private:
    Token expect(TokenType tt);
    // use RAII (ie scoping) to parse matching parens if found
    class ParenParserRAII {
      private:
        Parser* parser;
        bool parseParen;

      public:
        ParenParserRAII(Parser* parser) : parser(parser), parseParen(false) {
            if(parser->lexer.peek().token_type == TokenType::LPAREN) {
                parser->expect(TokenType::LPAREN);
                parseParen = true;
            }
        }
        ~ParenParserRAII() {
            if(parseParen) {
                parser->expect(TokenType::RPAREN);
            }
        }
    };

    command::CommandPtr parse_command(command::CommandContext context);
    command::ExprPtr parse_if_statement();
    std::vector<event::EventType> parse_on_statement();

    std::vector<event::EventType> parse_event_list();
    event::EventType parse_event();

    ActionPtrList parse_action_list(TokenType SEP);
    action::ActionPtr parse_action();

    action::Dump::DumpArg parse_dump_arg();
    std::vector<action::Dump::DumpArg> parse_dump_arg_list();

    std::pair<command::ExprPtr, ActionPtrList> parse_watch_args();

    command::ExprPtr parse_lvalue_expr();
    command::ExprPtr parse_expr();

    // helpers

    // builds an action group if it makes sense to do it
    ActionPtrList make_action_group(const ActionPtrList&);
};

} // namespace parser
} // namespace ishell

#endif
