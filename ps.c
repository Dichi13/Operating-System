#include "proc.h"

void writeInt(int x);
int mod(int a, int b);
int div(int a, int b);

int main() {
	int i, pid;

	enableInterrupts();
	for(i = 0; i < 8; i++){
		if (pcbPool[i].state != DEFUNCT) {
			char *state;
			switch (pcbPool[i].state) {
				case 1 : state = "RUNNING"; break;
				case 2 : state = "STARTING"; break;
				case 3 : state = "READY"; break;
				case 4 : state = "PAUSED";			
			};

			pid = (pcbPool[i].segment >> 12) - 2;

			interrupt(0x21, 0x00, "Program with PID ", 0, 0);
			writeInt(pid);
			interrupt(0x21, 0x00, " is ", 0, 0);
			interrupt(0x21, 0x00, state, 0, 0);
			interrupt(0x21, 0x00, "\n", 0, 0);
		}
	}

	interrupt(0x21, 0x07, i, 0, 0);
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