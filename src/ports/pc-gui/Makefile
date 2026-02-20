CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Werror
TARGET  = snake-gui

CORE_DIR    = ../../core
SRC_DIR     = .
RAYLIB_DIR  = /opt/homebrew

CFLAGS  += -I$(RAYLIB_DIR)/include
LDFLAGS  = -L$(RAYLIB_DIR)/lib -lraylib
LDFLAGS += -framework IOKit -framework Cocoa -framework OpenGL

CORE_SRC = $(CORE_DIR)/game.c $(CORE_DIR)/snake.c $(CORE_DIR)/lcd.c $(CORE_DIR)/theme.c $(CORE_DIR)/renderer.c $(CORE_DIR)/ui.c
PORT_SRC = $(SRC_DIR)/main.c $(SRC_DIR)/platform_raylib.c $(SRC_DIR)/audio.c

SRCS = $(CORE_SRC) $(PORT_SRC)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -I$(CORE_DIR)/../.. -o $@ $(SRCS) $(LDFLAGS)

ICON_PNG    = ../../../public/icon.png
ICONSET_DIR = Snake.iconset
ICNS_FILE   = Snake.icns
DIST_DIR    = ../../../downloads
APP_NAME    = $(DIST_DIR)/Snake.app

bundle: $(TARGET)
	rm -rf $(ICONSET_DIR) $(APP_NAME)
	mkdir -p $(DIST_DIR)
	mkdir -p $(ICONSET_DIR)
	sips -z 16 16     $(ICON_PNG) --out $(ICONSET_DIR)/icon_16x16.png
	sips -z 32 32     $(ICON_PNG) --out $(ICONSET_DIR)/icon_16x16@2x.png
	sips -z 32 32     $(ICON_PNG) --out $(ICONSET_DIR)/icon_32x32.png
	sips -z 64 64     $(ICON_PNG) --out $(ICONSET_DIR)/icon_32x32@2x.png
	sips -z 128 128   $(ICON_PNG) --out $(ICONSET_DIR)/icon_128x128.png
	sips -z 256 256   $(ICON_PNG) --out $(ICONSET_DIR)/icon_128x128@2x.png
	sips -z 256 256   $(ICON_PNG) --out $(ICONSET_DIR)/icon_256x256.png
	sips -z 512 512   $(ICON_PNG) --out $(ICONSET_DIR)/icon_256x256@2x.png
	sips -z 512 512   $(ICON_PNG) --out $(ICONSET_DIR)/icon_512x512.png
	sips -z 1024 1024 $(ICON_PNG) --out $(ICONSET_DIR)/icon_512x512@2x.png
	iconutil -c icns $(ICONSET_DIR) -o $(ICNS_FILE)
	rm -rf $(ICONSET_DIR)
	mkdir -p $(APP_NAME)/Contents/MacOS
	mkdir -p $(APP_NAME)/Contents/Resources
	cp $(TARGET) $(APP_NAME)/Contents/MacOS/
	cp $(ICNS_FILE) $(APP_NAME)/Contents/Resources/
	rm -f $(ICNS_FILE)
	printf '<?xml version="1.0" encoding="UTF-8"?>\n\
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n\
<plist version="1.0">\n\
<dict>\n\
  <key>CFBundleExecutable</key>\n\
  <string>snake-gui</string>\n\
  <key>CFBundleIconFile</key>\n\
  <string>Snake</string>\n\
  <key>CFBundleIdentifier</key>\n\
  <string>com.retro.snake</string>\n\
  <key>CFBundleName</key>\n\
  <string>Snake</string>\n\
  <key>CFBundlePackageType</key>\n\
  <string>APPL</string>\n\
  <key>CFBundleVersion</key>\n\
  <string>1.0</string>\n\
  <key>NSHighResolutionCapable</key>\n\
  <true/>\n\
</dict>\n\
</plist>\n' > $(APP_NAME)/Contents/Info.plist
	cd $(DIST_DIR) && zip -r Snake.app.zip Snake.app

clean:
	rm -rf $(TARGET) $(APP_NAME) $(ICONSET_DIR) $(ICNS_FILE)

.PHONY: clean bundle
