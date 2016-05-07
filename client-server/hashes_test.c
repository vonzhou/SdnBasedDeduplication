#include "global.h"

static int strip(char *string)
{
    char *c;
    if ((c = strchr(string, '\r'))) {
        *c = 0;
    }
    if ((c = strchr(string, '\n'))) {
        *c = 0;
    }
}

void test1(){
	// Open the dictionary file
    FILE *fp, *outfp; 
    if (!(fp = fopen("dictionary", "r"))) {
        fprintf(stderr, "E: Couldn't open words file\n");
        fflush (stderr);
        return;
    }

    if (!(outfp = fopen("dictionary.c.out", "w"))) {
        fprintf(stderr, "E: Couldn't open file for write\n");
        fflush (stderr);
        return;
    }

    // Add all dictionary words to the filter
    char *p;
    char line[1024];
    while (fgets(line, 1024, fp)) {
        strip(line);
        unsigned res = sax_hash(line);
        unsigned res2 = sdbm_hash(line);
        unsigned res3 = jenkins_hash(line);
        unsigned res4 = murmur_hash(line);
        fprintf(outfp, "%u\n", res4);
    }
    fclose(outfp);
}

int main(int argc, char *argv[])
{
	
	test1();
    
    // const char *s = "hell123";
    // printf("%s, sax hash = %u\n", s, sax_hash(s));
    // printf("%s, sdbm hash = %u\n", s, sdbm_hash(s));
    // printf("%s, jenkins hash = %u\n", s, jenkins_hash(s));
    // printf("%s, murmur2 hash = %u\n", s, murmur_hash(s));

    // unsigned k = *(unsigned*)s;
    // printf("%u\n", k);

    
    return 0;
}
