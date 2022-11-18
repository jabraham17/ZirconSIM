


example debufing
./build/bin/zircon test/envp-musl-debug.out -control hart:before_execute pc\<=0x105b8,pc\>=0x104ec pc -control hart:after_execute pc\<=0x105b8,pc\>=0x104ec gpr -e VAR=VALUE -- he hh

