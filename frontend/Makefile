TARGET := iphone:clang:latest:15.0
ARCHS := arm64 arm64e

DEBUG = 0
FINALPACKAGE = 1

include $(THEOS)/makefiles/common.mk

TWEAK_NAME = ScreenOverlay
ScreenOverlay_FILES = $(wildcard src/*.x) $(wildcard src/*.xm) $(wildcard src/*.m) $(wildcard src/*.c)
ScreenOverlay_CFLAGS = -Iinclude -include ../settings/settings.h -fobjc-arc -O3 -ffast-math -flto
ScreenOverlay_LDFLAGS = -flto

include $(THEOS_MAKE_PATH)/tweak.mk
