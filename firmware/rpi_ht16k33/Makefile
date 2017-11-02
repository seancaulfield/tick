OVERLAY_DIR := /boot/overlays/
#TARGET := offbutt
TARGET := gpio-shutdown-overlay
DTC := dtc

$(TARGET): $(TARGET).dtbo

%.dtbo: %.dts
	$(DTC) -@ -I dts -O dtb -o $@ $<

install: $(TARGET).dtbo
	sudo install -m 0755 -o root -g root -t $(OVERLAY_DIR) $<
	sudo install -m 0755 -o root -g root -t /usr/local/bin/ turnoff
	sudo install -m 0755 -o root -g root -t /usr/local/bin/ clock.py
	sudo install -m 0755 -o root -g root -t /etc/triggerhappy/triggers.d/ offbutt-trigger.conf
	sudo install -m 0755 -o root -g root -t /etc/udev/rules.d/ 01-gpiokeys.rules

uninstall:
	sudo $(RM) $(OVERLAY_DIR)/$(TARGET)
	sudo $(RM) /usr/local/bin/turnoff
	sudo $(RM) /usr/local/bin/clock.py
	sudo $(RM) /etc/triggerhappy/triggers.d/offbutt-trigger.conf
	sudo $(RM) /etc/udev/rules.d/01-gpiokeys.rules

clean:
	$(RM) *.dtbo

.PHONY: clean install
