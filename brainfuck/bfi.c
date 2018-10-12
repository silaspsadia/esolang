#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INIT_TAPESIZE 4096000
#define INSTRUCTION_BUFSIZE 500000
#define DEBUG

int proc_char(char c);
int error(char *msg);

static char *_tape;
static int _tp;

static int ip;
static int nchars;
static char *ibuf, c;

int main(int argc, char **argv)
{
	FILE *file;

	if (argc != 2) 
		error("not enough command line args provided\n");
	if (!(file = fopen(argv[1], "r"))) 
		error("failed to open file for reading\n");
	
	rewind(file);

	if ((ibuf = calloc(nchars + 1, sizeof(char))) == NULL) 
		error("failed to allocate memory for instruction buffer\n");
	if ((_tape = calloc(INIT_TAPESIZE, sizeof(char))) == NULL) 
		error("failed to allocate memory for master tape\n");
	
		
	for (nchars = 0; (ibuf[nchars] = fgetc(file)) != EOF; nchars++);

	fclose(file);

	ip = 0;
	while (ip < nchars) {
		c = ibuf[ip];
		ip = proc_char(c);
	}
}

int proc_char(char c) {
	switch (c) {
		case '>':
			++_tp;
			break;
		case '<':
			if (_tp <= 0) {
				error("cannot go behind 0th position\n");
				return 0;
			} else {
				--_tp;
			}
			break;
		case '+':
			++_tape[_tp];
			break;
		case '-':
			--_tape[_tp];
			break;
		case '.':
			putchar(_tape[_tp]);
			break;
		case ',':
			_tape[_tp] = getchar();
			break;
		case '[':
			if (_tape[_tp] == 0) {
				int dist = 1, level = 0;
				while (ip + dist < nchars) {
					dist++;
					c = ibuf[ip + dist];
					if (c == ']') {
						if (level == 0) {
							return ip + dist + 1;
						} else {
							--level;
						}
					} else if (c == '[') {
						++level;
					}
				}
				printf("TAPE\n");
			}
			break;
		case ']':
			if (_tape[_tp] > 0) {
				int dist = 1, level = 0;
				while (ip - dist >= 0) {
					c = ibuf[ip - dist];
					if (c == '[') {
						if (level == 0) {
							return ip - dist + 1;
						} else {
							--level;
						}
					} else if (c == ']') {
						++level;
					}
					++dist;
				}
				printf("reached end of tape while processing loop\n");
			}
			break;
		default: 
			;
	}
	return ip + 1;
}	

int error(char *msg) {
	printf("Error: %s", msg);
	exit(1);
}
