#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
unsigned   int   sqrt_16(unsigned   long   M)  
{  
	unsigned   int   N,   i;  
	unsigned   long   tmp,   ttp;       //   结果、循环计数  
	if   (M   ==   0)                               //   被开方数，开方结果也为0  
		return   0;  

	N   =   0;  

	tmp   =   (M   >>   30);                     //   获取最高位：B[m-1]  
	M   <<=   2;  
	if   (tmp   >   1)                             //   最高位为1  
	{  
		N   ++;                                   //   结果当前位为1，否则为默认的0  
		tmp   -=   N;  
	}  

	for   (i=15;   i> 0;   i--)             //   求剩余的15位  
	{  
		N   <<=   1;                             //   左移一位  

		tmp   <<=   2;  
		tmp   +=   (M   >>   30);           //   假设  

		ttp   =   N;  
		ttp   =   (ttp <<1)+1;  

		M   <<=   2;  
		if   (tmp   >=   ttp)               //   假设成立  
		{  
			tmp   -=   ttp;  
			N++;  
		}  

	}  

	return   N;  
} 

int main(int argc, char *argv[])
{
	int i;

	i = atoi(argv[1]);
	
	printf("%f", sqrt_16(i*100)/10.0);
}
