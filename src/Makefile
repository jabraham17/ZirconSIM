-include $(ROOT_PROJECT_DIRECTORY)options.mk

TOPTARGETS=all clean cppcheck
# grep all subdirs of src, extract directory names, sort them, and extract just the basename
SUBDIRS= $(patsubst $(ROOT_PROJECT_DIRECTORY)src/%/,%,$(sort $(dir $(wildcard $(ROOT_PROJECT_DIRECTORY)src/*/*))))

ifeq ($(WASM), 1)
# remove native only
SUBDIRS:=$(filter-out zircon,$(SUBDIRS))
else
# remove wasm only
SUBDIRS:=$(filter-out zircon-wasm,$(SUBDIRS))
endif

.DEFAULT_GOAL: all

.PHONY: $(TOPTARGETS)
$(TOPTARGETS): $(SUBDIRS)

-include $(ROOT_PROJECT_DIRECTORY)src/dependencies.mk
$(call make_prereqs)

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	@printf "$(COLOR_RED)$(COLOR_BOLD)MAKE -C $@ $(MAKECMDGOALS)$(COLOR_RESET)\n"
	@$(MAKE) --no-print-directory -C $@ $(MAKECMDGOALS)
