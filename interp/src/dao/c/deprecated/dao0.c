#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_BIT_SIZE (unsigned long)(1 << 14)

#define FILE_SYMBOLIC ".dao"
#define FILE_COMPILED ".wuwei"

#define IDLES 0x0
#define SWAPS 0x1
#define LATER 0x2
#define MERGE 0x3
#define SIFTS 0x4
#define EXECS 0x5
#define DELEV 0x6
#define EQUAL 0x7
#define HALVE 0x8
#define UPLEV 0x9
#define READS 0xA
#define DEALC 0xB
#define SPLIT 0xC
#define POLAR 0xD
#define DOALC 0xE
#define INPUT 0xF

#define exit(x) return x;

typedef FILE* File;
typedef int boolean;
typedef struct PATH* Path;
typedef char* String;

void compile(File, File);
void run(File);

void idles(Path);
void swaps(Path);
void later(Path); 
void merge(Path);
void sifts(Path);
void execs(Path, Path); 
void delev(Path); 
void equal(Path);
void halve(Path);
void uplev(Path); 
void reads(Path); 
void dealc(Path);
void split(Path);
void polar(Path); 
void doalc(Path); 
void input(Path);

unsigned char getNybble(char);
char getChar(unsigned char);
unsigned long mask(int);
unsigned long report_by_bit_index(Path, unsigned int, unsigned int);

unsigned long* report_tot(Path);

void write_by_bit_index(Path, unsigned int, unsigned int, unsigned long);

char*			bin(unsigned long);
char*			itoa(unsigned long, unsigned char, unsigned char);
void			bin_print(Path);
void			skip();

void todo();

boolean aligned(Path);

struct PATH
{
	Path 			owner;						/* OWNER   PROGRAM     */
	Path 			child;						/* CHILD   PROGRAM     */

	unsigned long	prg_data[ MAX_BIT_SIZE ];	/*         DATA        */
	unsigned long	prg_allocbits;				/* OPEN    DATA   BITS */
	unsigned long	prg_index;					/* INSTRUCTION  POINTER*/
	unsigned char	prg_level;					/* OPERATING LEVEL     */
	
	unsigned long	sel_length;					/* LENGTH OF SELECTION */
	unsigned long	sel_index;					/* INDEX  OF SELECTION */
};

const struct PATH NEW_PATH =
{
	NULL, NULL,
	{0}, 1, 0, 0, 1, 0
};

#define scan_by_char(c,f,x) while((c = fgetc(f)) != EOF) {x}

#define wheel(k, n, x) if (++k == n) {k = 0; x}

static char VERBOSE = 0;
static char HURRY = 0;
static char FORCE = 0;

static Path* P_RUNNING;

int main(int argc, char * const argv[])
{

	if (argc < 2)
	{
		
		printf("\t\t#################################\n");
		printf("\t\t##############     ##############\n");
		printf("\t\t#########     #####     #########\n");
		printf("\t\t######   ###############   ######\n");
		printf("\t\t####   ###################   ####\n");
		printf("\t\t###  #######################  ###\n");
		printf("\t\t##  #####   ########====#####  ##\n");
		printf("\t\t#  #####     #####==    ===###  #\n");
		printf("\t\t# =#####     ####=         ==## #\n");
		printf("\t\t#+ =#####   ####=    +++     =# #\n");
		printf("\t\t#+  ==#########=    +++++     =+#\n");
		printf("\t\t#++   ===####==     +++++     ++#\n");
		printf("\t\t##++     ====        +++     ++##\n");
		printf("\t\t###++                       ++###\n");
		printf("\t\t####+++                   +++####\n");
		printf("\t\t######+++               +++######\n");
		printf("\t\t#########+++++     +++++#########\n");
		printf("\t\t##############+++++##############\n");
		printf("\t\t#################################\n");
		printf("\t\t###===========================###\n");
		printf("\t\t##|      D      A      O      |##\n");
		printf("\t\t###===========================###\n");
		printf("\t\t#################################\n");



		printf("\r\n[Welcome to DAOLANGUAGE UTILITY ver 0.0.0.1]\n");
		printf("\tPlease remember to enter a filename as a parameter, for example:\n\n");
		printf("\t\"> dao hello_world.dao\"\n");
		printf("\t\tto compile, or...\n\n");
		printf("\t\"> dao hello_world.wuwei\"\n");
		printf("\t\tto execute.\n");
		printf("\n");
		printf("Options:\n");
		printf("\t-v   :   Enable Verbose Execution (For Debugging)\n");
		printf("\t-h   :   Run Immediately After Compiling\n");
		printf("\t-f   :   Force Execution of Any File as DAOLANGUAGE (DANGEROUS)\n");
		printf("\n");
		printf("This implementation is not yet complete. Please wait kindly.\n\n");
		return(0);
	}
	else
	{
		String inputFileName = argv[1];
		File inputFile = fopen(inputFileName,"rb");
		
		int c;
		opterr = 0;
  		while ((c = getopt(argc, argv, "vhf")) != -1)
    		switch (c)
      		{
      		case 'v':
        		VERBOSE = 1;
        		break;
        	case 'h':
        		HURRY = 1;
        		break;
        	case 'f':
        		FORCE = 1;
        		break;
        	case '?':
        		printf("Unknown option -%c.\n\n", optopt);
        		break;
      		}

		if (inputFile == NULL)
		{
			printf("Could not find \"%s\" - is it in this directory?\n", inputFileName);
			fclose(inputFile);
			exit(1);
		}
		else 
		{
			if (~strcmp(FILE_SYMBOLIC, &inputFileName[strlen(inputFileName)-4]))
			{
				File outputFile;

				inputFileName[strlen(inputFileName)-4] = 0;
				inputFileName = strncat(inputFileName, FILE_COMPILED, sizeof(FILE_COMPILED));
				outputFile = fopen(inputFileName,"w+");

				if (VERBOSE)
					printf("\n%s%s\n", "Compiling symbolic dao to ", inputFileName);
				compile(inputFile, outputFile);
				if (VERBOSE)
					printf("Finished compiling.");

				fclose(inputFile);
				fclose(outputFile);

				if (HURRY)
				{
					inputFile = fopen(inputFileName,"rb");
					if (VERBOSE)
					{
						printf("\n\n");
						printf("\t=====================\n");
						printf("\t|Beginning Execution|\n");
						printf("\t=====================\n\n");
					}
				}
				else
				{
					return 0;
				}
			}
			if (FORCE || ~strcmp(FILE_COMPILED, &inputFileName[strlen(inputFileName)-6]))
			{
				int c;
				int i = 0;
				int shift = 0;
				int j = 0;
				int k = 0;
				
				struct PATH newPath = NEW_PATH;
				Path dao = &newPath;
	
				if (VERBOSE)
					printf("%s%s.\nLoading data:\n", "Running ", inputFileName);
	
				scan_by_char(c, inputFile,
					(dao -> prg_data)[i] |= ((unsigned long)c << ((3 - shift) * 8));
					wheel(shift, 4, i++;)
				)
				if (feof(inputFile))
				{
  					if (VERBOSE) printf("Hit end of file at position %x.\n\n", i*4 + shift);
				}
				else
				{
  					printf("Encountered an error during file read.\n");
  					return(22);
				}
	
				while (j < (i + ((shift + 3) / 4)))
				{
					if (VERBOSE)
					{
						printf("%x   ", (dao -> prg_data)[j]);
						wheel(k, 7, printf("\n");)
					}
					j++;
				}
				if (VERBOSE)
					printf("(%d bytes)\n\n", 4 * j);
				
				
	
				while ((dao -> prg_allocbits) / 32 < j)
					(dao -> prg_allocbits) *= 2;

				P_RUNNING = &dao;
				
				execs(dao, NULL);
			}
		}
	}


	exit(0)
}

void compile(File input, File output)
{
	
	boolean emptyBuffer = 1;
	unsigned char toWrite = 0;
	unsigned char isComment = 0;

	int ch;
	scan_by_char(ch, input,
		switch((char)ch)
		{
		case '\t':			case ' ':			break;
   		case '@': 			isComment = 1; 		break; 
   		case (char)0x0D:
   		case (char)0x0A: 	isComment = 0; 		break;

		default:
			if (!isComment)
			{
				if (VERBOSE)
					putchar((char)ch);
				if (!emptyBuffer)
				{
					toWrite |= getNybble((char)ch);
					fputc(toWrite, output);
					emptyBuffer = 1;
					if (VERBOSE)
						printf(" %x\n", toWrite);
				}
				else
				{
					toWrite = (char)(getNybble((char)ch) << 4);
					emptyBuffer = 0;
				}
			}
			break;
		}
	)
	if (!emptyBuffer) {
		fputc(toWrite, output);
		if (VERBOSE)
			printf(". %x\n", toWrite);
	}
}

/**
 * Run binary dao.
 */
void run(File input)
{
	
}

unsigned char getNybble(char ch)
{
   switch (ch) 
   {
		case '.': return IDLES;	case '!': return SWAPS;	case '/': return LATER; case ']': case ')': return MERGE;	
		case '%': return SIFTS;	case '#': return EXECS;	case '>': return DELEV;	case '=': return EQUAL;	
		case '(': return HALVE;	case '<': return UPLEV;	case ':': return READS; case 'S': return DEALC;	
		case '[': return SPLIT;	case '*': return POLAR;	case '$': return DOALC;	case ';': return INPUT;
		default: return IDLES;
	}
}

char getChar(unsigned char ch)
{
   switch (ch) 
   {
		case IDLES: return '.';	case SWAPS: return '!';	case LATER: return '/'; case MERGE: return ')';
		case SIFTS: return '%';	case EXECS: return '#';	case DELEV: return '>';	case EQUAL: return '=';
		case HALVE: return '(';	case UPLEV: return '<';	case READS: return ':'; case DEALC: return 'S';
		case SPLIT: return '[';	case POLAR: return '*';	case DOALC: return '$';	case INPUT: return ';';
		default: return '?';
	}
}

char* bin(unsigned long val)
{
	return itoa(val, 32, 2);
}

char* itoa(unsigned long val, unsigned char len, unsigned char radix)
{
	static char buf[32] = {'0'};
	int i = 33;
	for(; val && i ; --i, val /= radix)
		buf[i] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[val % radix];
	for(; i ; i--)
		buf[i] = '0';
	return &buf[2+(32-len)];
}




/*█████████████████████████████████████████████*/
/*████████████████████     ████████████████████*/
/*███████████████     █████     ███████████████*/
/*████████████   ███████████████   ████████████*/
/*██████████   ███████████████████   ██████████*/
/*█████████  ███████████████████████  █████████*/
/*████████  █████   ████████▒▒▒▒█████  ████████*/
/*███████  █████     █████▒▒░░░░▒▒▒███  ███████*/
/*███████ ▒█████     ████▒░░░░░░░░░▒▒██ ███████*/
/*███████▓░▒█████   ████▒░░░░▓▓▓░░░░░▒█ ███████ yet to do: level stuff, ascend and descend, execs replacement */
/*███████▓░░▒▒█████████▒░░░░▓▓▓▓▓░░░░░▒▓███████*/
/*███████▓▓░░░▒▒▒████▒▒░░░░░▓▓▓▓▓░░░░░▓▓███████*/
/*████████▓▓░░░░░▒▒▒▒░░░░░░░░▓▓▓░░░░░▓▓████████*/
/*█████████▓▓░░░░░░░░░░░░░░░░░░░░░░░▓▓█████████*/
/*██████████▓▓▓░░░░░░░░░░░░░░░░░░░▓▓▓██████████*/
/*████████████▓▓▓░░░░░░░░░░░░░░░▓▓▓████████████*/
/*███████████████▓▓▓▓▓░░░░░▓▓▓▓▓███████████████*/
/*████████████████████▓▓▓▓▓████████████████████*/
/*█████████████████████████████████████████████*/
/*████████╬═══════════════════════════╬████████*/
/*████████║      D      A      O      ║████████*/
/*████████╬═══════════════════════════╬████████*/
/*█████████████████████████████████████████████*/

#define CELL 32

#define levlim(l)		if (((*P_RUNNING) -> prg_level) >= l) {if(VERBOSE)printf(" LEV_SKIP");return;}
#define levconsk(l,x)	if (((*P_RUNNING) -> prg_level) < l) {x}
#define levcons(l,x,y)	if (((*P_RUNNING) -> prg_level) < l) {x} else {y}

#define intIndex 		( P_IND % CELL )
#define arrIndex 		( P_IND / CELL )

#define P_LEN 			(path -> sel_length)
#define P_IND 			(path -> sel_index)

#define P_ALC 			(path -> prg_allocbits)
#define P_LEV 			(path -> prg_level)
#define P_PIND			(path -> prg_index)
#define P_DATA 			(path -> prg_data)

#define P_OWNER			(path -> owner)
#define P_CHILD			(path -> child)

static Path* P_WRITTEN;
static unsigned int floor = 0;

void idles(Path path) {}

void swaps(Path path)
{
	levlim(2)
	/*
	 Get selection position
	 Get selection left
	 Get selection right
	 Write selection left to right
	 Write selection right to left
	*/
	if (P_LEN == 1)
	 	return;

	if (P_LEN < CELL)
	{
		int len = P_LEN;
		int shift = CELL - intIndex - len;
		long report = (P_DATA[arrIndex] >> shift) & mask(len);
		long left = report >> (len >> 1);
		long right = report & mask(len >> 1);
		long recombine = (right << (len >> 1)) | left;
		P_DATA[arrIndex] &= ~(mask(len) << shift);
		P_DATA[arrIndex] |= recombine << shift;
	}
	else
	{
		int i = 0;
		int leftIndex = arrIndex;
		int half = (P_LEN / CELL) / 2;
		long holder;
		while (i < half)
		{
			holder = P_DATA[leftIndex + i];
			P_DATA[leftIndex + i] = P_DATA[leftIndex + half + i];
			P_DATA[leftIndex + half + i] = holder;
			i++;
		}
	}
}

void later(Path path)
{
	if (aligned(path) || (((*P_RUNNING) -> prg_level) >= 4))
		P_IND += P_LEN;
	else
		merge(path);
}

void merge(Path path)
{
	levlim(7)
	if (P_LEN < P_ALC)
	{
		if (!aligned(path))
			P_IND -= P_LEN;
		P_LEN <<= 1;
	}
	else
	{
		if (P_OWNER != NULL)
			P_WRITTEN = &P_OWNER;
	}
}

void sifts(Path path)
{
	/*
	// Get current index
	// LOOP Linear nybble
		// If pos IDLES call this position L
		// Keep going until read non-idles. Call pos R.
		// LOOP Linear nybble as long as L still IDLES and R in bounds and not IDLES
			// Write R to L
			// Write IDLES to R
		*/
	int l = P_IND;
	levlim(5)
	while (l + 4 < P_ALC)
	{
		if (!report_by_bit_index(path, l, 4))
		{
			int r = l;
			if (VERBOSE) printf("IDLES (%s) encountered at index %d.\n", bin(report_by_bit_index(path, l, 4)), l);
			for (; report_by_bit_index(path, r, 4) && ((r + 4) < P_ALC); r++); /* Move right to first */
			for (;!report_by_bit_index(path, l, 4) && report_by_bit_index(path, r, 4) && ((r + 4) < P_ALC); l += 4, r += 4)
			{
				write_by_bit_index(path, l, 4, report_by_bit_index(path, r, 4));
				write_by_bit_index(path, r, 4, 0);
			}
		}
		l += 4;
	}
}

static unsigned char command = 0;

void execs(Path path, Path caller)
{
	unsigned long tempNum1 = 0;
	unsigned long tempNum2 = 0;
	unsigned long tempNum3 = 0;
	if ((path -> prg_level) >= 8) 
	{
		if (VERBOSE)
			printf(" LEV_SKIP");
		return;
	}
	floor++;
	P_RUNNING = &path;
	/* Probably also do the other cases...? Nah, not yet. Nibble execs is good 'nuff.*/
	if (P_CHILD == NULL)
		P_CHILD = (malloc(sizeof(struct PATH)));
	if (P_CHILD == NULL)
	{
		printf("FATAL ERROR: Unable to allocate memory.");
		return;
	}

	if (VERBOSE)
		printf("Allocated %d bytes to program.\n", sizeof(*P_CHILD));

	memcpy(P_CHILD, &NEW_PATH, sizeof(struct PATH));
	(*(*path).child).owner = path;
	P_WRITTEN = &P_CHILD;
	P_PIND = (P_IND / 4);

	/* Execs Loop */
	for (; P_PIND < (P_ALC / 4); P_PIND++)
	{
		/* Bug zone */
		if (VERBOSE)
		{
			/*
			printf("[%x]", command);
			printf("[%x]", P_RUNNING);
			printf("[%x]", &P_PIND);
			*/
			printf("[%x]", *P_RUNNING);
		}
		
		tempNum1 = ((*P_RUNNING) -> prg_index);
		tempNum2 = ((*P_RUNNING) -> prg_data)[(tempNum1*4) / 32];
		command = (tempNum2 >> (32 - ((tempNum1*4) % 32) - 4)) & mask(4);
/*
		command = report_by_bit_index((*P_RUNNING), tempNum1 * 4, 4);
		command = report_by_bit_index((*P_RUNNING), ((*P_RUNNING) -> prg_index) * 4, 4);
		command = (((((*P_RUNNING)) -> prg_data)[P_PIND / 8] >> (32 - (P_PIND % 8) - 4)) & mask(4));
		((*P_RUNNING)[i / 32] >> (32 - (tempNum1 % 8) - 4)) & mask(4);
*/

		if (VERBOSE)
			printf("c: %x", command);
		/* Bug zone */


		if (VERBOSE)
		{
			printf("[%x] ", *P_RUNNING);
			if ((int)(*P_RUNNING) == 0)
				printf("AAAAAAA");

			printf("pind: %s ", itoa(P_PIND, 5, 16));
			printf("[%x] ", *P_RUNNING);

			printf("F%x L%d ", floor, (*P_RUNNING) -> prg_level);
			printf("[%x] ", *P_RUNNING);
			printf("char: %c ", getChar(command));
			printf("[%x] ", *P_RUNNING);
			bin_print(P_CHILD);
			printf(" : ");
		}

		switch(command)
		{
			case IDLES: idles(*P_WRITTEN); break;
			case SWAPS: swaps(*P_WRITTEN); break;
			case LATER: later(*P_WRITTEN); break;
			case MERGE: merge(*P_WRITTEN); break;
			case SIFTS: sifts(*P_WRITTEN); break;
			case EXECS: execs(*P_WRITTEN, path); break;
			case DELEV: delev(*P_WRITTEN); break;
			case EQUAL: equal(*P_WRITTEN); break;
			case HALVE: halve(*P_WRITTEN); break;
			case UPLEV: uplev(*P_WRITTEN); break;
			case READS: reads(*P_WRITTEN); break;
			case DEALC: dealc(*P_WRITTEN); break;
			case SPLIT: split(*P_WRITTEN); break;
			case POLAR: polar(*P_WRITTEN); break;
			case DOALC: doalc(*P_WRITTEN); break;
			case INPUT: input(*P_WRITTEN); break;
		}
		if (VERBOSE)
			printf("\n");
	}

	P_RUNNING = &caller;

	floor--;
	return;
}

void delev(Path path)
{
	if (((*P_RUNNING) -> prg_level) > 0)
		((*P_RUNNING) -> prg_level)--;
}

void equal(Path path)
{
	levlim(5)
	/* If leftmost and rightmost of selection are not equal then skip data pointer */
	if (report_by_bit_index(path, P_IND, 1) ^ report_by_bit_index(path, P_IND + P_LEN - 1, 1))
	{
		if (P_RUNNING != NULL)
			skip();
	}
	else
		if (VERBOSE)
			printf("EQUAL");
}

void halve(Path path)
{
	levlim(7)
	if (P_LEN == 1)
	{
		if (P_CHILD != NULL)
		{
			P_WRITTEN = &P_CHILD;
			((*P_WRITTEN) -> sel_length) = ((*P_WRITTEN) -> prg_allocbits);
		}
		return;
	}

	(P_LEN) /= 2;
}

void uplev(Path path)
{
	levcons(9, ((*P_RUNNING) -> prg_level)++;, ((*P_RUNNING) -> prg_level)--;)
	((*P_RUNNING) -> prg_index) = 0;
}

void reads(Path path)
{
	levlim(6)
	if (P_LEN == 8 || P_LEN == 16)
		putchar(report_by_bit_index(path, P_IND, P_LEN));
	else if (P_LEN == 32)
		printf("%c%c", report_by_bit_index(path, P_IND, 16), report_by_bit_index(path, P_IND + 16, 16));
	else if (P_LEN <= CELL)
	{
		String out = bin(report_by_bit_index(path, P_IND, P_LEN));
		printf("%s", &out[strlen(out) - P_LEN]);
	}
	else
	{
		long pos = P_IND;
		for (; pos < (P_IND + P_LEN); pos += 16)
			putchar(report_by_bit_index(path, pos, 16));
	}
} 

void dealc(Path path)
{
	levlim(2)
	if (P_ALC == 1)
	{
		unsigned char report = report_by_bit_index(path, 0, 1);
		if (((*P_RUNNING) -> owner) != NULL)
		{
			unsigned long ownind = (((*P_RUNNING) -> owner) -> prg_index);
			if (VERBOSE)
			{
				printf("Terminating program from position %x with value %x",\
					ownind, report);
			}
			/* Terminate - overwrite the EXECS in the P_RUNNING */
			if (report)
				write_by_bit_index(*P_RUNNING, ownind, 4, SWAPS);
			else
				write_by_bit_index(*P_RUNNING, ownind, 4, IDLES);
		}
		/* Kick out of caller */
		((*P_RUNNING) -> prg_index) = ((*P_RUNNING) -> prg_allocbits) + 1;
		return;
	}
	/* Halve prg_alloc and sel_len, or terminate */
	P_ALC >>= 1;
	halve(path);
	if ((P_IND + P_LEN) >= P_ALC)
		P_IND -= P_ALC;
}

void split(Path path)
{
	/*
		Get selection
		Selection left  |= MASK(LEN/2)
		Selection right &= MASK(LEN/2)
		Halve
	*/
	if (((*P_RUNNING) -> prg_level) < 1)
	{
		unsigned int len = P_LEN;
	
		if (len == 1)
		{
			if (P_CHILD != NULL)
			{
				P_WRITTEN = &P_CHILD;
				((*P_WRITTEN) -> sel_length) = ((*P_WRITTEN) -> prg_allocbits);
				split(*P_WRITTEN);
				halve(*P_WRITTEN);
			}
			return;
		}
		
		if (len <= CELL)
		{
			unsigned int shift = CELL - intIndex - len;
			unsigned long polarized = (mask(len >> 1) << (len >> 1));
			P_DATA[arrIndex] &= ~(mask(len) << shift);
			P_DATA[arrIndex] |= polarized << shift;
		}
		else
		{
			int leftIndex = arrIndex;
			int rightIndex = leftIndex + (len / CELL) - 1;
			while (leftIndex < rightIndex)
			{
				P_DATA[leftIndex++] = 0xFFFFFFFF;
				P_DATA[rightIndex--] = 0;
			}
		}
	}
	halve(path);
		
}

void polar(Path path)
{
	levlim(3)
	/* If leftmost and rightmost of selection are polar then don't skip data pointer */
	if (!(report_by_bit_index(path, P_IND, 1) && !report_by_bit_index(path, P_IND + P_LEN - 1, 1)))
	{
		if (P_RUNNING != NULL)
			skip();
	}
	else
		if (VERBOSE)
			printf("POLAR");
}

void doalc(Path path)
{
	levlim(1)
	/* If it's still below MAX_BIT_SIZE then double length and alloc */
	if (P_ALC < MAX_BIT_SIZE) {
		P_ALC <<= 1;
		merge(path);
	}
	else
	{
		printf("FATAL ERROR 1: Program exceeded maximum memory.");
		abort();
	}
}

void input(Path path)
{
	levlim(6)
	write_by_bit_index(path, P_IND, P_LEN, getchar());
}

/*▄▄
 ▐███▄
  ▀████▄
    ▀████▄
       ▀███▄▄
          ▀█████▄▄
              ▀▀████▄▄▄
                  ▀▀████████▄▄
                         ▀▀▀█████▄▄
                               ▀▀████▄
                                    ▀███▄▄
                                       ▀████▄
                                         ▀████▄	
                                           ▀███▌
                                             ▀▀*/

boolean aligned(Path path)
{
	return P_IND % (P_LEN << 1) == 0;
}

unsigned long mask(int length) 
{
	if (length < CELL)	return (char)((int)1 << length) - 1;
	else			 	return 0xFFFFFFFF;
} 

unsigned long report_by_bit_index(Path path, unsigned int i, unsigned int len)
{
	return (P_DATA[i / CELL] >> (CELL - (i % CELL) - len)) & mask(len);
}

void write_by_bit_index(Path path, unsigned int i, unsigned int len, unsigned long write)
{
	int shift = CELL - intIndex - len;
	write &= mask(len);
	P_DATA[i / CELL] &= ~(mask(len) << shift);
	P_DATA[i / CELL] |= (write << shift);
}

void bin_print(Path path)
{
	if (P_ALC <= CELL)
	{
		String out = bin(report_by_bit_index(path, 0, P_ALC));
		printf("%s", &out[strlen(out) - P_ALC]);
	}
	else
	{
		long i = 0;
		for (; i < (P_ALC / CELL); i++)
			printf("%s", bin(P_DATA[i]));
	}
}

void todo()
{
	printf("!!! NOT IMPLEMENTED YET !!!\n");
	return;
}

void skip()
{
	if (VERBOSE)
		printf("%s ", "SKIP");
	((*P_RUNNING) -> prg_index)++;
}