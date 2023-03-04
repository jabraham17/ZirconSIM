-include $(ROOT_PROJECT_DIRECTORY)options.mk
TARGET=$(LIB_DIRECTORY)libhart.a

TARGET_DEPENDS=$(GEN_DIRECTORY)generated-definition/

-include $(ROOT_PROJECT_DIRECTORY)src/library.mk

$(TARGET_DEPENDS):
	@mkdir -p $@
	$(AT)python3 $(SRC_PATH)generate-includes.py $(OUT_OF_TREE_INST) >$@instructions.inc
	$(AT)python3 $(SRC_PATH)generate-includes.py $(OUT_OF_TREE_REG) >$@registers.inc
	$(AT)python3 $(SRC_PATH)generate-includes.py $(OUT_OF_TREE_SYSCALL) >$@syscalls.inc
