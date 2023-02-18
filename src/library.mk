-include $(ROOT_PROJECT_DIRECTORY)src/sources.mk
$(TARGET): $(TARGET_DEPENDS) $(OBJECTS) $(LIBRARIES_FILE_NAMES) Makefile
	$(AR) rcs $@ $(OBJECTS)
	$(RANLIB) $@
