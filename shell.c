#define TRUE 1
#define EMPTY 0x00
#define FALSE 0
#define DIRS_ENTRY_LENGTH 16
#define ENTRY_LENGTH 32
#define MAX_NAME 15

int countArgs(char* str);
int strcmp(char* str1, char* str2);
void cd(char* parentIndex);
void clear(char* buffer, int length);
int lookSlash(int startIndex, char* path);
void printString(char* string, int length);
void writeInt(int x);
void pause(int pid);
void resume(int pid);
void kill(int pid);
void parallel(int pid);

int main() {
	char input[50];
	char argc;
	char str[16];
	char ch[16];
	char and[16];
	char curdir;
	int success;
	int i, j, iterator;

	interrupt(0x21, 0x21, &curdir, 0, 0);
	
	while(1) {
		char **argv;

		// Menunggu input
		interrupt(0x21, 0x00, "$ ", 0, 0);
		interrupt(0x21, 0x01, input, 1, 0);	
		
		//Parse untuk argumen
		iterator = 0;
		
		while (input[iterator] != ' ' && input[iterator] != '\0') {
			iterator++;
		}
		iterator++;
		
		// Menyalin tiap argumen ke argv
		argc = countArgs(input);
		for (i = 0; i < argc; i++) {
			for (j = 0; input[iterator] != ' ' && input[iterator] != '\0'; j++) {
				argv[i][j] = input[iterator];
				iterator++;
			}
			argv[i][j] = '\0';
			iterator++;
		}
		
		
		// Menyalin nama program (kata terkiri)
		for (i = 0; input[i] != ' ' && input[i] != '\0'; i++) {
			str[i] = input[i];
		}
		str[i] = '\0';

		// Passing curdir, argc, dan argv
		interrupt(0x21, 0x20, curdir, argc, argv);

		clear(*argv, iterator-1);

		interrupt(0x21, 0x23, 0, ch, 0);
		interrupt(0x21, 0x23, argc-1, and, 0);

		// Meluncurkan program
		if (strcmp(str, "cd")) {
			cd(&curdir);
		} else if (strcmp(str, "ps")) {
			interrupt(0x21, 0x35, 0, 0, 0);
		} else if (strcmp(str, "pause")) {
			pause(ch[0] - '0');
		} else if (strcmp(str, "resume")) {
			resume(ch[0] - '0');
		} else if (strcmp(str, "kill")) {
			kill(ch[0] - '0');
		} else if (strcmp(str, "parallel")) {
			parallel(ch[0] - '0');
		} else {
			// Luncurkan program eksternal
			if (!strcmp(and, "&")) {
				interrupt(0x21, 0xFF << 8 | 0x6, str, &success);
			} else {
				interrupt(0x21, 0xFF << 8 | 0x36, str, &success);
			}
			if (success == -1) {
				interrupt(0x21, 0x00, str, 0, 0);
				interrupt(0x21, 0x00, " is not a command.\n", 0, 0);
			} 
		}
	}
	
	return 0;
}

int countArgs(char* str) {
	int i = 0;
	int count = 0;
	
	while (str[i] != '\0') {
		if (str[i] == '\x20' && str[i+1] != '\0') {
			count++;
		}
		i++;
	}
	
	return count;
}

int strcmp(char *str1, char *str2) {
	int count1 = 0, count2 = 0;
	int i;
	
	for (i = 0; str1[i] != '\0'; i++) {
		count1++;
	}
	
	for (i = 0; str2[i] != '\0'; i++) {
		count2++;
	}
	
	if (count1 != count2) {
		return FALSE;
	} else {
		for (i = 0; i < count1; i++) {
			if (str1[i] != str2[i]) {
				return 0;
			}
		}
		
		return TRUE;
	}	
}

void cd(char *parentIndex) {
	int i, j, iterator, pivot;
	int cont;
	char argc;
	char *path;
	char parent;
	char dirs[512];

	interrupt(0x21, 0x22, &argc, 0, 0);
	interrupt(0x21, 0x23, 0, path, 0);
	interrupt(0x21, 0x02, dirs, 0x101, 0);

	if (argc == 1) {
		if ((!strcmp("..", path)) && (!strcmp("/..", path))) {
			// Pencarian indeks direktori pada sektor dirs
			parent = *parentIndex;
			if (path[0] != '/') pivot = 0; else pivot = 1;
			while (TRUE) {
				i = 0;
				while (i < ENTRY_LENGTH) {
					cont = TRUE;
					iterator = pivot;
				
					// Mencocokkan nama direktori dengan path
					for (j = 0; dirs[i*DIRS_ENTRY_LENGTH+j+1] != '\0' || (path[iterator] != '\0' && path[iterator] != '/') && j < MAX_NAME; j++) {
						if (dirs[i*DIRS_ENTRY_LENGTH] != parent || dirs[i*DIRS_ENTRY_LENGTH+j+1] != path[iterator]){
							cont = FALSE;
							break;
						}
						iterator++;
					}
					if (cont) {
						break;
					} else {
						i++;
					}
				}
			
				if (i == ENTRY_LENGTH || j == 0) {
					if (lookSlash(pivot, path)) {
						// Direktori tidak ditemukan
						interrupt(0x21, 0x00, "Directory not found.\n", 0, 0);
						return;
					}
				}
			
				// Menyalin indeks parent untuk iterasi berikutnya
				parent = i;
				if (!lookSlash(pivot, path) && j !=0) {
					*parentIndex = parent;
					return;
				}

				// Pivot diletakkan setelah garis miring
				pivot = iterator+1;
			}
		} else {
			if (*parentIndex != 0xFF) {
				*parentIndex = dirs[(*parentIndex)*DIRS_ENTRY_LENGTH];
			}
		}
	} else {
		interrupt(0x21, 0x00, "Use: cd <path> or cd ..", 0, 0);
	}
}

void clear(char *buffer, int length){
  int i;
  for(i = 0; i < length; ++i){
    buffer[i] = EMPTY;
  }
}

int lookSlash(int startIndex, char* path) {
	int i;
	for (i = startIndex; i < MAX_NAME; i++) {
		if (path[i] == '/' || path[i] == '\0') {
			break;
		}
	}
	
	if (path[i] == '/') {
		return TRUE;
	} else {
		return FALSE;
	}
}


void printString(char *string, int length){
	int i = 0;
	char c;
	
	while (i < length) {
		c = string[i];
		if (c == '\n') interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
		interrupt(0x10, 0xE00 + c, 0, 0, 0);
		i++; 
	}
}

void pause(int pid) {
	int success;
	interrupt(0x21, 0x32, (pid + 2) << 12, &success, 0);
}

void resume(int pid) {
	int success;
	interrupt(0x21, 0x33, (pid + 2) << 12, &success, 0);
}

void kill(int pid) {
	int success;
	interrupt(0x21, 0x34, (pid + 2) << 12, &success, 0);
}

void parallel(int pid) {
	int success;
	interrupt(0x21, 0x37, (pid + 2) << 12, &success, 0);
}