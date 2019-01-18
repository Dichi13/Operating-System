#define MAIN
#include "proc.h"
#define MAX_BYTE 256
#define SECTOR_SIZE 512

#define INSUFFICIENT_DIR_ENTRIES -1

// end of salinan
#define ARGS_SECTOR 512
#define DIRS_ENTRY_LENGTH 16
#define FILES_ENTRY_LENGTH 16
#define SECTORS_ENTRY_LENGTH 16
#define ENTRY_LENGTH 32
#define MAX_NAME 15
#define MAX_SECTORS 16
#define MAP_SECTOR 0x100
#define DIRS_SECTOR 0x101
#define FILES_SECTOR 0x102
#define SECTORS_SECTOR 0x103
#define INSUFFICIENT_SECTORS 0
#define INSUFFICIENT_SEGMENTS -1
#define NOT_FOUND -1
#define ALREADY_EXISTS -2
#define INSUFFICIENT_ENTRIES -3
#define EMPTY 0x00
#define USED 0xFF
#define TRUE 1
#define FALSE 0
#define SUCCESS 0

// Constants for the various states that a process may be in.
#define DEFUNCT 0
#define RUNNING 1
#define STARTING 2
#define READY 3
#define PAUSED 4

void handleInterrupt21 (int AX, int BX, int CX, int DX);
void printString(char *string);
void readString(char *string, int disableProcessControls);
int mod(int a, int b);
int div(int a, int b);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void readFile(char *buffer, char *path, int *result, char parentIndex);
void clear(char *buffer, int length);
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
void executeProgram(char *path, int *result, char parentIndex);
void executeParallel(char *path, int *result, char parentIndex);
void terminateProgram(int *result);
void makeDirectory(char *path, int *result, char parentIndex);
void deleteFile(char *path, int *result, char parentIndex);
void deleteDirectory(char *path, int *success, char parentIndex);
void putArgs(char curdir, char argc, char **argv);
void getCurdir(char *curdir);
void getArgc(char *argc);
void getArgv(char index, char *argv);
void handleTimerInterrupt(int segment, int stackPointer);
void yieldControl();
void sleep();
void pauseProcess(int segment, int *result);
void resumeProcess(int segment, int *result);
void killProcess(int segment, int *result);
int lookSlash(int startIndex, char* path);
void writeInt(int x);
void ps();

int main() {
	int success = 1;
	char buffer[512];
	char sfaasdf[512];
	initializeProcStructures();
	makeInterrupt21();
	makeTimerInterrupt();

	// Menjalankan shell
	putArgs(0xFF, 0, 0);
	interrupt(0x21, 0xFF << 8 | 0x6, "shell", &success);
	return 0;
}

void handleInterrupt21 (int AX, int BX, int CX, int DX) {
	char AL, AH;
	AL = (char) (AX);
	AH = (char) (AX >> 8);
	switch (AL) {
		case 0x00:
			printString(BX);
			break;
		case 0x01:
			readString(BX, CX);
			break;
		case 0x02:
			readSector(BX, CX);
			break;
		case 0x03:
			writeSector(BX, CX);
			break;
		case 0x04:
			readFile(BX, CX, DX, AH);
			break;
		case 0x05:
			writeFile(BX, CX, DX, AH);
			break;
		case 0x06:
			executeProgram(BX, CX, AH);
			break;
		case 0x07:
			terminateProgram(BX);
			break;
		case 0x08:
			makeDirectory(BX, CX, AH);
			break;
		case 0x09:
			deleteFile(BX, CX, AH);
			break;
		case 0x0A:
			deleteDirectory(BX, CX, AH);
			break;
		case 0x20:
			putArgs(BX, CX, DX);
			break;
		case 0x21:
			getCurdir(BX);
			break;
		case 0x22:
			getArgc(BX);
			break;
		case 0X23:
			getArgv(BX, CX);
			break;
		case 0x30:
			yieldControl();
			break;
		case 0x31:
			sleep();
			break;
		case 0x32:
			pauseProcess(BX, CX);
			break;
		case 0x33:
			sleep();
			resumeProcess(BX, CX);
			break;
		case 0x34:
			killProcess(BX, CX);
			break;
		case 0x35:
			ps();
			break;
		case 0x36:
			executeParallel(BX, CX, AH);
			break;
		case 0x37:
			resumeProcess(BX, CX);
			break;
		default:
			printString("Invalid interrupt");
	}
}

int mod(int a, int b){
  while(a >= b){
    a = a-b;
  }

  return a;
}

int div(int a, int b){
  int q = 0;
  while(q*b <= a) {
    q = q+1;
  }

  return q-1;
}

void printString(char *string){
	int i = 0;
	char c;
	
	while (string[i] != '\0') {
		c = string[i];
		if (c == '\n') interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
		interrupt(0x10, 0xE00 + c, 0, 0, 0);
		i++; 
	}
}

void readString(char *string, int disableProcessControls) {
	char c = interrupt(0x16, 0, 0, 0, 0);
	int i = 0;
	int continueReading = TRUE;

	clear(string, 8);

	if (disableProcessControls == FALSE) {
		if (c == 0x03) {
			terminateProgram(&i);
			continueReading = FALSE;
		} else if (c == 0x1A) {
			sleep();
			resumeProcess(0x2000, &i);
			continueReading = FALSE;
		}
	}
	if (continueReading) {
		while(c != '\r'){		
			if (c == '\b') {
				if (i > 0) {
					string[i] = '\0';
					interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
					interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
					interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
					i--;
				}
			} else {
				string[i] = c;
				interrupt(0x10, 0xE00 + c, 0, 0, 0);
				i++;
			}

			c = interrupt(0x16, 0, 0, 0, 0);
		}
		interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
		interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
		string[i] = '\0';
	}
}

void readSector(char *buffer, int sector){
	interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char *buffer, int sector){
	interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void readFile(char *buffer, char *path, int *result, char parentIndex) {
	int dir, dirIndex, parent, iterator, pivot;
	int i, j;
	int cont, done;
	char dirs[SECTOR_SIZE];
	char files[SECTOR_SIZE];
	char sectors[SECTOR_SIZE];

	readSector(dirs, DIRS_SECTOR);
	readSector(files, FILES_SECTOR);
	readSector(sectors, SECTORS_SECTOR);
	
	// Pencarian indeks direktori pada sektor dirs
	parent = parentIndex;
	if (path[0] != '/') pivot = 0; else pivot = 1;
	done = FALSE;
	while (!done && lookSlash(pivot, path)) {
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
		
		// Pivot diletakkan setelah garis miring
		pivot = iterator+1;
		
		if (path[iterator] == '/' && i == ENTRY_LENGTH) {
			// Direktori tidak ditemukan
			*result = NOT_FOUND;
			printString("readFile notFound\n");
			return;
		}
		// Menyalin indeks parent untuk iterasi berikutnya
		parent = i;
		
		// Apabila seluruh kondisi terpenuhi, keluar dari iterasi
		if (path[iterator] == '\0' && cont) {
			done = TRUE;
		}	
	}

	// Pencarian nama file pada files
	i = 0;
	while (i < ENTRY_LENGTH) {
		cont = TRUE;
		iterator = pivot;

		// Mencocokkan nama file dengan path
		for (j = 0; files[i*FILES_ENTRY_LENGTH+j+1] != '\0' || (path[iterator] != '\0' && path[iterator] != '/') && j < MAX_NAME; j++) {
			if (files[i*FILES_ENTRY_LENGTH] != parent || files[i*FILES_ENTRY_LENGTH+j+1] != path[iterator]){
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
	printString("Lewaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaat\n");
	if (i == ENTRY_LENGTH || j == 0) {
		printString("Kok bisa\n");
		*result = NOT_FOUND;
		return;
	} else {
		int k = 0;
		while ((k < SECTORS_ENTRY_LENGTH) && (sectors[i*SECTORS_ENTRY_LENGTH+k] != EMPTY)) {
			readSector(buffer + k * SECTOR_SIZE, sectors[i*SECTORS_ENTRY_LENGTH+k]);
			k++;
		}
		*result = i;
	}
}

void clear(char *buffer, int length){
  int i;
  for(i = 0; i < length; ++i){
    buffer[i] = EMPTY;
  }
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex) {
	int dir, dirIndex, sectorCount, parent, iterator, pivot;
	int i, j;
	int cont, done;
	char map[SECTOR_SIZE];
	char dirs[SECTOR_SIZE];
	char files[SECTOR_SIZE];
	char sectors[SECTOR_SIZE];
	char sectorBuffer[SECTOR_SIZE];
	char dirName[MAX_NAME];

	clear(dirName, MAX_NAME);
	
	readSector(map, MAP_SECTOR);
	readSector(dirs, DIRS_SECTOR);
	readSector(files, FILES_SECTOR);
	readSector(sectors, SECTORS_SECTOR);
	
	// Mencari indeks files yang kosong
	for (dirIndex = 0; dirIndex < ENTRY_LENGTH; ++dirIndex) {
		if (files[dirIndex * DIRS_ENTRY_LENGTH+1] == '\0') {
		  break;
		}
	}
	
	if (dirIndex >= ENTRY_LENGTH) {
		*sectors = INSUFFICIENT_ENTRIES;
		return;
	}
	
	// Menghitung jumlah sektor yang kosong
	for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < *sectors; ++i) {
	  if (map[i] == EMPTY) {
	    sectorCount++;
	  }
	}
	if (sectorCount < *sectors) {
	  *sectors = INSUFFICIENT_SECTORS;
	  return;
	}
	
	// Pencarian indeks direktori pada sektor dirs
	parent = parentIndex;
	if (path[0] != '/') pivot = 0; else pivot = 1;
	done = FALSE;
	while (!done && lookSlash(pivot, path)) {
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
		
		// Pivot diletakkan setelah garis miring
		pivot = iterator+1;
		
		if (j == 0 || i == ENTRY_LENGTH) {
			// Direktori tidak ditemukan
			*sectors = NOT_FOUND;
			return;
		}
		// Menyalin indeks parent untuk iterasi berikutnya
		parent = i;
		
		// Apabila seluruh kondisi terpenuhi, keluar dari iterasi
		if (path[iterator] == '\0' && cont) {
			done = TRUE;
		}	
	}
	
	// Pencarian nama file pada files
	i = 0;
	while (i < ENTRY_LENGTH) {
		cont = TRUE;
		iterator = pivot;

		// Mencocokkan nama file dengan path
		for (j = 0; files[i*FILES_ENTRY_LENGTH+j+1] != '\0' || (path[iterator] != '\0' && path[iterator] != '/') && j < MAX_NAME; j++) {
			if (files[i*FILES_ENTRY_LENGTH] != parent || files[i*FILES_ENTRY_LENGTH+j+1] != path[iterator]){
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
		for (j = pivot; path[j] != '\0'; j++) {
			dirName[j-pivot] = path[j];
		}
		dirName[j] = '\0';

		files[dirIndex * FILES_ENTRY_LENGTH] = parent;
		for (i = 0; dirName[i] != '\0'; ++i) {
			files[dirIndex * FILES_ENTRY_LENGTH + i + 1] = dirName[i];
		}
		
		for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < *sectors; ++i) {
			if (map[i] == EMPTY) {
				map[i] = USED;
				sectors[dirIndex * DIRS_ENTRY_LENGTH + sectorCount] = i;
				clear(sectorBuffer, SECTOR_SIZE);
				for (j = 0; j < SECTOR_SIZE; ++j) {
					sectorBuffer[j] = buffer[sectorCount * SECTOR_SIZE + j];
				}
				writeSector(sectorBuffer, i);
				++sectorCount;
			}
		}
		writeSector(files, FILES_SECTOR);
		writeSector(sectors, SECTORS_SECTOR);
		writeSector(map, MAP_SECTOR);
	} else {
		*sectors = ALREADY_EXISTS;
	}
}

void executeProgram(char *path, int *result, char parentIndex) {
	struct PCB* pcb;
	int segment;
	int i, fileIndex;
	char buffer[MAX_SECTORS * SECTOR_SIZE];
	readFile(buffer, path, result, parentIndex);

	if (*result != NOT_FOUND) {
		setKernelDataSegment();
		segment = getFreeMemorySegment();
		restoreDataSegment();
		fileIndex = *result;
		if (segment != NO_FREE_SEGMENTS) {
			setKernelDataSegment();
			pcb = getFreePCB();
			pcb->index = fileIndex;
			pcb->state = STARTING;
			pcb->segment = segment;
			pcb->stackPointer = 0xFF00;
			pcb->parentSegment = running->segment;
			addToReady(pcb);
			restoreDataSegment();
			for (i = 0; i < SECTOR_SIZE * MAX_SECTORS; i++) {
				putInMemory(segment, i, buffer[i]);
			}
			initializeProgram(segment);
			sleep();
		} else {
			*result = INSUFFICIENT_SEGMENTS;
		}
	}
}


void executeParallel(char *path, int *result, char parentIndex) {
	struct PCB* pcb;
	int segment;
	int i, fileIndex;
	char buffer[MAX_SECTORS * SECTOR_SIZE];
	readFile(buffer, path, result, parentIndex);

	if (*result != NOT_FOUND) {
		setKernelDataSegment();
		segment = getFreeMemorySegment();
		restoreDataSegment();
		fileIndex = *result;
		if (segment != NO_FREE_SEGMENTS) {
			setKernelDataSegment();
			pcb = getFreePCB();
			pcb->index = fileIndex;
			pcb->state = STARTING;
			pcb->segment = segment;
			pcb->stackPointer = 0xFF00;
			pcb->parentSegment = running->segment;
			addToReady(pcb);
			restoreDataSegment();
			for (i = 0; i < SECTOR_SIZE * MAX_SECTORS; i++) {
				putInMemory(segment, i, buffer[i]);
			}
			initializeProgram(segment);
		} else {
			*result = INSUFFICIENT_SEGMENTS;
		}
	}
}

void terminateProgram(int *result) {
	int parentSegment;

	setKernelDataSegment();
	parentSegment = running->parentSegment;
	releaseMemorySegment(running->segment);
	releasePCB(running);
	restoreDataSegment();
	if (parentSegment != NO_PARENT) {
		resumeProcess(parentSegment, result);
	}
	yieldControl();
}

void makeDirectory(char *path, int *result, char parentIndex) {
	int dir, dirIndex, parent, iterator, pivot;
	int i, j;
	int cont;
	char dirName[MAX_NAME];
	char dirs[SECTOR_SIZE];
	
	clear(dirName, MAX_NAME);

	readSector(dirs, DIRS_SECTOR);
	
	// Cek entri kosong pada sektor dirs
	for (dirIndex = 0; dirIndex < ENTRY_LENGTH; dirIndex++) {
		if (dirs[dirIndex*DIRS_ENTRY_LENGTH+1] == EMPTY) {
			break;
		}
	}
	
	if (dirIndex == ENTRY_LENGTH) {
		*result = INSUFFICIENT_ENTRIES;
		return;
	}

	// Pencarian indeks direktori pada sektor dirs
	parent = parentIndex;
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
				*result = NOT_FOUND;
				return;
			} else {
				// Menyalin nama dari current directory
				for (j = pivot; path[j] != '\0'; j++) {
					dirName[j-pivot] = path[j];
				}
				dirName[j] = '\0';
				break;
			}
		} else if (path[iterator] == '\0' && cont) {
			// Direktori sudah ada
			*result = ALREADY_EXISTS;
			return;
		}
		
		// Pivot diletakkan setelah garis miring
		pivot = iterator+1;
		
		// Menyalin indeks parent untuk iterasi berikutnya
		parent = i;
	}
	
	dirs[dirIndex*DIRS_ENTRY_LENGTH] = parent;
	i = 0;
	while (dirName[i] != '\0') {
		dirs[dirIndex*DIRS_ENTRY_LENGTH+i+1] = dirName[i];
		i++;
	}
	dirs[dirIndex*DIRS_ENTRY_LENGTH+i+1] = '\0';
	
	writeSector(dirs, DIRS_SECTOR);
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

void deleteFile(char *path, int *result, char parentIndex) {
	int dir, dirIndex, parent, iterator, pivot;
	int i, j;
	int cont, done;
	char map[SECTOR_SIZE];
	char dirs[SECTOR_SIZE];
	char files[SECTOR_SIZE];
	char sectors[SECTOR_SIZE];
	
	readSector(map, MAP_SECTOR);
	readSector(dirs, DIRS_SECTOR);
	readSector(files, FILES_SECTOR);
	readSector(sectors, SECTORS_SECTOR);
	
	// Pencarian indeks direktori pada sektor dirs
	parent = parentIndex;
	if (path[0] != '/') pivot = 0; else pivot = 1;
	done = FALSE;
	while (!done && lookSlash(pivot, path)) {
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
		
		// Pivot diletakkan setelah garis miring
		pivot = iterator+1;
		
		if (path[iterator] == '/' && i == ENTRY_LENGTH) {
			// Direktori tidak ditemukan
			*result = NOT_FOUND;
			return;
		}
		// Menyalin indeks parent untuk iterasi berikutnya
		parent = i;
		
		// Apabila seluruh kondisi terpenuhi, keluar dari iterasi
		if (path[iterator] == '\0' && cont) {
			done = TRUE;
		}	
	}
	
	// Pencarian nama file pada files
	i = 0;
	while (i < ENTRY_LENGTH) {
		int cont = TRUE;
		iterator = pivot;
		
		// Mencocokkan nama file dengan path
		for (j = 0; files[i*FILES_ENTRY_LENGTH+j+1] != '\0' || (path[iterator] != '\0' && path[iterator] != '/') && j < MAX_NAME; j++) {
			if (files[i*FILES_ENTRY_LENGTH] != parent || files[i*FILES_ENTRY_LENGTH+j+1] != path[iterator]){
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
		*result = NOT_FOUND;
		return;
	} else {
		files[i*FILES_ENTRY_LENGTH+1] = '\0';
		for (j = 0; sectors[i*FILES_ENTRY_LENGTH+j] != '\0'; j++) {
			map[sectors[i*FILES_ENTRY_LENGTH+j]] = EMPTY;
		}
	}
	
	writeSector(map, MAP_SECTOR);
	writeSector(files, FILES_SECTOR);

	*result = 0;
}

void deleteDirectory(char *path, int *success, char parentIndex) {
	int dir, dirIndex, parent, iterator, pivot;
	int i, j;
	int done, cont;
	char dirs[SECTOR_SIZE];
	char files[SECTOR_SIZE];
	
	readSector(dirs, DIRS_SECTOR);
	readSector(files, FILES_SECTOR);
	
	// Pencarian indeks direktori pada sektor dirs
	parent = parentIndex;
	if (path[0] != '/') pivot = 0; else pivot = 1;
	done = FALSE;
	while (!done) {
		i = 0;
		while (i < ENTRY_LENGTH) {
			cont = TRUE;
			iterator = pivot;
			
			// Mencocokkan indeks parent dan nama direktori dengan path
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
		
		// Pivot diletakkan setelah garis miring
		pivot = iterator+1;
		
		if (i == ENTRY_LENGTH || j == 0) {
			// Direktori tidak ditemukan
			*success = NOT_FOUND;
			return;
		}
		// Menyalin indeks parent untuk iterasi berikutnya
		parent = i;
		
		// Apabila seluruh kondisi terpenuhi, keluar dari iterasi
		if (path[iterator] == '\0' && cont) {
			done = TRUE;
		}	
	}
	
	// Menghapus direktori yang ada di dalam direktori ini
	for (i = 0; i < ENTRY_LENGTH; i++) {
		if (dirs[i*DIRS_ENTRY_LENGTH] == parent) {
			char otherPath[16];
			otherPath[0] = '/'; 
			for (j = 1; dirs[i*DIRS_ENTRY_LENGTH+j] != '\0'; j++) {
				otherPath[j] = dirs[i*DIRS_ENTRY_LENGTH+j+1];
			}
			deleteDirectory(otherPath[j], success, i);
		}
	}
	
	// Menghapus file yang ada di dalam direktori ini
	for (i = 0; i < ENTRY_LENGTH; i++) {
		if (files[i*FILES_ENTRY_LENGTH] == parent) {
			char otherPath[16];
			otherPath[0] = '/'; 
			for (j = 1; files[i*FILES_ENTRY_LENGTH+j] != '\0'; j++) {
				otherPath[j] = dirs[i*FILES_ENTRY_LENGTH+j+1];
			}
			deleteFile(otherPath[j], success, i);
		}
	}
	
	dirs[parent*DIRS_ENTRY_LENGTH+1] = '\0';

	writeSector(dirs, DIRS_SECTOR);
	writeSector(files, FILES_SECTOR);

	*success = 0;
}

void putArgs (char curdir, char argc, char **argv) {
	char args[SECTOR_SIZE];
	int i, j, p;
	clear(args, SECTOR_SIZE);
	
	args[0] = curdir;
	args[1] = argc;
	i = 0;
	j = 0;
	
	for (p = 2; p < ARGS_SECTOR && i < argc; ++p) {
		args[p] = argv[i][j];
		if (argv[i][j] == '\0') {
			++i;
			j = 0;
		} else {
			++j;
		}
	}
	
	writeSector(args, ARGS_SECTOR);
}
	
void getCurdir (char *curdir) {
	char args[SECTOR_SIZE];
	readSector(args, ARGS_SECTOR);
	*curdir = args[0];
}

void getArgc (char *argc) {
	char args[SECTOR_SIZE];
	readSector(args, ARGS_SECTOR);
	*argc = args[1];
}

void getArgv (char index, char *argv) {
	char args[SECTOR_SIZE];
	int i, j, p;
	readSector(args, ARGS_SECTOR);
	
	i = 0;
	j = 0;
	for (p = 2; p < ARGS_SECTOR; ++p) {
		if (i == index) {
			argv[j] = args[p];
			++j;
		}
		if (args[p] == '\0') {
			if (i == index) {
				break;
			} else {
				++i;
			}
		}
	}
}

void handleTimerInterrupt(int segment, int stackPointer) {
	struct PCB *currPCB;
	struct PCB *nextPCB;
	setKernelDataSegment();
	currPCB = getPCBOfSegment(segment);
	currPCB->stackPointer = stackPointer;
	if (currPCB->state != PAUSED) {
		currPCB->state = READY;
		addToReady(currPCB);
	}

	do {
		nextPCB = removeFromReady();
	}
	while (nextPCB != NULL && (nextPCB->state == DEFUNCT || nextPCB->state == PAUSED));
	
	if (nextPCB != NULL) {
		nextPCB->state = RUNNING;
		segment = nextPCB->segment;
		stackPointer = nextPCB->stackPointer;
		running = nextPCB;
	} else {
		running = &idleProc;
	}
	restoreDataSegment();
	returnFromTimer(segment, stackPointer);
}

void yieldControl () {
	interrupt(0x08, 0, 0, 0, 0);
}

void sleep () {
	setKernelDataSegment();
	running->state = PAUSED;
	restoreDataSegment();
	yieldControl();
}

void pauseProcess (int segment, int *result) {
	struct PCB *pcb;
	int res;
	setKernelDataSegment();
	pcb = getPCBOfSegment(segment);
	if (pcb != NULL && pcb->state != PAUSED) {
		pcb->state = PAUSED;
		res = SUCCESS;
	} else {
		res = NOT_FOUND;
	}
	restoreDataSegment();
	*result = res;
}

void resumeProcess (int segment, int *result) {
	struct PCB *pcb;
	int res;

	setKernelDataSegment();
	pcb = getPCBOfSegment(segment);
	if (pcb != NULL && pcb->state == PAUSED) {
		pcb->state = READY;
		addToReady(pcb);
		res = SUCCESS;
	} else {
		res = NOT_FOUND;
	}
	restoreDataSegment();

	*result = res;
}

void killProcess (int segment, int *result) {
	struct PCB *pcb;
	int res;
	
	setKernelDataSegment();
	pcb = getPCBOfSegment(segment);
	if (pcb != NULL) {
		releaseMemorySegment(pcb->segment);
		releasePCB(pcb);
		res = SUCCESS;
	} else {
		res = NOT_FOUND;
	}
	restoreDataSegment();

	*result = res;
}

void writeInt(int x)
{
  char number[6], *d;
  int q = x, r;  d = number + 5;
  *d = 0; 
  d--;
  while(q > 0)
    {
      r = mod(q,10);
      q = div(q,10);
      *d = r + 48; 
      d--;
    }
  d++;

	interrupt(0x21, 0x0, d, 0, 0);
}

void ps() {
	int i, pid, state, notNull, fileIndex;
	char a;
	struct PCB* pcb;
	char files[SECTOR_SIZE];

	readSector(files, FILES_SECTOR);

	for(i = 0; i < 8; i++){
		state = DEFUNCT;
		setKernelDataSegment();
		pcb = getPCBOfSegment((i+2) << 12);
		if (pcb != NULL) {
			pid = (pcb->segment >> 12) - 2;
			state = pcb->state;
			fileIndex = pcb->index;
		}
		restoreDataSegment();
		if (state != DEFUNCT) {
			setKernelDataSegment();
			printString("Program with PID ");
			a = pid + '0';
			interrupt(0x10, 0xE00 + a, 0, 0, 0);
			printString(" (");
			restoreDataSegment();
			printString(&files[fileIndex * FILES_ENTRY_LENGTH + 1]);
			setKernelDataSegment();
			printString(") is ");
			switch (state) {
				case 1 : printString("RUNNING\n"); break;
				case 2 : printString("STARTING\n"); break;
				case 3 : printString("READY\n"); break;
				case 4 : printString("PAUSED\n");			
			}
			restoreDataSegment();
		}
	}
}
