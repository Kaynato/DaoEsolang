#include <stdio.h>

int DEBUG = 0;

int main()
{
	int x = ' ';

	int inc = 0;
	long j = 0;
	int lim = 0;
	int rog_ind = 6;
	int rog_len = 181 + 23*3;

	int nlcheck = 0;

	int TRACKER_1 = 0;
	char AMTCHAR = '0';
	char SUBCHAR = '0';

	char* rog = "$(*.)SS-*E.)(/.8'7*1).+=.,$/,E)&(/0.'(2.-)(,.,&-/..$**2,'(,.+&-/.-$)*7+&(+.+%(+/.'(..*$,*1.%)*/)&)*0+-(+.-$-/0)&**1,((,..$-/0)'**3-$0*1.&(/0);**5,$;-.3'(91)3-*3.$?/.[?(6.)+(*5)$5E.M";

	for (; rog_ind < rog_len; rog_ind++)
	{
		lim = j + (rog[rog_ind] - rog[inc%5]);

		SUBCHAR = rog[j%5];
		AMTCHAR = rog[rog_ind];
		
		inc++;
		
		for(; j < lim; j++)
		{
			if (!(j%33) && !DEBUG)
			{
				nlcheck = rog_len - 3;
				for (; rog_len > nlcheck; rog_len--)
					putchar(9+(rog_len-nlcheck-1)/2);
			}

			if (((j - 669) % 7 == 0) && (j < 686) && (j > 668))
				putchar((++j*1189-809958)/(j*17-11586));
			if (!DEBUG) putchar(x);
			
			TRACKER_1++;
		}
		if (DEBUG)
			printf("%c\t%d\t\t%d\t%d\t\tindex %d\n", x, TRACKER_1, SUBCHAR, AMTCHAR, j);
		TRACKER_1 = 0;

		x ^= 0x1C-0x0E*(x&2);
		x ^= (x&3) + ((x&2) != 2);
	}

	return 0;

		printf("\t\t                                 \n");
		printf("\t\t              =====              \n");
		printf("\t\t         =====#####=====         \n");
		printf("\t\t      ===###############===      \n");
		printf("\t\t    ===###################===    \n");
		printf("\t\t   ==#######################==   \n");
		printf("\t\t  ==#####   ########====#####==  \n");
		printf("\t\t ==#####     #####==    ===###== \n");
		printf("\t\t ==#####     ####=         ==##= \n");
		printf("\t\t = =#####   ####=    ###     =#= \n");
		printf("\t\t =  ==#########=    #####     == \n");
		printf("\t\t ==   ===####==     #####     == \n");
		printf("\t\t  ==     ====        ###     ==  \n");
		printf("\t\t   ==                       ==   \n");
		printf("\t\t    ===                   ===    \n");
		printf("\t\t      ===               ===      \n");
		printf("\t\t         =====     =====         \n");
		printf("\t\t              =====              \n");
		printf("\t\t                                 \n"); /*669*/
		printf("\t\t   ===========================   \n");
		printf("\t\t         D      A      O         \n");
		printf("\t\t   ===========================   \n");
		printf("\t\t                                 \n");

	return 0;
}