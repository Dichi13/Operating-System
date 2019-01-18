#define TRUE 1
#define FALSE 0

int strcmp(char *str1, char *str2);
void clear(char *buffer, int length);

int main() {
	int i;
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
		interrupt(0x21, curdir << 8 | 0x08, argv[0], &i, 0);
		if (i != 1) {
			if (i == -1) {
				interrupt(0x21, 0x00, "Error creating directory: Directory not found.\n", 0, 0);
			} else if (i == -2) {
				interrupt(0x21, 0x00, "Error creating directory: Directory already exists.\n", 0, 0);
			} else {
				interrupt(0x21, 0x00, "Error creating directory.\n", 0, 0);
			}
			interrupt(0x21, 0xFF << 8 | 0x6, "shell", 0x2000, &i);
		}
	} else {
		interrupt(0x21, 0x00, "Arguments needed: mkdir <path>\n", 0, 0);
	}

	interrupt(0x21, 0x07, i, 0, 0);
}
