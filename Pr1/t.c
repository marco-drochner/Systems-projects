#include<stdio.h>
#include <stdlib.h>
int rempwr2(int x, int n);
unsigned float_half(unsigned uf);
int main(int argc, char const *argv[])
{
  // unsigned x = ~0-2;
  // int f = x;
  // printf("%d\n", f);
  int arg1 = atoi(argv[1]);
  int arg2 = atoi(argv[2]);
	printf("%d\n", rempwr2(arg1,arg2));
	return 0;
}
int rempwr2(int x, int n) {

// if ((x == (1<<31)) || (x==~(1<<31)))
//     return x%(int)pow(2,n);  
  
  // convert to 1's complement
  int xSign = x >> 31;
  int from_1s_to_2s_comp = 1 & (xSign);
                      // = 1 if x is neg, 0 if positive
  int from_2s_to_1s_comp = 1+~from_1s_to_2s_comp;
                      // = -from_1s_to_2s_comp
  int onesCompX = x + from_2s_to_1s_comp;
  int zeroOut = ~((~0) << n);
  int replaceWithSign = (xSign) << n;
  int onesCompRes = (onesCompX & zeroOut) | replaceWithSign;
  return onesCompRes + from_1s_to_2s_comp;
}
