-include $(ROOT_PROJECT_DIRECTORY)options.mk
TARGET=$(LIB_DIRECTORY)libishell.a

TARGET_DEPENDS=$(OBJ_PATH)parser/expr_parser_table.inc

-include $(ROOT_PROJECT_DIRECTORY)src/library.mk

$(TARGET_DEPENDS):
	@mkdir -p $(OBJ_PATH)parser/
	$(AT)python3 $(SRC_PATH)parser/build-table.py $(SRC_PATH)parser/ExprParsing.ods >$@
