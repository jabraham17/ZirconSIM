
# Commands

commands can be applied to specific events or run prior to instruction execution
commands consist of actions with optional conditions
there are also special watch commands
commands can also directly execute

action command string: ACTION_LIST <if COND> <on EVENT_LIST>
watch command string:  WATCH ACTION_LIST

if no condition supplied, always pefrom acrion
if no event supplied, perform prior to instruction execution

## actions
- stop
- pause
- resume
- disasm EXPR
  - resolve expr and dump as an instruction the memory
<!-- - mem EXPR
  - read and print memory address
  - same as 'dump $m\[EXPR\] -->
- dump EXPR
  - print result of expression
<!-- - reg CLASS
  - print all registers in class
- reg REG_NAME
  - print single register
  - works the same as 'dump $REG_NAME' -->

## conditions
- EXPR cond_op EXPR

## WATCH
- WATCH REG_NAME
- WATCH MEM_LOCATION

## expressions
expr -> LPAREN expr RPAREN
expr -> expr PLUS expr
expr -> expr MINUS expr
expr -> expr MULT expr
expr -> expr DIV expr
expr -> expr LSHIFT NUM
expr -> expr RSHIFT NUM
expr -> MEM LBRAC expr RBRAC
expr -> REGISTER
<!-- expr -> DOLLAR CLASS LBRAC NUM RBRAC -->
expr -> NUM
expr -> PC


MEM -> $m
REGISTER -> $reg_name
PC -> $pc



command -> action_command | watch_command
watch_command -> WATCH REGISTER action_list | WATCH MEM action_list
action_command -> action_list if_statement on_statement
if_statement -> IF cond | EPSILON
on_statement -> ON event_list | EPSILON
action_list -> action | action COMMA action_list
event_list -> event | event COMMA event_list
cond -> ppexpr cond_op ppexpr
cond_op -> EQUALS | NOTEQUAL | LESSTHAN | LESSTHAN_EQUALTO | GREATERTHAN | GREATERTHAN_EQUALTO | BOOLEAN_AND | BOOLEAN_OR
event -> SUBSYSTEM COLON EVENT
action -> STOP | PAUSE | RESUME | DISASM ppexpr | DUMP ppexpr

ppexpr -> expr
expr -> LPAREN expr RPAREN
expr -> expr PLUS expr
expr -> expr MINUS expr
expr -> expr MULT expr
expr -> expr DIV expr
expr -> expr LSHIFT NUM
expr -> expr RSHIFT NUM
expr -> expr BITWISE_AND NUM
expr -> expr BITWISE_OR NUM
expr -> MEM LBRAC expr RBRAC
expr -> REGISTER
expr -> NUM
expr -> PC
