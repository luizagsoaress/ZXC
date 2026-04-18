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
#include <signal.h>
#include <stdbool.h>
#include <curl/curl.h>
#include <ncurses.h>

#ifndef SHELL_H
#define SHELL_H

int zxc_cd(char **args);
int zxc_help(char **args);
int zxc_exit(char **args);
int zxc_ct(char **args);
char* ApiCall(char *args, char* apiKey);
char **zxc_split_line(char *line);
int zxc_execute(char **args);
void cancelSignal(int sig);
void freeMemory(void **ptrs, int count);

struct Memory {
    char *data;
    size_t size;
};

char *builtin_str[] = {
  	"cd",
  	"help",
  	"exit",
  	"ct"
};

int (*builtin_func[]) (char **) = {
  	&zxc_cd,
  	&zxc_help,
  	&zxc_exit,
  	&zxc_ct
};

int zxc_num_builtins() {
  	return sizeof(builtin_str) / sizeof(char *);
}

int zxc_cd(char **args) {
  	char* dir = NULL;
      	
  	if (args[1] == NULL) {
    	dir = getenv("HOME");
    	if(dir == NULL) fprintf(stderr, "cd: expected argument to \"cd\"\n");
  	} else dir = args[1];
	
  	if (chdir(dir) != 0) perror("cd");
  	
  	return 1;
}

int zxc_help(char **args) {
  	int i;
  	printf("Luiza G. Soares zxc\n");
  	printf("Type the program name and arguments, then press enter.\n");
  	printf("The following commands are built into the shell:\n");
	
  	printf("\n");
  	for (i = 0; i < zxc_num_builtins(); i++) {
    	printf("* %s\n", builtin_str[i]);
  	}
  	printf("\n");
	
  	printf("Use the man command for information on other programs.\n");
  	return 1;
}

int zxc_exit(char **args){
  	return 0;
}

char* clearString(char *s) {
    int j = 0;
    for (int i = 0; s[i]; i++) {
      if (s[i] == '\\' && s[i+1] == 'n') {
        s[j++] = '\n';
        i++;
        continue;
      } if (s[i] == '\\' && s[i+1] == 'r') {
        s[j++] = '\r';
        i++;
        continue;
      } if (s[i] == '\\' && s[i+1] == 't') {
        s[j++] = '\t';
        i++;
        continue;
      } if (s[i] == '\\' && s[i+1] == '"') {
        s[j++] = '"';
        i++;
        continue;
      } if (s[i] == '"' && s[i+1] == '\\') {
        s[j++] = '"';
        i++;
        continue;
      } if (s[i] == '*' && s[i+1] == '*') {
        s[j++] = '-';
        i++;
        continue;
      }
    
      s[j++] = s[i];
    }
    s[j] = '\0';
    return s;
}

void initialize() {
  	initscr();
  	cbreak();
  	noecho();
  	start_color();
  	init_pair(1, COLOR_YELLOW, COLOR_BLACK);
  	keypad(stdscr, TRUE);
}

int menuChoice = 0;

void createMenu() {
  	initialize();
	
  	int opt = 0;
  	const char *opts[] = {"- Use saved API key.", "- Enter a new API key.", "*** Delete saved API key." };
  	int n = 3;
	
  	while (1) {
    	clear();
    	
    	for (int i = 0; i < n; i++) {
      	int x = (COLS - strlen(opts[i])) / 2;
      	int y = (LINES - n) / 2;
      	if (i == opt) attron(A_REVERSE);
      	attron(COLOR_PAIR(1));
      	mvprintw(y + i, x, "%s", opts[i]);
      	if (i == opt) attroff(A_REVERSE);
    	}
    	int c = getch();
    	if (c == KEY_UP) {
      	opt = (opt + n - 1) % n;
    	}
    	else if (c == KEY_DOWN) opt = (opt + 1) % n;
    	else if (c == '\n' || c == 27) {
      	endwin();
      	if(opt == 0) {
        	menuChoice = 1;
        	break;
      	} else if(opt == 1) {
        	menuChoice = 2;
        	break;
      	} else if(opt == 2) {
        	menuChoice = 3;
        	break;
      	}
    	}
    	attroff(COLOR_PAIR(1));
  	}
}

int zxc_ct(char **args) {
  	char* cwdColorG = getenv("DIR_COLOR");
  	char* resetG = getenv("RESET_COLOR");
  	char* redG= getenv("RED_COLOR");
  	char* symbG= getenv("SIMB_COLOR");
  	char* apiKey = (char*)malloc(1024 * sizeof(char));
  	char* question = (char*)malloc(11000 * sizeof(char));
  	char* questionString = (char*)malloc(11000 * sizeof(char));
	
  	if(!apiKey) return 1;
  	if(!question) return 1;
  	if(!questionString) return 1;
	
  	char line[1024]; 
  	apiKey[0] = '\0';
  	question[0] = '\0';
  	questionString[0] = '\0';
  	
  	char* dirHome = getenv("HOME");
  	char filePath[1024];
  	snprintf(filePath, sizeof(filePath), "%s/.api_key", dirHome);
  	FILE *apiKeyConf = fopen(filePath, "a+");
	
  	if (!apiKeyConf) {
    	perror("error opening file");
    	void *ctPtrs[] = {apiKey, question, questionString};
		freeMemory(ctPtrs, 3);
    	return 1;
  	}
	
  	fseek(apiKeyConf, 0, SEEK_END);
  	long size = ftell(apiKeyConf);
  	
  	rewind(apiKeyConf);
  	
  	if(args[1] == NULL) {
    	printf("\n%s* The AI used to answer questions is GROQ (Meta AI). To use it, first create an API key on the official website. Errors may occur — if so, just try again or rephrase your question.\n", symbG);
    	printf("\n%s* Don't worry, your API key will only be stored locally in a file named '.api_key' in the ${HOME} directory, which you can delete at any time.\n", symbG);
    	createMenu();
    	if(size <= 1 && menuChoice == 1) {
      		printf("\n%s- You don't have an API key registered yet. Please add one first.\n\n", resetG);
      		void *ctPtrs[] = {apiKey, question, questionString};
			freeMemory(ctPtrs, 3);
      		return 1;
    	} else if(size > 1 && menuChoice == 1) {
      		while(fgets(line, sizeof(line), apiKeyConf)) {}
      		line[strcspn(line, "\n")] = '\0';
		
      		printf("\n%sType your question ( max 10,000 characters ) > %s", cwdColorG, resetG);
      		fgets(question, 10000, stdin);
      		question[strcspn(question, "\n")] = '\0'; 
		
      		char question2[10000];
      		int i = 0;
      		int j = 0;
      		while (question[i] != '\0') {
        		if (question[i] == '\\') {
          		question2[j++] = '\\';
          		question2[j++] = '\\';
        		} else {
          		question2[j++] = question[i];
        		}
        		i++;
      		}
      		question2[j] = '\0';
		
      		strcat(questionString, "{\"model\":\"llama-3.1-8b-instant\",\"messages\":[{\"role\":\"user\",\"content\":\"");
      		strcat(questionString, question2);
      		strcat(questionString, "\"}]}");
      		
      		char* result = ApiCall(questionString, line);
      		if(!result) return 1;
		
      		char* cleanResult = clearString(result);
      		printf("\n%sAnswer:\n\n%s%s\n\n", cwdColorG, resetG, cleanResult);
      		void *ctPtrs[] = {apiKey, question, questionString, result};
			freeMemory(ctPtrs, 4);
      		return 1;
    	} else if((size <= 0 && menuChoice == 2) || (size > 0 && menuChoice == 2)) {
      		apiKeyConf = fopen(filePath, "w");
		
      		printf("\n%sType your API key >%s ", cwdColorG, resetG);
      		fgets(apiKey, 1024, stdin);
      		apiKey[strcspn(apiKey, "\n")] = '\0';
		
      		fprintf(apiKeyConf, "%s\n", apiKey);
      		fclose(apiKeyConf);
      		
      		size = 0;
      		snprintf(filePath, sizeof(filePath), "%s/.api_key", dirHome);
      		apiKeyConf = fopen(filePath, "a+");
	
      		if (!apiKeyConf) {
        		perror("error opening file");
        		void *ctPtrs[] = {apiKey, question, questionString};
				freeMemory(ctPtrs, 3);
        		return 1;
      		}
	
      		fseek(apiKeyConf, 0, SEEK_END);
      		size = ftell(apiKeyConf);
		
      		rewind(apiKeyConf);
		
      		if(size <= 1) {
        		printf("\n%s- Invalid API key format. Please try again.\n\n", resetG);
        		void *ctPtrs[] = {apiKey, question, questionString};
				freeMemory(ctPtrs, 3);
        		return 1;
      		}
		
      		printf("\n%sType your question ( max 10,000 characters ) > %s", cwdColorG, resetG);
      		fgets(question, 10000, stdin);
      		question[strcspn(question, "\n")] = '\0'; 
		
      		char question2[10000];
      		int i = 0;
      		int j = 0;
      		while (question[i] != '\0') {
          		if (question[i] == '\\') {
              		question2[j++] = '\\';
              		question2[j++] = '\\';
          		} else {
              		question2[j++] = question[i];
          		}
          		i++;
      		}
      		question2[j] = '\0';
	
      		strcat(questionString, "{\"model\":\"llama-3.1-8b-instant\",\"messages\":[{\"role\":\"user\",\"content\":\"");
      		strcat(questionString, question2);
      		strcat(questionString, "\"}]}");
		
      		char* result = ApiCall(questionString, apiKey);
      		if(!result) return 1;
      		char* cleanResult = clearString(result);
      		printf("\n%sAnswer:\n\n%s%s\n\n", cwdColorG, resetG, cleanResult);
      		free(result);
    		} else if(size >= 1 && menuChoice == 3) {
      		char *line = (char*)malloc(1024 * sizeof(char));
      		char **args;
      		int status;
		
      		strcat(line, "rm ");
      		strcat(line, filePath);
		
      		args = zxc_split_line(line);
      		fflush(stdout);
      		status = zxc_execute(args);
		
      		if(status >= 1) {
        		printf("\n%s- Key deleted successfully.\n\n", resetG);
        		void *ctPtrs[] = {apiKey, question, questionString};
				freeMemory(ctPtrs, 3);
        		return 1;
      		} else {
        		printf("\n%s- Failed to delete key.\n\n", resetG);
        		void *ctPtrs[] = {apiKey, question, questionString};
				freeMemory(ctPtrs, 3);
        		return 1;
      		}
    } else if(size <= 0 && menuChoice == 3) {
      	printf("\n%s- You don't have an API key registered yet. Please add one first.\n\n", resetG);
      	void *ctPtrs[] = {apiKey, question, questionString};
		freeMemory(ctPtrs, 3);
      	return 1;
    }
  	} else if(args[1] != NULL) fprintf(stderr, "ct: API key required.\n");
  	
  	void *ctPtrs[] = {apiKey, question, questionString};
	freeMemory(ctPtrs, 3);
	return 1;
}

static size_t callback(void *contents, size_t size, size_t nmemb, void *userp) {
  	size_t total = size * nmemb;
  	struct Memory *mem = (struct Memory *)userp;
  	mem->data = realloc(mem->data, mem->size + total + 1);
  	memcpy(&(mem->data[mem->size]), contents, total);
  	mem->size += total;
  	mem->data[mem->size] = '\0';
  	return total;
}

char* getResponse(char *json) {
  	char *p = strstr(json, "\"content\":\"");
  	if (!p) {
    	return NULL;
  	}
  	p += strlen("\"content\":\"");
  	char *start = p;
  	char *end = p;
	
  	while (*end) {
    	if (*end == '"' && *(end - 1) != '\\') {
      	break;
    	}
    	end++;
  	}
	
  	int size = end - start;
  	char *output = (char*)malloc(size + 1 * sizeof(char));
  	if(!output) return NULL;
  	strncpy(output, start, size);
  	output[size] = '\0';
  	return output;
}

void freeMemory(void **ptrs, int count) {
  	for (int i = 0; i < count; i++) {
    	if (ptrs[i]) {
      	free(ptrs[i]);
      	ptrs[i] = NULL;
    	}
  	}
}

char* ApiCall(char *args, char* apiKey) {
  	CURL *curl = curl_easy_init();
	
  	char* resetG = getenv("RESET_COLOR");
	
  	struct curl_slist *headers = NULL;
	
  	char* headerApiKey = (char*)malloc(1024 * sizeof(char));
  	char* bodyString = (char*)malloc(1024 * sizeof(char));
	
  	if(!headerApiKey) return NULL;
  	if(!bodyString) return NULL;
  	
  	headerApiKey[0] = '\0';
  	bodyString[0] = '\0';
	
  	strcat(headerApiKey, "Authorization: Bearer ");
  	strcat(headerApiKey, apiKey);
	
  	strcat(bodyString, args);
	
  	headers = curl_slist_append(headers, "Content-Type: application/json");
  	headers = curl_slist_append(headers, headerApiKey); 
	
  	curl_easy_setopt(curl, CURLOPT_URL, "https://api.groq.com/openai/v1/chat/completions");
  	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyString);
	
  	struct Memory response = {0};
  	response.data = malloc(1);
  	response.size = 0;
	
  	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
  	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
	
  	CURLcode res = curl_easy_perform(curl);
	
  	if (res != CURLE_OK) {
    	return NULL;
  	}
	
  	char* p = getResponse(response.data);
	
  	if(!p) {
    	printf("\n%s- Invalid response. Please check your API key or rephrase the question and try again. \n\n", resetG);
    	return NULL;
  	}
	
  	size_t responseSize = strlen(response.data);
  	char* result = (char*)malloc(responseSize * sizeof(char));
  	if(!result) return NULL;
  	if (p) {
      	strcpy(result, p);
  	}
	
  	curl_easy_cleanup(curl);
  	void *apiPtrs[] = {headerApiKey, bodyString, response.data, p};
	freeMemory(apiPtrs, 4);
	return result;
}
 
pid_t zxc_launch(char **args) {
  	pid_t pid;
  	int status;
	
  	char **finalArgs = args;
  	char **aliasColor = NULL;
  	int x = 0;
	
  	if(strcmp(args[0], "ls") == 0) {
    	while(args[x] != NULL) x++;
    	aliasColor = (char**)malloc((x + 2) * sizeof(char*));
    	for(int i = 0; i < x; i++) {
      	aliasColor[i] = args[i];
    	}
    	aliasColor[x] = "--color=auto";
    	aliasColor[x + 1] = NULL;
    	finalArgs = aliasColor;
  	}
	
  	signal(SIGINT, SIG_IGN);
	
  	pid = fork();
  	if (pid == 0) {
    	signal(SIGINT, SIG_DFL);
    	if (execvp(finalArgs[0], finalArgs) == -1) {
      	perror("zxc");
    	}
    	exit(EXIT_FAILURE);
  	} else if (pid < 0) {
    	perror("zxc");
  	} else {
    	do {
      	waitpid(pid, &status, WUNTRACED);
    	} while (!WIFEXITED(status) && !WIFSIGNALED(status));
  	}
	
  	signal(SIGINT, cancelSignal);
	
	void *apiPtrs[] = {aliasColor};
	freeMemory(apiPtrs, 1);
  	return pid;
}


int zxc_execute(char **args) {
  	if (args[0] == NULL) {
    	return 1;
  	}
  	
  	int count = 0;
  	while(args[count] != NULL) {
    	if(strcmp(args[count], "&&") == 0) {
      	args[count] = NULL;
      	int status = zxc_execute(args);      
      	if (status == 1) {
        	return zxc_execute(&args[count+1]);    
      	}
      	return status;
    	}
    	count++;
  	}
	
  	for (int i = 0; i < zxc_num_builtins(); i++) {
    	if (strcmp(args[0], builtin_str[i]) == 0) {
      	return (*builtin_func[i])(args);
    	}
  	}
	
  	return zxc_launch(args);
}

#define zxc_TOK_BUFSIZE 64
#define zxc_TOK_DELIM " \t\r\n\a"

char **zxc_split_line(char *line) {
  	int bufsize = zxc_TOK_BUFSIZE;
  	int position = 0;
  	char **tokens = (char**)malloc(bufsize * sizeof(char*));
  	if(!tokens) return NULL;
  	char *token;
  	char **tokensBackup;
	
  	if (!tokens) {
    	fprintf(stderr, "zxc: allocation error\n");
    	exit(EXIT_FAILURE);
  	}
	
  	token = strtok(line, zxc_TOK_DELIM);
  	while (token != NULL) {
    	tokens[position] = token;
    	position++;
	
    	if (position >= bufsize) {
      	bufsize += zxc_TOK_BUFSIZE;
      	tokensBackup = tokens;
      	tokens = realloc(tokens, bufsize * sizeof(char*));
      	if (!tokens) {
		  	free(tokensBackup);
        	fprintf(stderr, "zxc: allocation error\n");
        	exit(EXIT_FAILURE);
      	}
    	}
	
    	token = strtok(NULL, zxc_TOK_DELIM);
  	}
  	tokens[position] = NULL;
  	return tokens;
}

void cancelSignal(int sig) {
  	printf("\n");
  	rl_on_new_line();
  	rl_replace_line("", 0);
  	rl_redisplay();
}

void zxc_loop(void) {
  	rl_bind_key('\x03', rl_named_function("abort"));
  	rl_catch_signals = 0;
  	signal(SIGINT, cancelSignal); 
  	
  	char pathHistory[256];
  	snprintf(pathHistory, sizeof(pathHistory), "%s/.zxc_history", getenv("HOME"));
  	read_history(pathHistory);
	
  	char *line;
  	char **args;
  	int status;
	
  	do {
    	char cwdBuf[1024];
    	char *cwd = getcwd(cwdBuf, sizeof(cwdBuf));
    	char *dirHome = getenv("HOME");
    	char result[1024];
    	int bufsize = 1024;
    	int position = 0;
    	char **tokens = (char**)malloc(bufsize * sizeof(char*));
    	char **cwdTotal = (char**)malloc(bufsize * sizeof(char*));
    	char **tokenSplit = (char**)malloc(bufsize * sizeof(char*));
    	char *token;
    	char cwdSplit[64][64]; 
	
    	if(!tokens) return;
    	if(!cwdTotal) return;
    	if(!tokenSplit) return;
	
    	int i = 0;
    	int j = 0;
    	int count = 0;
    	int tokenCount = 1;
        	
    	if (strcmp(cwd, dirHome) == 0) {
      	strcpy(result, "~");
    	} else if (strncmp(cwd, dirHome,  strlen(dirHome)) == 0) {
      	token = strtok(cwd, "/");
      	while (token != NULL) {
        	tokens[position++] = token;
        	token = strtok(NULL, "/");
      	}
      	tokens[position] = NULL;
	
      	if (position >= 6) {
        	while (tokens[i] != NULL) {
          	if(i >= position - 1) {
            	j = ceil((double)tokenCount / 2);
            	for(int k = 0; k < j; k++) {
              	tokenSplit[k] = tokens[k];
              	if(k <= 2) {
                	strncpy(cwdSplit[k], tokenSplit[k], (floor(strlen(tokenSplit[k]) / 2)));
                	cwdSplit[k][strlen(tokenSplit[k]) / 2] = '\0';
              	} else {
                	strncpy(cwdSplit[k], tokenSplit[k], 1);
                	cwdSplit[k][1] = '\0';
              	}
              	cwdTotal[k] = cwdSplit[k];
              	count++;
            	}
            	for(int x = count; x < position; x++) {
              	cwdTotal[x] = tokens[x];
            	}
          	} 
          	tokenCount++;
          	i++;
        	}
        	for (int idx = 0; idx < position; idx++) {
          	if (cwdTotal[idx] == NULL) {
            	cwdTotal[idx] = tokens[idx]; 
          	}
        	}
        	snprintf(result, sizeof(result), "~");
        	for (int i = 2; i < position; i++) {
          	strcat(result, "/");
          	strcat(result, cwdTotal[i]);
        	}
        	} else if(position <= 5)  {
          	char *cwdOriginal = getcwd(cwdBuf, sizeof(cwdBuf));
          	snprintf(result, sizeof(result), "~%s", cwdOriginal + strlen(dirHome));
        	} 
    	} else {
      	strcpy(result, cwd);
    	}
        	
    	char* cwdColorG = getenv("CWD_COLOR");
    	char* resetG = getenv("RESET_COLOR");
    	char* symbolG = getenv("SIFRA_COLOR");
    	char* symbolChar = "〉";
	
    	char prompt[256];
    	snprintf(prompt, sizeof(prompt), "%s%s%s %s %s", cwdColorG, result, symbolG, symbolChar, resetG);
	
    	line = readline(prompt);  
    	if (line && *line) {
      	add_history(line);
    	}
	
    	args = zxc_split_line(line);
    	fflush(stdout);
	
    	status = zxc_execute(args);
        	
    	printf("%s", resetG);
    	fflush(stdout);
	
    	write_history(pathHistory);
    	
    	void *loopPtrs[] = {line, args, tokens, cwdTotal, tokenSplit};
		freeMemory(loopPtrs, 5);
  	} while (status);
}

#endif
