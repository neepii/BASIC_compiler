#include "basicc.h"
#include "hash.h"
int min_available_id = 0;



hashmap * S_TABLE = NULL; //global

void introduce_s_table() {
  S_TABLE = create_table();
}

hashmap * create_table() {
  hashmap * hm;
  hm = (hashmap*) malloc(sizeof(hashmap));
  hm->list = (LL_NODE**) malloc(sizeof(LL_NODE*) * S_TABLE_SIZE);
  for (int i = 0; i < S_TABLE_SIZE; i++)
  {
    hm->list[i] = NULL;
    hm->inds[i] = -1;
  }
  return hm;
}


void free_s_table() {
    for (int i = 0; i < S_TABLE_SIZE; i++)
    {   
        if (S_TABLE->inds[i] == -1) break;
        int ind = S_TABLE->inds[i];
        FreeLLIST_all(S_TABLE->list[ind]);
        S_TABLE->list[ind] = NULL;
    
    }
    free(S_TABLE->list);
    free(S_TABLE);
    
}

void add_symbol(AST * data) {
    char name[TOKEN_LEN];
    data->inTable = true;
    if (data->tag == tag_assign) {
        strcpy(name, data->oper.assignExp.identifier->oper.varExp);
    } else if (data->tag == tag_str) {
        sprintf(name, "str%d", min_available_id);
    } else if (data->tag == tag_var) {
        sprintf(name, "%s", data->oper.varExp);
    }
    int index = (int) hash(name) % S_TABLE_SIZE;
    S_TABLE->list[index] = appendLLnode(S_TABLE->list[index], name, data);
    S_TABLE->inds[min_available_id] = index;
    if (data->tag == tag_assign) {
        data->oper.assignExp.identifier->oper.symbol = min_available_id;
        data->oper.assignExp.identifier->tag = tag_symbol;
    } else {
        data->oper.symbol = min_available_id;
        data->tag = tag_symbol;
    }
    min_available_id++;
}

LL_NODE * MakeLLnode(char * name,AST * data) {

    LL_NODE * l = (LL_NODE*)malloc(sizeof(LL_NODE));
    strncpy(l->name, name,strlen(name));

    if (data->tag == tag_int || data->tag == tag_assign) {
        l->data.i = data->oper.intExp;
        l->type = type_int;
    } else if(data->tag == tag_str) {
        strncpy(l->data.c, data->oper.strExp + 1, strlen(data->oper.strExp)-2);
        l->type = type_string;
    } else if(data->tag == tag_var) {
        strncpy(l->data.c, data->oper.varExp, strlen(data->oper.varExp));
        l->type = type_variable;
    } 
    else {
        l->type = type_null;
    }

    l->next = NULL;
    return l;
}

void removeLLnode(LL_NODE * head, char * name) {
    LL_NODE * temp = head, *prev;
    if (temp != NULL && match(name, temp->name)) {
        prev = temp;
        temp = temp->next;
        free(prev);
        return;
    }

    while(temp->name != name && temp != NULL) {
        prev = temp;
        temp = temp->next;
    }
    prev->next = temp->next;
    free(temp);
    
}

AST * getLLdata(LL_NODE * head, char* name) {
    LL_NODE * node = head;
    while (!match(node->name, name) && node->next != NULL){
        node = node->next;
    }
    if (node->type) {
        
    }
    return NULL;
}

LL_NODE * appendLLnode(LL_NODE * head, char * name, AST * data) {
    LL_NODE * newnode = MakeLLnode(name, data);
    newnode->id = min_available_id;
    newnode->next = head;
    return newnode;
}




void FreeLLIST_all(LL_NODE * l) {
    if (l->next != NULL) FreeLLIST_all(l->next);
    free(l); 
}


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

void sortAST(AST *arr[], int left, int right) {
    int i, j;
    i = left; j = right;
    int pivot = arr[(left+right) /2]->oper.numline.value;  
    AST * temp;
    do{
        while ((arr[i]->oper.numline.value < pivot) && (i < right)) i++;
        while ((arr[j]->oper.numline.value > pivot) && (j > left)) j--;
        if (i<= j) {
            temp = arr[j];
            arr[j] = arr[i];
            arr[i] = temp;
            i++; j--;
        }
    } while (i <= j);

    if (i < right) sortAST(arr, i, right);
    if (j > left) sortAST(arr, left, j);
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

unsigned long hash(char * str) {
    
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
