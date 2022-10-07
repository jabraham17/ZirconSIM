
cpu=
elf= mem
mem= 
zircon= cpu elf mem

define make_depen
$(eval $1: $($1))
endef
map = $(foreach a,$(2),$(call $(1),$(a)))
define make_prereqs
$(call map,make_depen,cpu mem riscv-sim elf memimg)
endef
