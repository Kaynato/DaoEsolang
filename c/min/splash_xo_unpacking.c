#include 								 <stdio.h>
int var1 = 0;
int i;
int n = 0;
int m = 0;
int x = 64;
int index = 3;
int var2;

int main()
{
	// Define the data for the ASCII to be printed
	char* data = "$(*.)SS-*E.)(/.8'7*1).+=.,$/,E)&(/0.\
	'(2.-)(,.,&-/..$**2,'(,.+&-/.-$)*7+&(+.+%(+/.'(..\
	*$,*1.%)*/)&)*0+-(+.-$-/0)&**1,((,..$-/0)'**3-$0*\
	1.&(/0);**5,$;-.3'(91)3-*3.$?/.[?(6.)+(*5)$5E.MK";

	// Define the index
	x /= 10;
	index = x;
	x *= 5;
	var2 = 210 + data[x];
	x += 2;

	// Loop over data payload of data string
	for (i=0; index<var2; index++)
	{
		// Advance index until index-36 is divisible by 50
		// Basically we do this to SKIP THE POSITION OF THE TAB CHARACTER!!!!
		while(!((index++ - 36) % 50));
		index--;

		// Load the data char from the datastring into memory for printing X chars
		// An offset to the value is calculated with offsets in the beginning of the string
		for(n=m + data[index] - data[i++%5]; m<n; m++)
		{
			if(!(m++%33))
				for(var1=var2-3+!(m-1); var2-var1; var2--)
					putchar(9+(var2-var1-1)/2);

			m--;
			
			// If m-669 is divisible by 8 and in the [668, 686] exclusive range
			// do the thing
			if( (!((m-669) % 8)) && (m<686) && (m>668) )
			{
				// This is an integer equation that converts the
				// Possible values of m that fulfill the above conditoin
				// Into the ASCII codes for D A O
				// Made this with Wolfram Mathematica
				m++;
				putchar((m*1189-811862)/(m*17-11616+2));
			}
			else
				putchar(x);
		}
		// Cycle the ASCII code between ' ', '=', '#'
		// Made this using truth tables [pain sounds]
		x ^= (x&2?0:'8'/2) | ((x&(2+1)) + ((x&2)!=2));
	}
	// I think this is supposed to become 184 when we arrive at this point
	var2--;
	return var2 - 183;
}





	/***************** -O3 friendly *****************/
	/*
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
	printf("\t\t                                 \n");
	printf("\t\t   ===========================   \n");
	printf("\t\t         D      A      O         \n");
	printf("\t\t   ===========================   \n");
	printf("\t\t                                 \n");
	*/