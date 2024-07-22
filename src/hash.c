#include "hash.h"
#define HASH_MOD 65521


void quicksort(unsigned long *arr, int left, int right) {
    int i, j;
    i = left; j = right;
    unsigned long pivot = arr[(left+right) /2];  
    unsigned long temp;
    do{
        while ((arr[i] < pivot) && (i < right)) i++;
        while ((arr[j] > pivot) && (j > left)) j--;
        if (i<= j) {
            temp = arr[j];
            arr[j] = arr[i];
            arr[i] = temp;
            i++; j--;
        }
    } while (i <= j);

    if (i < right) quicksort(arr, i, right);
    if (j > left) quicksort(arr, left, j);
}


void test_hashes_on_keywords(){
    FILE * keystream = fopen("listofkeywords.txt", "r");
    int len = 1;
    for (int i = fgetc(keystream); i != EOF; i=fgetc(keystream)) {
        if (i == '\n') len++;
    }
    rewind(keystream);
    char key[50];
    unsigned long arrHash[len];
    int i = 0;
    while (!feof(keystream))
    {
        fgets(key,50,keystream);
        if (key[strlen(key)-1] == '\n') key[strlen(key)-1] = 0;
        arrHash[i] = hash(key);
        printf("#define %s_H %li\n", key, arrHash[i]);
        i++;
    }
    quicksort(arrHash, 0, len);
    for (int i = 0; i < len-1; i++) {
        if (arrHash[i] == arrHash[i+1]) {
            printf("WARNING: COLLISION DETECTED: %li", arrHash[i]);
        }
    }

    fclose(keystream);
}

unsigned long hash(char * str) { //adler-32
    
    int i;
    int len = strlen(str);
    unsigned long a = 0;
    unsigned long b = 1;

    for (i = 0; i < len; i++)
    {
        a = (a + str[i]) % HASH_MOD;
        b = (a+b) % HASH_MOD;
    }
    
    return (b>>16) | a;
}