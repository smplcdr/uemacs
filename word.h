#ifndef _WORD_H
#define _WORD_H 1

#include "defines.h"

#define WORDPRO 1

int wrapword (bool f, int n);
int backword (bool f, int n);
int forwword (bool f, int n);
int upperword (bool f, int n);
int lowerword (bool f, int n);
int capword (bool f, int n);
int delfword (bool f, int n);
int delbword (bool f, int n);
#if WORDPRO
int gotobop (bool f, int n);
int gotoeop (bool f, int n);
int fillpara (bool f, int n);
int justpara (bool f, int n);
int killpara (bool f, int n);
int wordcount (bool f, int n);
#endif

#endif
