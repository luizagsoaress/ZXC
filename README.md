## About

ZXC is a Unix shell written in C, originally based on Stephen Brennan's "Write a Shell in C" tutorial. The core loop, process management and builtins follow the same fork/exec pattern from the tutorial, but the project expanded from there: custom prompt with path truncation, readline integration, command history, multiple commands per line with `&&`, a GTK terminal interface with tab support, and AI queries via GROQ API. Colors and other settings are loaded at startup from a local config file.

The screenshots below show basic usage: navigating to Downloads with `cd`, creating a folder with `mkdir`, editing a file with `nano`, and reading it back with `cat`.

![image](./gitPhotos/github(test)_2.png)

![image](./gitPhotos/github(nano).png)

## Features

- Custom prompt with path truncation
- Readline integration with command history
- Multiple commands per line with `&&`
- Built-in commands: `cd`, `help`, `exit`, `ct`
- AI queries via GROQ API (`ct` command)
- GTK terminal interface with tab support
- Color and settings customization via local config file

## Technologies

| Technology | Usage |
|---|---|
| C | Core shell implementation |
| GTK3 | Terminal GUI interface |
| VTE | Terminal emulator widget |
| Readline | Input handling and history |
| libcurl | GROQ API requests |
| ncurses | Interactive API key menu |

## `ct` Command

The `ct` command integrates the GROQ AI into the shell, allowing the user to make requests directly from the terminal.

To use it:

- Type `ct`
- Select "Enter a new API key" if you don't have one saved yet
- Type your question

You can delete your saved key at any time by selecting "Delete saved API key" instead.

The API key is stored locally at `~/.api_key`.

If you use an invalid key, the response will be unreadable or return garbage, and an error alert may appear. Please use a correct one.

## Installation

**Dependencies:** gcc, make, libreadline, libcurl, gtk3, vte-2.91, ncurses

> In your terminal of choice, run the following commands:

```bash
 git clone https://github.com/luizagsoaress/ZXC.git
 cd ZXC
 make && make install
```

## Run

> After the installation, run with: 

```bash
 make run
```

## Config File

The config file is located at `~/.local/share/zxc/conf`.

## Binary Files

The binary files are located at `~/.local/bin`.


