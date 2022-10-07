
MKFILE_PATH=$(abspath $(lastword $(MAKEFILE_LIST)))
MKFILE_DIR=$(dir $(MKFILE_PATH))
export ROOT_PROJECT_DIRECTORY=$(MKFILE_DIR)

TOPTARGETS=all cppcheck
SUBDIRS=src
.PHONY: $(TOPTARGETS)

-include $(ROOT_PROJECT_DIRECTORY)options.mk

all: $(BUILD_DIRECTORY) $(BIN_DIRECTORY) $(LIB_DIRECTORY) $(OBJ_DIRECTORY)
	@:

$(BUILD_DIRECTORY) $(BIN_DIRECTORY) $(LIB_DIRECTORY) $(OBJ_DIRECTORY):
	$(AT)mkdir -p $@

clean:
	$(RM) -r $(BUILD_DIRECTORY)

$(TOPTARGETS): $(SUBDIRS)

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	@printf "$(COLOR_RED)$(COLOR_BOLD)MAKE -C $@ $(MAKECMDGOALS)$(COLOR_RESET)\n"
	@$(MAKE) --no-print-directory -C $@ $(MAKECMDGOALS)

.PHONY: dump_paths
dump_paths:
	$(info CC=$(CC_1))
	$(info CXX=$(CXX_1))
	$(info LD=$(LD_1))
	$(info YACC=$(YACC_1))
	$(info LEX=$(LEX_1))
	$(info AR=$(AR_1))
	$(info RANLIB=$(RANLIB_1))

