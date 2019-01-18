int main() {
	int i;
	char argc;
	char argv[4][16];

	enableInterrupts();

	interrupt(0x21, 0x22, &argc, 0, 0);
	for (i = 0; i < argc; ++i) {
		interrupt(0x21, 0x23, i, argv[i], 0);
		interrupt(0x21, 0x00, argv[i], 0, 0);
		interrupt(0x21, 0x00, " ", 0, 0);	
	}
	
	interrupt(0x21, 0x00, "\n", 0, 0);
	interrupt(0x21, 0x07, i, 0, 0);
}
