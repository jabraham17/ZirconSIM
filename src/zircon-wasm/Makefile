-include $(ROOT_PROJECT_DIRECTORY)options.mk
-include $(ROOT_PROJECT_DIRECTORY)src/dependencies.mk
LIBRARIES= $(zircon-wasm)
SYSTEM_LIBRARIES= 
override LDFLAGS+= --no-entry -sMODULARIZE=1 -sEXPORT_ES6=0 -sINVOKE_RUN=0 -sEXPORT_NAME=Mod -sFORCE_FILESYSTEM=1 -sEXPORTED_RUNTIME_METHODS=["FS"] -lembind --pre-js=zircon-prejs.js
TARGET=$(BIN_DIRECTORY)zircon.js
-include $(ROOT_PROJECT_DIRECTORY)src/executable.mk

$(TARGET): install

.PHONY: install
install: $(SRC_PATH)zircon-main.js $(SRC_PATH)zircon-worker.js $(SRC_PATH)index.html
	$(AT)cp $^ $(BIN_DIRECTORY)
