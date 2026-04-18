#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include <math.h>
#include <curl/curl.h>
#include <termios.h>
#include "shell.h"

void escapesCleaner(char *src, char *destiny) {
	int i = 0, j = 0;
	if (src[0] == '$' && src[1] == '\'') {
		i = 2;
		int len = strlen(src);
		if (src[len - 1] == '\'')
			src[len - 1] = '\0';
	}
	while (src[i]) {
		if (src[i] == '\\' && src[i + 1]) {
			i++;
			if (src[i] == 'e' || src[i] == 'E') {
				destiny[j++] = '\033';
			} else if (src[i] == '0' && src[i+1] == '3' && src[i+2] == '3') {
				destiny[j++] = '\033';
				i += 2;
			} else if (src[i] == 'n') {
				destiny[j++] = '\n';
			} else if (src[i] == 't') {
				destiny[j++] = '\t';
			} else {
				destiny[j++] = '\\';
				destiny[j++] = src[i];
			}
		} else {
			destiny[j++] = src[i];
		}
		i++;
	}
	destiny[j] = '\0';
}

int main(int argc, char **argv) {
	struct termios t;
	tcgetattr(STDIN_FILENO, &t);
	t.c_lflag |= ISIG;
	t.c_cc[VINTR] = 3;
	tcsetattr(STDIN_FILENO, TCSANOW, &t);

	char *dirHome = getenv("HOME");
	char path[1024];
	snprintf(path, sizeof(path), "%s/.local/share/zxc/conf", dirHome);

	FILE *conf = fopen(path, "r+");
	char line[1024];

	if (conf) {
		while (fgets(line, sizeof(line), conf)) {
			line[strcspn(line, "\n")] = '\0';
			if (line[0] == '\0' || line[0] == '#') continue;
			char *x = strchr(line, '=');
			if (!x) continue;
			*x = '\0';
			char processed[1024];
			escapesCleaner(x + 1, processed);
			setenv(line, processed, 1);
		}
		fclose(conf);
	}

	zxc_loop();
	return 0;
}
