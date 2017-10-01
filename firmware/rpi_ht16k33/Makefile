OVERLAY_DIR := /boot/overlays/
TARGET := offbutt
DTC := dtc

$(TARGET): $(TARGET).dtbo

%.dtbo: %.dts
	$(DTC) -@ -I dts -O dtb -o $@ $<

install: $(TARGET).dtbo
	# Only needed for custom shutdown button
	#sudo install -m 0755 -o root -g root -t $(OVERLAY_DIR) $<
	#sudo install -m 0755 -o root -g root -t /usr/local/bin/ turnoff
	#sudo install -m 0644 -o root -g root -t /etc/triggerhappy/triggers.d/ offbutt-trigger.conf
	#sudo install -m 0644 -o root -g root -t /etc/udev/rules.d/ 01-gpiokeys.rules
	sudo install -m 0755 -o root -g root -t /usr/local/bin/ clock.py
	sudo install -m 0755 -o root -g root -t /lib/systemd/system/ rpiclock.service

uninstall:
	# Only needed for custom shutdown button
	#sudo $(RM) $(OVERLAY_DIR)/$(TARGET)
	#sudo $(RM) /usr/local/bin/turnoff
	#sudo $(RM) /etc/triggerhappy/triggers.d/offbutt-trigger.conf
	#sudo $(RM) /etc/udev/rules.d/01-gpiokeys.rules
	sudo $(RM) /usr/local/bin/clock.py
	sudo $(RM) /lib/systemd/system/rpiclock.service

clean:
	$(RM) *.dtbo

.PHONY: clean install
