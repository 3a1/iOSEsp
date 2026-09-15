TARGET := iphone:clang:latest:15.0
ARCHS := arm64 arm64e

include $(THEOS)/makefiles/common.mk

TOOL_NAME = backend
backend_FILES = $(wildcard src/*.c)
backend_CFLAGS = -Iinclude -include ../settings/settings.h -Wall -O3 -fomit-frame-pointer
backend_LDFLAGS = -L$(CURDIR) -ljailbreak
backend_CODESIGN_FLAGS = -Sent.plist

include $(THEOS_MAKE_PATH)/tool.mk

after-all::
	cp "$(THEOS_OBJ_DIR)/$(TOOL_NAME)" ./$(TOOL_NAME);
