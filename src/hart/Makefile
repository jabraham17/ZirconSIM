-include $(ROOT_PROJECT_DIRECTORY)options.mk
TARGET=$(LIB_DIRECTORY)libhart.a

TARGET_DEPENDS=$(GEN_DIRECTORY)generated-definition/

-include $(ROOT_PROJECT_DIRECTORY)src/library.mk

$(TARGET_DEPENDS): $(TARGET_DEPENDS)instructions.inc $(TARGET_DEPENDS)registers.inc $(TARGET_DEPENDS)syscalls.inc

$(TARGET_DEPENDS)instructions.inc: $(OUT_OF_TREE_INST)
	$(AT)mkdir -p $(dir $@)
	$(PYTHON3) $(SRC_PATH)generate-includes.py $^ >$@

$(TARGET_DEPENDS)registers.inc: $(OUT_OF_TREE_REG)
	$(AT)mkdir -p $(dir $@)
	$(PYTHON3) $(SRC_PATH)generate-includes.py $^ >$@

$(TARGET_DEPENDS)syscalls.inc: $(OUT_OF_TREE_SYSCALL)
	$(AT)mkdir -p $(dir $@)
	$(PYTHON3) $(SRC_PATH)generate-includes.py $^ >$@
