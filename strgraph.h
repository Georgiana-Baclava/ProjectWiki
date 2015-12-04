#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef _STRGRAPH_
#define _STRGRAPH_

#define INITSTRLEN 100
#define INCRSTR 50000000

typedef unsigned int uint;

/*nPos - position of name in NameString
dest - vector of pointers to destination nodes
pos - current position in dest vector
size - max number of dest components*/
typedef struct node  {
	uint nPos;
	uint *dest;
	unsigned short pos, size;
} Node;

//vector of nodes
typedef struct {
	Node **nodes;
	unsigned char max;
} NodeV;

//hashTable which stores nodes by their name
typedef struct {
	NodeV *h;
	uint max;
} Hash;

typedef struct {
	char *s;
	uint pos, size;
} NameString;

typedef struct {
	uint n;
	Hash *hashTable;
	NameString *S;
} StrGraph;

//allocates memory for graph
StrGraph *initStrGraph (uint hdim, uint sdim);

/*adds path from node with name src to node with name dest
if any of this nodes does not exist, it will add it*/
int addPath (StrGraph *aG, const char *src, const char *dest);

/*find node with name target in graph
if not found then adds it if add == 1*/
Node * findNode (StrGraph *aG, const char *target, int add);

/*saves content of graph to binary file*/
int saveStrGraph (StrGraph *aG, const char *fname);

/*loads content of graph from binary file*/
StrGraph * loadStrGraph (const char *fname);

void printStrGraph (StrGraph *aG, const char *fname);
 
/*frees the occupied memory*/
void destroyStrGraph (StrGraph **aG);

#endif
