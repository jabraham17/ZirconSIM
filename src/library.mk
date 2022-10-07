-include $(ROOT_PROJECT_DIRECTORY)src/sources.mk
$(TARGET): $(OBJECTS) $(LIBRARIES_FILE_NAMES) Makefile
	$(AR) rcs $@ $(OBJECTS)
	$(RANLIB) $@
