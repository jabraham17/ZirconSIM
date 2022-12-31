-include $(ROOT_PROJECT_DIRECTORY)src/sources.mk
$(TARGET): $(OBJECTS) $(LIBRARIES_FILE_NAMES) $(TARGET_DEPENDS) Makefile
	$(AR) rcs $@ $(OBJECTS)
	$(RANLIB) $@
