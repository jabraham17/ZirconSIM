


example debufing
./build/bin/zircon test/envp-musl-debug.out -control hart:before_execute pc\<=0x105b8,pc\>=0x104ec pc -control hart:after_execute pc\<=0x105b8,pc\>=0x104ec gpr -e VAR=VALUE -- he hh



./build/bin/zircon test/envp-elf-debug.out -control hart:after_execute pc\<=0x16e7c,pc\>=0x16e10 pc,gpr[9],gpr[10] -control hart:after_execute pc\>=0x1d400,pc\<=0x1d454 pc,gpr[9],gpr[10] -control hart:after_execute pc\>=0x1fe8c,pc\<=0x1feec pc,gpr[9],gpr[10] -control watch m[0x7fffffff0000fb88] -control watch gpr[9]


/build/bin/zircon test/envp-elf-debug.out -control watch m[0x7fffffff0000fb88] -control watch gpr[9] -control hart:before_execute pc=0x1fec0 pc,m[0x7fffffff0000fb88] -I -R --color
