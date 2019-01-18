void printName(char* str, int index);

int main() {
	int i, j;
	char argc;
	char curdir;
	char **argv;
	char dirs[512];
	char files[512];

	enableInterrupts();

	interrupt(0x21, 0x21, &curdir, 0, 0);
	interrupt(0x21, 0x22, &argc, 0, 0);
	for (i = 0; i < argc; ++i) {
		interrupt(0x21, 0x23, i, argv[i], 0);
	}

	interrupt(0x21, 0x02, dirs, 0x101, 0);
	interrupt(0x21, 0x02, files, 0x102, 0);
	
	interrupt(0x21, 0x00, "In this directory: \n", 0, 0);
	for (i = 0; i < 32; i++) {
		if(dirs[16*i] == curdir && dirs[16*i+1] != '\0') {
			interrupt(0x21, 0x00, "DIR  | ", 0, 0);
			printName(dirs, 16*i+1);
			interrupt(0x21, 0x00, "\n", 0, 0);
		}
	}
	for (i = 0; i < 32; i++) {
		if(files[16*i] == curdir && files[16*i+1] != '\0') {
			interrupt(0x21, 0x00, "FILE | ", 0, 0);
			printName(files, 16*i+1);
			interrupt(0x21, 0x00, "\n", 0, 0);
		}
	}

	interrupt(0x21, 0x07, i, 0, 0);
}

void printName(char* str, int idx) {
	int i;
	char temp[16];
	
	for (i = 0; i < 15 && str[idx+i] != '\0'; i++) {
		temp[i] = str[idx+i];
	}
	temp[i] = '\0';
	
	interrupt(0x21, 0x00, temp, 0, 0);
}
	
		
