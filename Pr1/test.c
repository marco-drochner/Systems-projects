#include <stdio.h>
unsigned float_abs(unsigned uf) ;
int main(int argc, char const *argv[])
{
        int it = -4194304;
        unsigned res = float_abs(it);
        printf("%u\n\n",res);

        // printf("it: %u\n", it);
        return 0;
}
unsigned float_abs(unsigned uf) {
  unsigned absol = uf & (-1^(1<<31));
  unsigned mask = -1; mask = mask
                    >>10;
  unsigned is_sigbit1_iff_NaN = (!((uf|mask) ^ -1))<<31;
  return absol | is_sigbit1_iff_NaN;
}
/*
gcc test.c -o test
./test

*/