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
  hm->next = NULL;
  return hm;
}


void free_s_table() {
    int i = 0, ind= S_TABLE->inds[0];
    int last= 0;
    while(ind != -1) {
        if (last != ind) FreeLLIST_all(&S_TABLE->list[ind]);
        last = ind;
        ind = S_TABLE->inds[++i];
    }
    free(S_TABLE->list);
    free(S_TABLE);
    
}

void insert_hashmap_addr(hashmap * table, int data, int id) {
    int ind = table->inds[id];
    LL_NODE **p = &table->list[ind];
    while(1) {
        if ((*p)->id == id) break;
        else if ((*p) == NULL) assert(0);
        p = &(*p)->next;
    }
    (*p)->data.addr = data;
}
int getId(char * str, hashmap * table) {
    int ind = (int) hash(str) % S_TABLE_SIZE;
    LL_NODE ** p = (table->list[ind]) ? &table->list[ind] : NULL;
    while(p != NULL) {
        if (match(str, (*p)->name)) return (*p)->id;
        p = &(*p)->next;
    }
    return -1;
}

int getAddrByID(int sym, hashmap * table) {
    int ind = table->inds[sym];
    LL_NODE ** p = &S_TABLE->list[ind];
    while((*p) != NULL) {
        if ((*p)->id == sym) return (*p)->data.addr;
        p = &(*p)->next;
    }
    return -1;
}
int getAddrByAST(AST * arg, hashmap* table) {
    assert(arg->tag == tag_symbol);
    int sym = arg->oper.symbol;
    return getAddrByID(sym, table);
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

int getIndexByHash(char * str) {
    int h = hash(cur_token()) % S_TABLE_SIZE;
    return S_TABLE->inds[h];
}
int getIndexBySymbol(AST * node) {
    assert(node->tag == tag_symbol);
    int id = node->oper.symbol;
    return S_TABLE->inds[id];
}

LL_NODE * MakeLLnode(char * name,AST * data) {

    LL_NODE * l = (LL_NODE*)malloc(sizeof(LL_NODE));
    strncpy(l->name, name,strlen(name));

    switch (data->tag)
    {
    case tag_int:
        l->data.i = data->oper.intExp;
        l->type = type_int;
        break;
    case tag_str:
        strncpy(l->data.c, data->oper.strExp + 1, strlen(data->oper.strExp)-2);
        l->type = type_string;
        break;
    case tag_var:
    case tag_assign:
        l->type = type_pointer_var;
        break;
    default:
        l->type = type_null;
        break;
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




void FreeLLIST_all(LL_NODE ** p) {
    LL_NODE * l = *p;
    if (l->next != NULL) FreeLLIST_all(&l->next);
    free(l); 
}


void quicksort(int *arr, int left, int right) {
    int i, j;
    i = left; j = right;
    int pivot = arr[(left+right) /2];  
    int temp;
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
    int arrHash[len];
    int i = 0;
    while (!feof(keystream))
    {
        fgets(key,50,keystream);
        if (key[strlen(key)-1] == '\n') key[strlen(key)-1] = 0;
        arrHash[i] = (int) hash(key);
        printf("#define %s_H %i\n", key, arrHash[i]);
        i++;
    }
    quicksort(arrHash, 0, len);
    for (int i = 0; i < len-1; i++) {
        if (arrHash[i] == arrHash[i+1]) {
            printf("WARNING: COLLISION DETECTED: %i", arrHash[i]);
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
