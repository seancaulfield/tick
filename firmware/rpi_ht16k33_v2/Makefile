install:
	scp clock.py clock.service chronos0:
	ssh chronos0 sudo install -m 0755 -o root -g root -t /usr/local/bin/ clock.py
	ssh chronos0 sudo install -m 0644 -o root -g root -t /etc/systemd/system/ clock.service
	ssh chronos0 sudo systemctl enable clock
	ssh chronos0 sudo systemctl start clock
	ssh chronos0 rm clock.py clock.service

uninstall:
	ssh chronos0 sudo systemctl stop clock
	ssh chronos0 sudo systemctl disable clock
	ssh chronos0 sudo rm -f /usr/local/bin/clock.py

.PHONY: install
