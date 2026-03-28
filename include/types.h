#ifndef _TYPES_H
#define _TYPES_H

using Boolean = bool;
const inline bool False = false;
const inline bool True = true;

typedef struct {long int h, l; } LongInt;
typedef struct {long int u, h, l; } DLongInt;

#define ADDED           1
#define DELETED         2

#define	BANHOST		0
#define	BANUSER		1
#define	BANLOGIN	2
#define	BANCHECK	3

#endif
