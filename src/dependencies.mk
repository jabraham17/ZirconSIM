
hart= 
elf= mem
mem= 
event= 
controller= command event hart
ishell= command event hart
command= event hart mem hart
zircon= controller command ishell hart elf mem trace event color common
zircon-wasm= controller command ishell hart elf mem trace event color common

define make_depen
$(eval $1: $($1))
endef
map = $(foreach a,$(2),$(call $(1),$(a)))
define make_prereqs
$(call map,make_depen,hart mem elf trace zircon zircon-wasm color event controller command ishell common)
endef
