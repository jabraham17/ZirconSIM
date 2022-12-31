-include $(ROOT_PROJECT_DIRECTORY)options.mk
TARGET=$(LIB_DIRECTORY)libishell.a

REL_PATH=$(shell realpath --relative-to $(ROOT_PROJECT_DIRECTORY) .)/
SRC_PATH=$(ROOT_PROJECT_DIRECTORY)$(REL_PATH)
OBJ_PATH=$(OBJ_DIRECTORY)$(REL_PATH)parser/
TARGET_DEPENDS=$(OBJ_PATH)expr_parser_table.inc
override INCLUDE+= -I$(OBJ_PATH)
$(OBJ_PATH)expr_parser_table.inc:
	$(AT)python3 $(SRC_PATH)parser/build-table.py $(SRC_PATH)parser/ExprParsing.ods >$@

-include $(ROOT_PROJECT_DIRECTORY)src/library.mk

