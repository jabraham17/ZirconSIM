# iShell

The iShell is an interactive shell that allows the user to control many aspects of the simulator.
iShell commands can be run at startup via the command line or executed from an interactive mode.

## Basic Command Syntax

There are two basic commands, action commands and watch commands.

Action commands have the form `actions [if condition] [on events]`.
A single command can express  multiple actions, separated by commas.
Both the `if`  and `on` statements are options.
`if` statements prevent all actions from being executed unless the condition is true.
Events can be specified as `subsystem:event` as a command separated list.
If the command is executed as a callback, then the event(s) specify which callback to install the action(s).

Watch commands have the form `watch expression actions`.
Watch commands will execute their actions if any value in the expression changes.

**Note:** Watch commands in their described form do not work this way.
An arbitrary expression will be evaluated and watch that memory address.
A single register can be watched as expected.
The current implementation is powerful but can act in surprising ways, use with caution and know that the above is the planned implementation.

## Command List

- `watch`
  - watch a value and execute a list of actions when it changes
- `stop`
  - stop execution of the simulator and exit
- `pause`
  - pause execution of the simulator and enter the interactive shell
- `resume`
  - resume execution of the simulator from the paused state
  - likely only useful from the interactive shell, but available everywhere
- `disasm <expr>`
  - evaluates the expression and attempts to disassemble it as a RISC-V instruction
- `dump <expr>`
  - evaluates the expression and prints the result as a signed 64-bit integer

## Grammar

All keywords such as `WATCH` and `DUMP` are case-insensitive.

```default
control        -> action_command | watch_command
watch_command  -> WATCH REGISTER action_list
watch_command  -> WATCH expr action_list
action_command -> action_list if_statement on_statement
if_statement   -> IF expr | EPSILON
on_statement   -> ON event_list | EPSILON
action_list    -> action | action COMMA action_list
event_list     -> event | event COMMA event_list
event          -> SUBSYSTEM COLON EVENT
action         -> STOP | PAUSE | RESUME | DISASM expr | DUMP expr
expr           -> expr * expr
expr           -> expr / expr
expr           -> expr + expr
expr           -> expr - expr
expr           -> expr << expr
expr           -> expr >> expr
expr           -> expr < expr
expr           -> expr <= expr
expr           -> expr > expr
expr           -> expr >= expr
expr           -> expr == expr
expr           -> expr ~= expr
expr           -> expr & expr
expr           -> expr | expr
expr           -> expr && expr
expr           -> expr || expr
expr           -> - expr
expr           -> ~ expr
expr           -> ! expr
expr           -> ( expr )
expr           -> $m [ expr ]
expr           -> $reg_name
expr           -> 0-9
expr           -> $pc
```

## Callback Execution Model

Commands can be installed as a callback on any number of events.
Every time an event occurs, it will fire all of its associated callbacks.

## Command Line Usage

The command line flag `-control SEQUENCE` allows a command to be entered at startup.
iShell commands executed in this way are installed as callbacks.

## Interactive Usage \[BETA\]

Currently there are only three ways to launch the interactive shell and only two are recommended.
All three ways put the simulator in a paused state, which allow the shell to appear.
Commands executed from here are currently executed immediately.

- The simulator executes an ebreak
- A callback is installed that executes `pause`
- **NOT RECOMMENDED** use the `--start-paused` command line flag
  - Command is 'racy', only works spuriously due to a race condition

**Note:** Design work is being done on the interactive shell and we caution users to not use it for now.
