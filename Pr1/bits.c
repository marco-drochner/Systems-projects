/* 
 * CS:APP Data Lab 
 * 
 * Marco Drochner marcodrochner
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */
// #include <stdio.h>
// int main(int argc, char const *argv[])
// {
//   unsigned it = ~1;
//   int toP = it;
//   printf("\n\n %d \n", toP);
//   return 0;
// }

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 10.0.0.  Version 10.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2017, fifth edition, plus
   the following additions from Amendment 1 to the fifth edition:
   - 56 emoji characters
   - 285 hentaigana
   - 3 additional Zanabazar Square characters */
/* 
 * evenBits - return word with all even-numbered bits set to 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 1
 */
int evenBits(void) {
  int _01010101_01010101 = 85 + (85<<8);
  return (_01010101_01010101 << 16) + _01010101_01010101;
}

/* 
 * bitNor - ~(x|y) using only ~ and & 
 *   Example: bitNor(0x6, 0x5) = 0xFFFFFFF8
 *   Legal ops: ~ &
 *   Max ops: 8
 *   Rating: 1
 */
int bitNor(int x, int y) {
  return (~x) & (~y);
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
        return (x >> (n << 3)) & 255;
}
/* 
 * tc2sm - Convert from two's complement to sign-magnitude 
 *   where the MSB is the sign bit
 *   You can assume that x > TMin
 *   Example: tc2sm(-5) = 0x80000005.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 4
 */
int tc2sm(int x) {
  int flippedOnlyIfNeg = ((x >> 31) ^ x);
  int add1ifNeg = (x >> 31) & 1;
  int sign = 1 << 31;
  int itsSign = sign & (x >> 31);
  // printf("\nMath:   x = %d", x);
  // printf("\n\nThis be it: %d\n", (((x >> 32) )));
  return flippedOnlyIfNeg + add1ifNeg + itsSign;
}

/* 
 * rotateLeft - Rotate x to the left by n
 *   Can assume that 0 <= n <= 31
 *   Examples: rotateLeft(0x87654321,4) = 0x76543218
 *   Legal ops: ~ & ^ | + << >> !
 *   Max ops: 25
 *   Rating: 3 
 */
int rotateLeft(int x, int n) {
  int sig;
  int negativeN;
  int maskToCancelIfNIs0;
  int leading1sTo0;
  sig = 1<<31;
  negativeN = (~n) + 1;

  maskToCancelIfNIs0 = !(!n) << 31;
  maskToCancelIfNIs0 = maskToCancelIfNIs0 >> 31;
  leading1sTo0 = ~((sig >> (32 + negativeN)) << 1);
  return (x << n) + (((x >> (32 + negativeN)) & leading1sTo0) & maskToCancelIfNIs0);

}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
  int mask;
  int _01010101_01010101 = 85 + (85<<8);
  mask = ((_01010101_01010101 << 16) + _01010101_01010101);
  x = (x&mask) + ((x>>1)&mask);
  //00110011
  mask = 51 + (51<<8);
  mask = mask + (mask<<16);
  x = (x&mask) + ((x>>2)&mask);
  //00001111
  mask = 15 + (15<<8);
  mask = mask + (mask<<16);
  x = (x&mask) + ((x>>4)&mask);
  //00000000 11111111
  mask = 255 + (255<<16);
  x = (x&mask) + ((x>>8)&mask);
  //00000000 00000000 11111111 11111111
  mask = 255 + (255<<8);
  x = (x&mask) + ((x>>16)&mask);
  return x;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return 1<<31;
}
/* 
 * sign - return 1 if positive, 0 if zero, and -1 if negative
 *  Examples: sign(130) = 1
 *            sign(-23) = -1
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 10
 *  Rating: 2
 */
int sign(int x) {
    // for 0
  int zeroOrNo = !(!x) << 31;
  int all1orAll0 = zeroOrNo >> 31;

  int allTheSB = x >> 31;

  return (allTheSB + !allTheSB) & all1orAll0;
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
  int sig = 1<<31;
  int xSigBit = x&sig;
  int allBitsXSig = xSigBit>>31;
  int neg1 = ~0;
  return !((x >> (n + neg1))^allBitsXSig);
}
/* 
 * isGreater - if x > y  then return 1, else return 0 
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {
  // basically, do y - x, and if it's negative, x is greater.
  // But because of overflow, cut the ints into left and right
  // parts. Give greater weight to the left part (weight of 2,
  // used to make a baseline of 1 negative implying x>y,
  // i.e. (1 - weight) = 1 - 2 = negative dif  or it = 1 - 0 = not negative dif).
  // But what if the left hand sides are equal? i.e. 1 - 0 = not negative
  // even if the smaller righthand side of x is greater?
  // Well, the right hand sides are subtracted (tinyRight_Y - tinyRight_X)
  // and given weight 1 (instead of 2, so that if y is actually greater,
  // the right hand's weight of 1 alone is not enough to make the baseline 1 negative,
  // but if x and y are equal, another 1 is subtracted, i.e. if x and y's lefthand side
  // are equal, only then will the weight of 1 of the right hand side make it negative
  // to imply x > y
  int negativeRX;
  int negIfRightGreater;
  int rightIsGreater;
  int leftDifference;
  int mask = ~(1<<30);
  int leftX = (x >> 2) & mask;
  int leftY = (y >> 2) & mask;
  int negativeLX = ~leftX + 1;
  int negIfLeftXGreater = (leftY + negativeLX);
  int leftIsGreater = (negIfLeftXGreater >> 30)&2;

  int xIsGreaterIfThisNotNegative =
    (~1)/*i.e.-2 */ + leftIsGreater;


  int tinyRight_X = (x & 3);
  int tinyRight_Y = (y & 3);
  negativeRX = ~tinyRight_X + 1; 
  negIfRightGreater = tinyRight_Y + negativeRX;
  rightIsGreater = (negIfRightGreater >> 31) & 1;

  leftDifference = negIfLeftXGreater;
  xIsGreaterIfThisNotNegative = xIsGreaterIfThisNotNegative
    + rightIsGreater + !leftDifference;
    
  return !(xIsGreaterIfThisNotNegative >> 31);
}
/* 
 * rempwr2 - Compute x%(2^n), for 0 <= n <= 30
 *   Negative arguments should yield negative remainders
 *   Examples: rempwr2(15,2) = 3, rempwr2(-35,3) = -3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int rempwr2(int x, int n) {
  int xSign = x >> 31;

  // convert to 1's complement
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
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2(int x) {
  return 2;
}
/* 
 * float_abs - Return bit-level equivalent of absolute value of f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
m *   single-precision floating point values.
 *   When argument is NaN, return argument..
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_abs(unsigned uf) {
  int absol;
  int sig = 1<<31;
  int nanPart = 255<<23;
  if (((uf & nanPart) == nanPart)
      && (uf << 9))
    return uf;
  absol = uf & (~sig);
  return absol;
}
/* 
 * float_half - Return bit-level equivalent of expression 0.5*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_half(unsigned uf) {
  // if(8388607==uf) return 0;
  // int signedUF = 0 | uf;
  int firstBitOfExp = (1<<23);
  // int expAndRightNeighbor;
  int nanOrInf = 255<<23;
  int exponent = uf & nanOrInf;


  int sig = 1<<31;
  if ((uf&~sig)) {} else{
    return uf;
  }
  
  if (exponent == nanOrInf)
    return uf;
  if (exponent&~firstBitOfExp) { // normal
    return uf - firstBitOfExp;
  } else {                     //denormalized or about to be
    // expAndRightNeighbor = nanOrInf | (nanOrInf>>1);
    int sf = uf;
    unsigned floatShifted = ((sf>>1)&~nanOrInf);//

    floatShifted += (floatShifted & 1)&uf;
    return floatShifted;
    
  }
}
