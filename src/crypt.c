/*
 *
 * Copyright (C) 1991, 1992, 1993 Michael Glad.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Very basic crypt(3) implementation straight after
 * das buch.
 *
 */

/* #define DEBUG      */
/* #define BENCHMARK  */
/* #define STANDALONE */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// char *strcpy();

static unsigned char keys[16][48];

static unsigned char bytemask[8] =
  { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

static unsigned char ip[64] = 
  { 58, 50, 42, 34, 26, 18, 10,  2, 60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14,  6, 64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17,  9,  1, 59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13,  5, 63, 55, 47, 39, 31, 23, 15, 7
  };

static unsigned char ip_1[64] =
  { 40,  8, 48, 16, 56, 24, 64, 32, 39,  7, 47, 15, 55, 23, 63, 31,
    38,  6, 46, 14, 54, 22, 62, 30, 37,  5, 45, 13, 53, 21, 61, 29,
    36,  4, 44, 12, 52, 20, 60, 28, 35,  3, 43, 11, 51, 19, 59, 27,
    34,  2, 42, 10, 50, 18, 58, 26, 33,  1, 41,  9, 49, 17, 57, 25
  };

static unsigned char pc_1[56] =
  { 57, 49, 41, 33, 25, 17,  9,  1, 58, 50, 42, 34, 26, 18,
    10,  2, 59, 51, 43, 35, 27, 19, 11,  3, 60, 52, 44, 36,
    63, 55, 47, 39, 31, 23, 15,  7, 62, 54, 46, 38, 30, 22,
    14,  6, 61, 53, 45, 37, 29, 21, 13,  5, 28, 20, 12,  4
  };

static unsigned char pc_2[48] =
  { 14, 17, 11, 24,  1,  5,  3, 28, 15,  6, 21, 10,
    23, 19, 12,  4, 26,  8, 16,  7, 27, 20, 13,  2,
    41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32
  };

static unsigned char ls[16] =
  { 1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1 };

static unsigned char eref[48] =
  { 32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,
     8,  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
    16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
    24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32,  1
  };
static unsigned char e[48];

static unsigned char p[32] = 
  { 16,  7, 20, 21, 29, 12, 28, 17,  1, 15, 23, 26,  5, 18, 31, 10,
     2,  8, 24, 14, 32, 27,  3,  9, 19, 13, 30,  6, 22, 11,  4, 25
  };

static unsigned char sel[8][4][16]=
      { { { 14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7 },
          {  0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8 },
          {  4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0 },
          { 15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13 }
        },

        { { 15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10 },
          {  3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5 },
          {  0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15 },
          { 13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9 }
        },

        { { 10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8 },
          { 13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1 },
          { 13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7 },
          {  1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12 }
        },

        { {  7,  13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15 },
          { 13,  8,  11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9 },
          { 10,  6,   9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4 },
          {  3, 15,   0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14 }
        },

        { {  2, 12,   4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9 },
          { 14, 11,   2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6 },
          {  4,  2,    1, 11, 10, 13,  7,  8, 15, 9, 12,  5,  6,  3,  0, 14 },
          { 11,  8,  12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3 }
        },

        { { 12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11 },
          { 10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8 },
          {  9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6 },
          {  4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13 }
        },

        { {  4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1 },
          { 13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6 },
          {  1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2 },
          {  6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12 }
        },

        { { 13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7 },
          {  1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2 },
          {  7, 11, 4,   1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8 },
          {  2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11 }
        }
      };

#define ascii_to_bin(c) (c>='a'?(c-59):c>='A'?(c-53):c-'.')
#define bin_to_ascii(c) (c>=38?(c-38+'a'):c>=12?(c-12+'A'):c+'.')

static void pr_bits(v,n)
  unsigned char *v;
  int n;
  { int i,j;
    n/=8;
    for(i = 0; i < n; i++)
      { int outp = 0;
        for(j = 0; j < 8; j++)
          outp |= v[8 * i+j]?bytemask[j%8]:0;
        printf("%02x ",outp);
      }
  }

static void pre_bits(v,n)
  unsigned char *v;
  int n;
  { int i,j;
    n/=8;
    for(i = 0; i < n; i++)
      { int outp = 0;
        for(j = 0; j < 8; j++)
          outp |= v[eref[8 * i+j] - 1]?bytemask[j%8]:0;
        printf("%02x ",outp);
      }
  }

void encrypt(),setkey();
static void f(),s(),permute(),xor();

char *crypt(k,salt)
  char *k,*salt;
  { int i,j;
    char c/*,key_c[9]*/;
    unsigned char key[64];
    unsigned char bits[66];
    static char outbuf[14];

    bzero(key,sizeof key);
    for(i = 0; (c = k[i] << 1) && i < 8; i++)
      for(j = 8; j--;)
        key[i * 8+j] = (c & bytemask[j])?1:0;
    setkey((char*)key);

    for(i = 0; i < 2; i++)
      { c = ascii_to_bin(salt[i]);
        for(j = 0; j < 6; j++)
          if((c >> j) & 0x1)
            { unsigned char t;
              t = e[6 * i+j];
              e[6*i+j] = e[6*i+j+24];
              e[6*i+j+24] = t;
            }
      }

    bzero((char*)bits,sizeof(bits));
    for(i = 25; i--;)
      encrypt((char*)bits,0);

#ifdef DEBUG
    pr_bits(bits,64); printf("\n");
#endif

    outbuf[0] = salt[0];
    outbuf[1] = salt[1];
    for(i = 0; i < 11; i++)
      { unsigned char tmp = 0;
        for(j = 0; j < 6; j++)
          tmp |= bits[6*i+j] << (5 - j);
        outbuf[i+2] = bin_to_ascii(tmp);
      }
    outbuf[i+2] = 0;

    return outbuf;

  }

#ifdef STANDALONE

main()
  { char *s;
    char buf[64];

#ifdef BENCHMARK
    int i;
    for(i = 1000; i--;)
#endif
      s = crypt("ABC$#","zI");
    printf("%s\n",s);

#ifdef DEBUG
    bcopy(eref,e,sizeof(eref));
    bzero(buf,64);
    encrypt(buf,0);
    pr_bits(buf,64); printf("\n");
#endif

  }

#endif
  
void encrypt(inbit,direction)
  char *inbit;
  int direction;
  { unsigned char temp[64];
    unsigned char lr[32*19];
    int i,j;

    permute(64,ip,(unsigned char*)inbit,lr);
    for(i = 0; i < 16; i++)
      { f(lr+(i+1)*32,keys[direction?15 - i:i],temp);
#ifdef DEBUG
        pr_bits(lr+(i+1)*32,32); printf("-> "); pr_bits(temp,32); printf("\n");
#endif
        xor(32,lr+i*32,temp,lr+(i+2)*32);
      }
    for(j = 0; j < 32; j++)
      lr[18*32+j] = lr[16*32+j];
    permute(64,ip_1,lr+17*32,(unsigned char*)inbit);
  }

void setkey(key)
  char *key;
  { int i,j;
    unsigned char key_halves[17][56];

    permute(56,pc_1,(unsigned char*)key,key_halves[0]);
    for(i = 1; i<= 16; i++)
      { for(j = 0; j < 28; j++)
          { key_halves[i][j]    = key_halves[i - 1][(j+ls[i - 1])%28];
            key_halves[i][j+28] = key_halves[i - 1][(j+28+ls[i - 1])%28+28];
          }
        permute(48,pc_2,key_halves[i],keys[i - 1]);
      }
    bcopy((char*)eref,(char*)e,sizeof(eref));
  }

static void f(r,key,out)
  unsigned char *r,*key,*out;
  { unsigned char e_r[48],bs[48],sbs[32];
    int i;
     permute(48,e,r,e_r);
     xor(48,e_r,key,bs);
     for(i = 0; i < 8; i++) 
       s(bs+i*6,sbs+i*4,i);
     permute(32,p,sbs,out);
   }

static void s(b,ps,i)
  unsigned char *b,*ps;
  int i;
  { int val,j;

    val = sel[i][b[0]*2+b[5]][b[1]*8+b[2]*4+b[3]*2+b[4]];
    for(j = 4; j--; )
      { ps[j] = val&0x1;
        val >>= 1;
      }
  }

static void permute(count,perm,in,out)
  int count;
  unsigned char *perm;
  unsigned char *in,*out;
  { int i;
    for(i = 0; i < count; i++)
      out[i] = in[perm[i] - 1];
  }

static void xor(count,in1,in2,out)
  int count;
  unsigned char *in1,*in2,*out;
  { int i;
    for(i = 0; i < count; i++) 
      out[i] = (in1[i] ^ in2[i]) & 0x1;
  }


