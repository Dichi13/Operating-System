#define TRUE 1
#define FALSE 0

int strcmp(char *str1, char *str2);
void clear(char *buffer, int length);

int main() {
	int i;
	char argc;
	char curdir;
	char buffer[512];
	char **argv;
	
	enableInterrupts();

	interrupt(0x21, 0x21, &curdir, 0, 0);
	interrupt(0x21, 0x22, &argc, 0, 0);
	for (i = 0; i < argc; ++i) {
		interrupt(0x21, 0x23, i, argv[i], 0);
	}
	
	if (argc == 1) {
		interrupt(0x21, 0xFF << 8 | 0x04, buffer, argv[0], &i);
		if (i > 0) {
			interrupt(0x21, 0x0, "Content of ", 0, 0);
			interrupt(0x21, 0x0, argv[0], 0, 0);
			interrupt(0x21, 0x0, ":\n", 0, 0);
			interrupt(0x21, 0x0, buffer, 0, 0);
			interrupt(0x21, 0x0, "\n", 0, 0);
		} else {
			interrupt(0x21, 0x0, "File ", 0, 0);
			interrupt(0x21, 0x0, argv[0], 0, 0);
			interrupt(0x21, 0x0, " might be corrupt or non-existent.\n", 0, 0);
			interrupt(0x21, curdir << 8 | 0x6, "shell", 0x2000, &i);
		}
	} else if (argc == 2 && strcmp(argv[1], "-w")) {
		interrupt(0x21, 0x0, "You're currently in write mode. Press enter to input your text to this file: \n", 0, 0);
		interrupt(0x21, 0x1, buffer, 0, 0);
		i = 1;
		interrupt(0x21, curdir << 8 | 0x05, buffer, argv[0], &i);
		if (i != 1) {
			interrupt(0x21, 0x0, "File cannot be created; probably because the path doesn't exist or insufficient space.", 0, 0);
		}
	} else {
		interrupt(0x21, 0x0, "Use cat <filepath> to view file or cat <filepath> -w to create a file to the designated path.\n", 0, 0);
	}

	interrupt(0x21, 0x07, i, 0, 0);
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

void clear(char *buffer, int length){
  int i;
  for(i = 0; i < length; ++i){
    buffer[i] = 0x00;
  }
}
