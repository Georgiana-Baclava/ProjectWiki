#include "strgraph.h"

#define SEPARATOR "|";
#define ARTICLE_TABLE_NAME "names.csv"
#define LINKS_TABLE_NAME "links.csv"
#define INIT_SIZE 10

unsigned int currentIndex = 1;

typedef struct {
	uint index;
	uint pos;
} ArticleData;

typedef struct {
	ArticleData *articles;
	uint size, pos;
} ArticleDataArray;

typedef struct {
	uint size;
	ArticleDataArray *data;
} ArticleHash;

uint hashFf (const char *s) {
	uint hash = 0;
	
	while (*s != '\0') {
		hash = *s + (hash << 6) + (hash << 16) - hash;
		++s;
	}
	
	return hash;
}

ArticleHash * initHash (uint size) {
	ArticleHash *h = (ArticleHash *) malloc(1 * sizeof(ArticleHash));
	if (!h) return NULL;

	h->size = size;

	h->data = (ArticleDataArray *) calloc(size, sizeof(ArticleDataArray *));
	if (!h->data) {
		free(h);
		return NULL;
	}

	for (unsigned int i = 0; i < size; ++i) {
		h->data[i].articles = (ArticleData *) calloc(INIT_SIZE, sizeof(ArticleData));
		h->data[i].size = INIT_SIZE;
		if (!h->data[i].articles) {
			free(h->data);
			free(h);
			return NULL;
		}
	}

	return h;
}

int getIndex (char *name, int nPos, ArticleHash *ah, NameString *allNames, FILE *outNames) {
	int pos = hashFf(name) % ah->size;
	if (strlen(name) > 255) return -1;
	unsigned int i = 0;

	while ((i < ah->data[pos].size) && (ah->data[pos].articles[i].index > 0)) {
		if (!strcmp(name, &allNames->s[ah->data[pos].articles[i].pos])) {
			return ah->data[pos].articles[i].index;
		}
		++i;
	}

	if (ah->data[pos].pos == ah->data[pos].size) {
		ah->data[pos].articles = realloc(ah->data[pos].articles, ah->data[pos].size + 5);
	}

	ah->data[pos].articles[i].index = currentIndex;
	++currentIndex;
	ah->data[pos].articles[i].pos = nPos;
	++ah->data[pos].pos;


	fprintf(outNames, "%d|%s\n", ah->data[pos].articles[i].index, &allNames->s[nPos]);

	return ah->data[pos].articles[i].index;
}

void graphToCsv (StrGraph *G, ArticleHash *ah, FILE *outNames, FILE *outLinks) {
	int secondIndex = 0;
	for (unsigned int i = 0; i < G->hashTable->max; ++i) {
		NodeV nv = G->hashTable->h[i];
		for (unsigned int j = 0; j < nv.max && nv.nodes[j] != NULL; ++j) {
			if (nv.nodes[j]) {
				int srcIndex = getIndex(&G->S->s[nv.nodes[j]->nPos], nv.nodes[j]->nPos, ah, G->S, outNames);
				for (int k = 0; k < nv.nodes[j]->pos; ++k) {
					int destIndex = getIndex(&G->S->s[nv.nodes[j]->dest[k]], nv.nodes[j]->dest[k], ah, G->S, outNames);
					if (srcIndex > -1 && destIndex > -1) {
						fprintf(outLinks, "%d|%d|%d\n", ++secondIndex, srcIndex, destIndex);
					}
				}
			}
		}
	}
}


int main (void) {
	FILE *in =  fopen ("save", "r");
	if (!in) {
		return 1;
	}

	FILE *outNames = fopen ("/tmp/names.csv", "w");
	if (!outNames) {
		fclose(in);
		return 1;
	}

	FILE *outLinks = fopen ("/tmp/links.csv", "w");
	if (!outLinks) {
		fclose(in);
		fclose(outNames);
		return 1;
	}

	StrGraph *G = loadStrGraph("save1");
	if (!G) {
		fclose(in);
		fclose(outNames);
		fclose(outLinks);
		return 2;
	}

	ArticleHash *ah = initHash(5000000);
	if (!ah) {
		free(G);
		fclose(in);
		fclose(outNames);
		fclose(outLinks);
		return 2;
	}

	graphToCsv(G, ah, outNames, outLinks);

	fclose(in);
	fclose(outNames);
	fclose(outLinks);

	return 0;
}