#include <stdio.h>
#include <stdlib.h>

unsigned long ptwo_round(unsigned long);

int main()
{
/*
	char* in = "1203912303!3929399";

	char* code = NULL;
	char* input = NULL;*/

	unsigned long num = 7;

	printf("%d\n", ptwo_round(num));



	return 0;

	if (0) {
		char   ch    = 'A';
		char*  chptr = NULL;
		char** chpt2 = NULL;

		printf("ch is %c\n", ch);
		printf("chptr points to %x\n", chptr);

		chptr = &ch;

		printf("chptr points to %x containing %c\n", chptr, *chptr);

		chpt2 = &chptr;

		printf("chpt2 points to %x containing %x containing %c\n", chpt2, *chpt2, **chpt2);


		chptr = "STRING";
		/* S T R I N G */
		/* 0 1 2 3 4 5 */

		printf("chptr points to %x containing ", chptr);
		putchar(*(chptr));
		putchar(*(chptr + 1));
		putchar(*(chptr + 2));
		putchar(*(chptr + 3));
		putchar(*(chptr + 4));
		putchar(*(chptr + 5));

		printf("\nchpt2 points to chptr containing ");
		putchar(*(*chpt2));
		putchar(*(*chpt2 + 1));
		putchar(*(*chpt2 + 2));
		putchar(*(*chpt2 + 3));
		putchar(*(*chpt2 + 4));
		putchar(*(*chpt2 + 5));
		putchar('\n');

		printf("\n");
		printf("\n");
		printf("\n");

		printf("%d\n", -0 / 2);
	}

	return 0;

	if (0)
	{
		int* ptr = NULL;
		int  x   = 0;
		printf("%d at %x\n", x, &x);
		x++;
		printf("%d at %x\n", x, &x);

		printf("ptr points to %x\n", ptr);
		ptr = &x;
		printf("ptr points to %x\n", ptr);
		printf("ptr points to %x containing %d\n", ptr, *ptr);

		x = 1203;
		printf("ptr points to %x containing %d\n", ptr, *ptr);

		ptr = (int*)x;
		printf("ptr points to %x\n", ptr);
	}




	return 0;

	printf("\n\n\n");
	printf("\tZ:\\>SUGGESTIONS -daily -no_arguments -forcible\n");
	printf("\t\tModule daily_update.fa1 loaded\n");
	printf("\t\tModule central_construct.fa1 loaded\n");
	printf("\t\tUnexpected error occurred in intent_parse.dll\n");
	printf("\t\tUnexpected error occurred in expect_input.dll\n");
	printf("\t\tUnexpected error occurred in FILENAME_MALFORMED_ERROR\n");
	printf("\t\tUnexpected error occurred in intent_parse_v2.dll\n");
	printf("\t\tUnexpected error occurred in select_input.dll\n");
	printf("\t\tUnknown object THIS_ERROR_WILL_NEVER_OCCUR_ANYWAY encountered at line 23932 of FILENAME_MALFORMED_ERROR\n");
	printf("\t\tUnexpected error occurred during loading\n");
	printf("\t\tUnexpected error occurred during loading\n");
	printf("\t\tTerminate?\n\t\tY / NULL_OPTION_ERROR\n");
	printf("\n\t>");
	getchar();
	return 0;
}



unsigned long ptwo_round(unsigned long x)
{
	unsigned long rounded = x;										/* Initialize bytes_alloc with the file_size value. */
	unsigned long	shift = 0;										/* Shift for first one of file size for rounding    */

	while ((rounded >> 1) != 0)										/* Determine leftmost '1' bit of file size.		 	*/
	{																/*													*/
		rounded >>= 1;												/* Shift right until the next shift zeroes it.		*/
		shift++;													/* Keep track of shifts.							*/
	}																/*													*/
	rounded <<= shift; 												/* Unshift. 										*/
	if (x != rounded)												/* If not a power of two, round up.					*/
		rounded <<= 1;

	return rounded;
}