CC = gcc
LIBS = -lreadline -lcurl -lncurses -lm
GUI_FLAGS = `pkg-config --cflags --libs gtk+-3.0 vte-2.91`

all: shell gui

shell:
	$(CC) shell/main.c -o shell/main $(LIBS)

gui:
	$(CC) interface/interface.c -o interface/interface $(GUI_FLAGS) $(LIBS)

install:
	mkdir -p ~/.local/bin
	mkdir -p ~/.local/share/zxc/images
	cp shell/.env.exemplo ~/.local/share/zxc/conf
	cp shell/main ~/.local/bin/main
	cp interface/interface ~/.local/bin/interface
	cp interface/images/* ~/.local/share/zxc/images/
	chmod +x ~/.local/bin/interface
	chmod +x ~/.local/bin/main	

uninstall:
	rm -f ~/.local/bin/main
	rm -f ~/.local/bin/interface
	rm -rf ~/.local/share/zxc
