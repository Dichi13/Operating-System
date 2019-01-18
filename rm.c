#define TRUE 1
#define FALSE 0

int strcmp(char *str1, char *str2);
void clear(char *buffer, int length);

int main() {
	int i=0, j=0;
	char argc;
	char curdir;
	char **argv;

	enableInterrupts();
	
	interrupt(0x21, 0x21, &curdir, 0, 0);
	interrupt(0x21, 0x22, &argc, 0, 0);
	for (i = 0; i < argc; ++i) {
		interrupt(0x21, 0x23, i, argv[i], 0);
	}
	if (argc == 1) {
		interrupt(0x21, curdir << 8 | 0x09, argv[0], &i, 0);
		interrupt(0x21, curdir << 8 | 0x0A, argv[0], &j, 0);
		if (i == -1 && j == -1) {
			interrupt(0x21, 0x00, "Error deleting directory or file: directory or file not found.\n", 0, 0);
			interrupt(0x21, 0xFF << 8 | 0x6, "shell", 0x2000, &i);
		}
		interrupt(0x21, 0xFF << 8 | 0x6, "shell", 0x2000, &i);
	} else {
		interrupt(0x21, 0x00, "Arguments needed: rm <path>\n", 0, 0);
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
