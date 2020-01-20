
#ifndef _SPAWN_H
#define _SPAWN_H 1

#include "defines.h"

int spawncli (bool f, int n);
int bktoshell (bool f, int n);
void rtfrmshell (void);
int spawn (bool f, int n);
int execprg (bool f, int n);
int pipecmd (bool f, int n);
int filter_buffer (bool f, int n);

#endif
