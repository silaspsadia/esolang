#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXEC_ARR_SIZE 1024000
#define MEMORY_SIZE   1024000
#define READ_BUFSIZE  400000

static int execution_mode;
static int nchars;
static int ncodons;
static int ip;
static int level;
static int exec;
static int *exec_arr;
static int loc;
static unsigned char strg;
static unsigned char *memory;
static unsigned char *ptr;

int proc_codon(int codon);

int main(int argc, char **argv) {
	int		i, place, inst, codon;
	char	c, *read_buf;
	FILE	*file;

	if (argc != 2) {
		printf("Usage: %s <file.rna>\n", argv[0]);
		return 1;
	}
	if ((file = fopen(argv[1], "r")) < 0) {
		printf("Error: could not open file\n");
		return 1;
	}
	if ((read_buf = calloc(READ_BUFSIZE, sizeof(char))) == NULL) {
		printf("Error: bad calloc for read buffer\n");
		return 1;
	}

	for (nchars = 0; (c = fgetc(file)) != EOF;) {
		if (c == ' ' || c == '\t' || c == '\n')
			continue;
		else if (c == '#') {
			while ((c = fgetc(file)) != '\n' && c != EOF)
				;
		} else if (c == '[') {
			while ((c = fgetc(file) != ']' && c != EOF))
				;		
		} else {
			read_buf[nchars] = c;
			nchars++;
		}
	}

	fclose(file);

	if ((exec_arr = calloc(EXEC_ARR_SIZE, sizeof(char))) == NULL) {
		printf("Error: bad calloc for execution array\n");
		return 1;
	}
	if ((memory = calloc(MEMORY_SIZE, sizeof(char))) == NULL) {
		printf("Error: bad calloc for memory array\n");
		return 1;
	}

	ptr = memory;
	ncodons = 0, i = 0;
	while (i < nchars) {
		codon = 0;
		for (place = 1; place <= 100; place *= 10) {
			inst = 0;
			switch (read_buf[i++]) {
				case 'A':
				case 'a':
					inst = 1;
					break;
				case 'U':
				case 'u':
					inst = 2;
					break;
				case 'G':
				case 'g':
					inst = 3;
					break;
				case 'C':
				case 'c':
					inst = 4;
					break;
			}
			if (inst == 0)
				continue;
			codon += inst * place;
		}
		exec_arr[ncodons] = codon;
		ncodons++;
	}

	execution_mode = 0, ip = 0;
	while (ip < ncodons) {
		ip = proc_codon(exec_arr[ip]);
	}
}	

int proc_codon(int codon) {
#ifdef DEBUG
	printf("Read codon: %i\n", codon);
#endif
	switch(codon) {
		case 321:	
			execution_mode = 1;
			break;
		case 112:
		case 312:
		case 132:		
			execution_mode = 0;
			break;
		case 332:
			if (execution_mode)
				strg = 0;
			break;
		case 111:
		case 311:
			if (execution_mode) 
				++strg;
			break;
		case 411:
		case 211:
			if (execution_mode)
				--strg;
			break;
		case 143:
		case 443:
		case 343:
		case 243:
			if (execution_mode)
				strg = *ptr;
			break;
		case 141:
		case 441:
		case 341:
		case 241:
			if (execution_mode)
				ptr = &memory[strg];
			break;
		case 144:
		case 444:
		case 344:
		case 244:
			if (execution_mode)
				scanf("%d", ptr);
			break;
		case 124:
		case 424:
		case 324:
		case 224:
#ifdef DEBUG
			printf("%i\n", *ptr);
#endif
			if (execution_mode) 
				putchar(*ptr);
			break;
		case 131:
		case 331:
		case 134:
		case 434:
		case 334:
		case 234:
			if (execution_mode) 
				*ptr += memory[strg];
			break;
		case 431:
		case 231:
		case 142:
		case 442:
		case 342:
		case 242:
			if (execution_mode)
				*ptr *= memory[strg];
			break;
		case 114:
		case 314:
			if (execution_mode)
				*ptr -= memory[strg];
			break;
		case 414:
		case 214:
			if (execution_mode)
				*ptr /= memory[strg];
			break;
		case 113:
		case 313:
			if (execution_mode) 
				*ptr = (*ptr == memory[strg] ? 1 : 0);
			break;
		case 413:
		case 213:
			// LOOP ACTIVATION INDICATOR
			if (!execution_mode)
				break;
			level = 0;
			loc = ip + 1;
			if (*ptr <= 0) {
				while (loc < ncodons) {
					exec = exec_arr[loc];
					if (exec == 412 || exec == 212) {
						if (level == 0)
							return loc + 1;
						else 
							--level;
					} else if (exec == 413 || exec == 213) {
						++level;
					}
					++loc;
				}
				printf("Error: reached end of file while processing loop\n");
				exit(1);
			}
			break;
		case 412:
		case 212:
			// LOOP TERMINATION INDICATOR
			if (!execution_mode)
				break;
			level = 0;
			loc = ip - 1;
			if (*ptr > 0) {
				while (loc > 0) {
					exec = exec_arr[loc];
					if (exec == 413 || exec == 213) {
						if (level == 0) 
							return loc + 1;
						else
							--level;
					} else if (exec == 412 || exec == 212) {
						++level;
					}
					--loc;
				}
				printf("Error: reached past 0th exec while processing loop\n");
				exit(1);
			}
			break;
		default:
			// No-op
			break;
	}
	return ip + 1;
}
