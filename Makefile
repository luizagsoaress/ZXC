CC = gcc
LIBS = -lreadline -lcurl -lncurses -lm
GUI_FLAGS = `pkg-config --cflags --libs gtk+-3.0 vte-2.91`

install:
	$(CC) -o shell/main shell/main.c $(LIBS)
	$(CC) interface/interface.c -o interface/interface $(GUI_FLAGS) $(LIBS)
	mkdir -p ~/.local/bin
	mkdir -p ~/.local/share/zxc/images
	cp shell/.env.exemplo ~/.local/share/zxc/conf
	cp shell/main ~/.local/bin/main
	cp interface/interface ~/.local/bin/interface
	cp interface/images/* ~/.local/share/zxc/images/
	chmod +x ~/.local/bin/interface
	chmod +x ~/.local/bin/main

run: 
	~/.local/bin/interface

uninstall:
	rm -f ~/.local/bin/main
	rm -f ~/.local/bin/interface
	rm -rf ~/.local/share/zxc
