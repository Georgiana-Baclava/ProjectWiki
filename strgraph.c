#include "strgraph.h"

extern inline uint hashF (const char *s) {
	uint hash = 0;
	
	while (*s != '\0') {
		hash = *s + (hash << 6) + (hash << 16) - hash;
		++s;
	}
	
	return hash;
}

extern inline uint addName (NameString *S, const char *s) {
	int l = strlen(s);
	uint pos = S->pos;
	
	if (pos + l + 1 >= S->size) {
		S->size += INCRSTR;
		char *test = (char *) realloc(S->s, S->size * sizeof(char));
		if (!test) return 2147483647;
		
		S->s = test;
	}
	
	strcpy(&S->s[pos], s);
	
	S->pos = pos + l + 1;
	
	return pos;
} 

StrGraph *initStrGraph (uint hdim, uint sdim) {
	StrGraph *aG = (StrGraph *) malloc(sizeof(StrGraph));
	
	if (!aG) return NULL;
	
	aG->S = (NameString *) calloc(1, sizeof(NameString));
	
	if (!aG->S) {
		free(aG);
		return NULL;
	}
	
	if (!sdim) sdim = INITSTRLEN;
	
	aG->S->s = (char *) malloc(sdim * sizeof(char));
	if (!aG->S->s) {
		free(aG->S);
		free(aG);
	}
	
	aG->S->size = sdim;
	
	aG->n = 0;
	aG->hashTable = (Hash *) malloc(sizeof(Hash));
	
	if (!aG->hashTable) {
		free(aG->S);
		free(aG);
		return NULL;
	}
	
	if (hdim) {
		aG->hashTable->h = (NodeV *) calloc(hdim, sizeof(NodeV));
	
		if (!aG->hashTable->h) {
			free(aG->S);
			free(aG->hashTable);
			free(aG);
		}
	
		NodeV *nv = aG->hashTable->h;
		for (uint i = 0; i < hdim; ++i) {
			nv[i].nodes = (Node **) calloc(2, sizeof(Node *));
			nv[i].max = 2;
		}
	
		aG->hashTable->max = hdim;
	} 
	
	return aG;
}

Node * findNode (StrGraph *aG, const char *target, int add) {
	uint pos = hashF(target) % aG->hashTable->max; //find hash
	
	NodeV *v = &aG->hashTable->h[pos]; //easier referencing
	
	//search in vector
	unsigned char i = 0;
	while (i < v->max && v->nodes[i]) {
		//if found then return
		if (!strcmp(target, &aG->S->s[v->nodes[i]->nPos])) {
			return v->nodes[i]; 
		}
		++i;
	}
	
	if (add) {
		//if this part is reached, new node needs to be added
		//first check if there is enough space
		if (i == v->max) {
			if (!v->max) {
				v->max = 1;
			} else {		
				v->max *= 2;
			}
			Node **test = (Node **) realloc(v->nodes, v->max * sizeof(Node *));
		
			if (!test) {
				printf("Allocation failed with size %lu\n", v->max * sizeof(Node *));
				return NULL; //allocation failed
			}
		
			v->nodes = test; //reallocation succesful
		
			memset(&v->nodes[i], 0, i * sizeof(Node *));
		}
	
		++aG->n; //increase nodes count
	
		//create new node
		v->nodes[i] = (Node *) malloc(sizeof(Node)); //allocate new node;
		v->nodes[i]->dest = (uint *) malloc(sizeof(uint)); //allocate dest vector
		v->nodes[i]->pos = 0; //initial pos 0
		v->nodes[i]->size = 1; //initial size 1
		
		//copy name
		v->nodes[i]->nPos = addName(aG->S, target);
		
		return v->nodes[i];
	}
	
	return NULL;
}
		
int addPath (StrGraph *aG, const char *src, const char *dest) {
	static Node *s = NULL;
	//search for source only if it differs from previous call
	if (!s || strcmp(src, aG->S->s + s->nPos)) {
		s = findNode(aG, src, 1);
	}
	
	Node *d = findNode(aG, dest, 1);
	
	if (!s || !d) return 2;
	
	//binary search to decide if element is already there
	int l = 0, r = (s->pos) ? s->pos - 1 : 0;
	while (l < r) {
		uint mid = (l + r) / 2;
		int diff = strcmp(dest, aG->S->s + s->dest[mid]);
		if (diff < 0) {
			r = mid - 1;
		} else if (diff > 0) {
			l = mid + 1;
		} else {
			return 0;
		}
	}
	
	//if this part is reached then new path needs to be inserted
	//first check if there is space (if not allocate new space)
	if (s->pos == s->size) {
		if (!s->size) {
			s->size = 1;
		} else {
			s->size *= 2;
		}
		
		uint *test = (uint *) realloc(s->dest, s->size * sizeof(uint));
		if (!test) {
			printf("Allocation failed with size: %lu\n", s->size * sizeof(uint));
			return 2;
		}
		
		s->dest = test;
	}
	
	//then perform ordered insertion
	if (l < s->pos) {
		memmove(&s->dest[l+1], &s->dest[l], (s->pos - l) * sizeof(uint));
	}
	s->dest[l] = d->nPos;
	++s->pos;	 
	
	return 1;
}
	 
int saveStrGraph (StrGraph *aG, const char *fname) {
	FILE *out = fopen(fname, "w");
	
	if (!out) return 1;
	
	//write nameString length
	fwrite(&aG->S->pos, sizeof(uint), 1, out);
	//write node count
	fwrite(&aG->n, sizeof(uint), 1, out);
	//write max dim for hash
	fwrite(&aG->hashTable->max, sizeof(uint), 1, out);
	
	//write all node names
	fwrite(aG->S->s, sizeof(char), aG->S->pos, out);
	
	//write name position and number of paths for each node
	for (uint i = 0; i < aG->hashTable->max; ++i) {
		NodeV *nv = &aG->hashTable->h[i];
		
		unsigned char max;
		for (max = 0; max < nv->max && nv->nodes[max]; ++max);		
		fwrite(&max, sizeof(unsigned char), 1, out);
		
		for (unsigned char j = 0; j < max; ++j) {
			fwrite(&nv->nodes[j]->nPos, sizeof(uint), 1, out);
			fwrite(&nv->nodes[j]->pos, sizeof(unsigned short), 1, out);
			
			//write all paths
			fwrite(nv->nodes[j]->dest, sizeof(uint), nv->nodes[j]->pos, out);
		}
	}
	
	fclose(out);
	
	return 0;
}

StrGraph * loadStrGraph (const char *fname) {
	FILE *in = fopen(fname, "r");
	
	if (!in) return NULL;
	
	uint strSize;
	fread(&strSize, sizeof(uint), 1, in);
	
	StrGraph *aG = initStrGraph(0, strSize);
	
	if (!aG) {
		fclose(in);
		return NULL;
	}
	
	aG->S->pos = strSize;	
	fread(&aG->n, sizeof(uint), 1, in);
	fread(&aG->hashTable->max, sizeof(uint), 1, in);
	
	aG->hashTable->h = (NodeV *) malloc(aG->hashTable->max * sizeof(NodeV));
	if (!aG->hashTable->h) {
		fclose(in);
		free(aG->hashTable);
		free(aG);
		return NULL;
	}
	
	//read all names
	fread(aG->S->s, sizeof(char), strSize, in);
	
	//read all nodes
	for (uint i = 0; i < aG->hashTable->max; ++i) {
		NodeV *nv = &aG->hashTable->h[i];
		fread(&nv->max, sizeof(unsigned char), 1, in);
		
		//allocate hash entry (NodeV)
		nv->nodes = (Node **) malloc(nv->max * sizeof(Node *));
		
		//read name and number of paths for each node	
		for (unsigned char j = 0; j < nv->max; ++j) {
			nv->nodes[j] = (Node *) malloc(sizeof(Node));
			
			fread(&nv->nodes[j]->nPos, sizeof(uint), 1, in);
			fread(&nv->nodes[j]->pos, sizeof(unsigned short), 1, in);
			nv->nodes[j]->size = nv->nodes[j]->pos;
			
			//alocate paths vector
			nv->nodes[j]->dest = (uint *) malloc(nv->nodes[j]->pos * sizeof(uint));
			
			fread(nv->nodes[j]->dest, sizeof(uint), nv->nodes[j]->pos, in); 
		}
	}			
	
	fclose(in);
	return aG;
}

void printStrGraph (StrGraph *aG, const char *fname) {
	FILE *out = fopen(fname, "w");
	
	if (!out) return;
	
	for (uint i = 0; i < aG->hashTable->max; ++i) {
		NodeV *nv = &aG->hashTable->h[i];
		
		for (unsigned char j = 0; j < nv->max && nv->nodes[j]; ++j) {
			fprintf(out, "'%s': ", &aG->S->s[nv->nodes[j]->nPos]);
			for (unsigned short k = 0; k < nv->nodes[j]->pos; ++k) {
				fprintf(out, "'%s' ", &aG->S->s[nv->nodes[j]->dest[k]]);
			}
			fprintf(out, "\n");
		}
	}
	
	fclose(out);
} 	

void destroyStrGraph (StrGraph **aG) {
	Hash *H = (*aG)->hashTable;
	
	free((*aG)->S->s);
	free((*aG)->S);
	
	for (uint i = 0; i < H->max; ++i) {
		NodeV *nv = &H->h[i];
		for (unsigned char j = 0; j < nv->max && nv->nodes[j]; ++j) {
			free(nv->nodes[j]->dest);
			free(nv->nodes[j]);
		}
		free(nv->nodes);
	}
	
	free(H->h);
	free(H);
	free(*aG);
	*aG = NULL;
}
	
	
