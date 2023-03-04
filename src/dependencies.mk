
# hart must be build first so that dependencies for out of tree *.inc is satisfied
hart= 
elf= hart
mem= hart
event= hart
ishell= hart
command= hart
common= hart
trace= hart
color= hart
zircon= hart command ishell elf mem trace event color common
zircon-wasm= hart command ishell elf mem trace event color common

define make_depen
$(eval $1: $($1))
endef
map = $(foreach a,$(2),$(call $(1),$(a)))
define make_prereqs
$(call map,make_depen,hart mem elf trace zircon zircon-wasm color event command ishell common)
endef
