-include $(ROOT_PROJECT_DIRECTORY)options.mk
-include $(ROOT_PROJECT_DIRECTORY)src/dependencies.mk
LIBRARIES= $(zircon-wasm)
SYSTEM_LIBRARIES= 

override CXXFLAGS+=
override LDFLAGS+= --no-entry -sMODULARIZE=1 -sEXPORT_ES6=0 -sINVOKE_RUN=0 -sEXPORT_NAME=Mod -sFORCE_FILESYSTEM=1 -sEXPORTED_RUNTIME_METHODS=["FS"] -fwasm-exceptions -lembind --pre-js=zircon-prejs.js

ZIRCON_SITE=$(BUILD_DIRECTORY)zircon-site/
ZIRCON_SITE_SRC=$(ZIRCON_SITE)src/
TARGET=$(ZIRCON_SITE_SRC)zircon.js

-include $(ROOT_PROJECT_DIRECTORY)src/executable.mk

$(TARGET): $(ZIRCON_SITE) $(ZIRCON_SITE_SRC) install_site

$(ZIRCON_SITE) $(ZIRCON_SITE_SRC):
	$(AT)mkdir -p $@

SITE_FILES=$(wildcard site/*)
.PHONY: install_site
install_site:  $(SRC_PATH)$(SITE_FILES)
	$(AT)cp -r $^ $(ZIRCON_SITE)
