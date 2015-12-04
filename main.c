#include "strgraph.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>

#define MAXLEN 100000
#define MAX 300
#define HDIM 5000000

typedef struct {
	StrGraph *G;
	FILE *f;
} WorkThreadArg;

void * Parsing(void *arg) {
	StrGraph *G = ((WorkThreadArg *) arg)->G;
	FILE *in = ((WorkThreadArg *) arg)->f;
	
	char buffer[MAXLEN], title[MAX], ref[MAX];
	
	char *exceptions[] = {"File:", "Image:", "wikt:", "WP:", "Wikipedia:", "Category:",
	"Talk:", "talk:", "User:", "user:", "Special:", "Template:"};

	
	//parsing the wikipedia file
	while(fgets(buffer, MAXLEN, in) != NULL){
		buffer[strlen(buffer)-1] = '\0';

		//parsing new page
		if(strstr(buffer,"<page>") != NULL){

			//save a pointer to the page title
			fgets(buffer, MAXLEN, in);
			buffer[strlen(buffer)-1] = '\0';

			char p[MAX];
			strcpy(p, strstr(buffer,"<title>"));
			
			//page doesn't reach the end
			while(strstr(buffer,"</page>") == NULL){

				fgets(buffer, MAXLEN, in);
				buffer[strlen(buffer)-1] = '\0';

				//check if has redirect title to another page
				if(strstr(buffer, "<redirect title=") != NULL){
					break;
				}

				//if has no redirection, it means that is an original page, so we save the title and the page references
				else if(strstr(buffer,"<revision>") != NULL){
					//save title
					if(p != NULL){
						int j = 0, i = 7;
					
						while(p[i] != '<'){
							title[j++] = p[i++];
						}

						title[j] = 0;
					}
				}
				
				char *q;
				if((q = strstr(buffer,"[[")) != NULL){
					
					while((q = strstr(q,"[[")) != NULL){
						int j = 0, except = 0;
						q += 2;

						while(*q && *q != ']' && *q != '|' && *q != '#'){
							ref[j++] = *q++;
						}

						ref[j] = 0;

						if(strchr(ref,':') == NULL){
							addPath(G, title, ref);
						}
						else{
							for(int k = 0; k < 12; ++k){
								if(strstr(ref,exceptions[k]) != NULL){
									except = 1;
									break;
								}
							}
						
							//add reference
							if(except == 0)
								addPath(G, title, ref);
						}
					}
				}
			}
		}
	}

	return NULL;
}

void * track (void *fp) {
	FILE *in = (FILE *) fp;
	struct stat st;
	
	//get file size
	fstat(fileno(in), &st);
	long double size = (long double) st.st_size;

	//print completion ratio every second
	long double completion = ftell(fp) / size;
	while (completion < 0.999) {
		sleep(1);
		printf("%Lf%% complete\n", completion * 100);
		completion = ftell(fp) / size;
	}
	
	return NULL;
}

int main(void){
	
	pthread_t workThread, trackThread;
	
	FILE *in;
	in = fopen("out.txt", "r");
	if(!in){
		printf("Error\n");
		return 0;
	}	

	StrGraph *G = initStrGraph(HDIM, INITSTRLEN);
	
	WorkThreadArg arg = {G, in};
	
	pthread_create(&workThread, NULL, Parsing, &arg);
	pthread_create(&trackThread, NULL, track, in);
	
	pthread_join(workThread, NULL);
  pthread_join(trackThread, NULL);

	fclose(in);
	
	saveStrGraph(G, "save");
	
	/*
	StrGraph *G = loadStrGraph("save");
	
	FILE *out = fopen("possible_exceptions", "w");
	if (!out) return 0;
	
	for (uint i = 0; i < G->hashTable->max; ++i) {
		for (unsigned char j = 0; j < G->hashTable->h[i].max; ++j) {
			if (G->hashTable->h[i].nodes[j]->pos == 0) {
				fprintf(out, "%s\n", G->S->s + G->hashTable->h[i].nodes[j]->nPos);
			}
		}
	}
	*/
	destroyStrGraph(&G);
	return 0;
}
