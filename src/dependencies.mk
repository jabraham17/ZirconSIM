
cpu= 
elf= mem
mem= 
event= 
controller= event cpu mem
zircon= controller cpu elf mem trace event color common
zircon-wasm= controller cpu elf mem trace event color common

define make_depen
$(eval $1: $($1))
endef
map = $(foreach a,$(2),$(call $(1),$(a)))
define make_prereqs
$(call map,make_depen,cpu mem elf trace zircon zircon-wasm color event controller common)
endef
