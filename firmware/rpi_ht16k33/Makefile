OVERLAY_DIR := /boot/overlays/
TARGET := offbutt
DTC := dtc

$(TARGET): $(TARGET).dtbo

%.dtbo: %.dts
	$(DTC) -@ -I dts -O dtb -o $@ $<

install: $(TARGET).dtbo
	sudo install -m 0755 -o root -g root -t $(OVERLAY_DIR) $<

uninstall:
	sudo $(RM) $(OVERLAY_DIR)/$(TARGET)

clean:
	$(RM) *.dtbo

.PHONY: clean install
