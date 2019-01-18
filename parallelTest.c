int main() {
	int i;
	enableInterrupts();

	while (1) {
		interrupt(0x21, 0x30, 0, 0, 0);
		interrupt(0x21, 0x00, "Tick\n", 0, 0);
		interrupt(0x21, 0x30, 0, 0, 0);
		interrupt(0x21, 0x30, 0, 0, 0);
		interrupt(0x21, 0x30, 0, 0, 0);
		interrupt(0x21, 0x30, 0, 0, 0);
	}
}
