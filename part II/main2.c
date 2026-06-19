#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h> 

#define DIRECTION 8
#define YEAR 5
#define DATE 3
#define DATEC 11
#define WEEKDAY 10
#define MAX_STR 64
#define MAX_ROWS 111440


//============================================ 
// Structures
//============================================ 

typedef struct {
    char direction[DIRECTION];
    char year[YEAR];
    int date[DATE];
    char weekday[WEEKDAY];
    char country[MAX_STR];
    char commodity[MAX_STR];
    char transport_mode[MAX_STR];
    char measure[MAX_STR];
    long long value;
    long long cumulative;
} Record;

typedef struct NodeA {
    Record* record;
    struct NodeA* left;
    struct NodeA* right;
    int height;
} NodeA;

typedef struct EqNode {
    Record* value;
    struct EqNode* next;
} EqNode;

typedef struct NodeB {
    Record* record;
    struct NodeB* left;
    struct NodeB* right;
    int height;

    EqNode* equal;
} NodeB;


typedef struct {
    char direction[DIRECTION];
    char year[YEAR];
    char date[DATEC];
    char weekday[WEEKDAY];
    char country[MAX_STR];
    char commodity[MAX_STR];
    char transport_mode[MAX_STR];
    char measure[MAX_STR];
    long long value;
    long long cumulative;
} RecordC;

typedef struct NodeC {
    RecordC* record;
    struct NodeC* next;
} NodeC;

typedef struct {
    NodeC **buckets;
    int m;
    int totalRecords;
}HashMap;

// ==============================================
// Recording related functions
// ==============================================


int load_csv(const char *filename, Record *data) {
    FILE *fptr = fopen(filename, "r");
    if(!fptr) {
        printf("[ERROR] No file %s found.\n", filename);
        return -1;
    }

    char line[512]; //temp info saver
    int count = 0;

    fgets(line, sizeof(line), fptr);

    while (fgets(line, sizeof(line), fptr) && count < MAX_ROWS) {
        line[strcspn(line, "\r\n")] = 0;

        char *temp;
        Record r;
        memset(&r, 0, sizeof(r));

        temp = strtok(line, ",");
        if(!temp) continue;
        strncpy(r.direction, temp, DIRECTION-1);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.year, temp, YEAR-1);

        // ~~~~~~~~~~~~~~~~
        // take the date
        temp = strtok(NULL, "/");
        if(!temp) continue;
        r.date[0] = strtol(temp, NULL, 10);

        temp = strtok(NULL, "/");
        if(!temp) continue;
        r.date[1] = strtol(temp, NULL, 10);
        
        temp = strtok(NULL, ",");
        if(!temp) continue;
        r.date[2] = strtol(temp, NULL, 10);
        // ~~~~~~~~~~~~~~~~

        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.weekday, temp, sizeof(r.weekday) - 1);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.country, temp, sizeof(r.country) - 1);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.commodity, temp, sizeof(r.commodity) - 1);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.transport_mode, temp, sizeof(r.transport_mode) - 1);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.measure, temp, sizeof(r.measure) - 1);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        r.value = atoll(temp);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        r.cumulative = atoll(temp);

        data[count++] = r;
    }

    fclose(fptr);
    return count;
}

int load_csv_C(const char *filename, RecordC *data) {
    FILE *fptr = fopen(filename, "r");
    if(!fptr) {
        printf("[ERROR] No file %s found.\n", filename);
        return -1;
    }

    char line[512]; //temp info saver
    int count = 0;

    fgets(line, sizeof(line), fptr);

    while (fgets(line, sizeof(line), fptr) && count < MAX_ROWS) {
        line[strcspn(line, "\r\n")] = 0;

        char *temp;
        RecordC r;
        memset(&r, 0, sizeof(r));

        temp = strtok(line, ",");
        if(!temp) continue;
        strncpy(r.direction, temp, DIRECTION-1);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.year, temp, YEAR-1);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.date, temp, DATEC-1);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.weekday, temp, sizeof(r.weekday) - 1);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.country, temp, sizeof(r.country) - 1);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.commodity, temp, sizeof(r.commodity) - 1);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.transport_mode, temp, sizeof(r.transport_mode) - 1);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.measure, temp, sizeof(r.measure) - 1);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        r.value = atoll(temp);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        r.cumulative = atoll(temp);

        data[count++] = r;
    }

    fclose(fptr);
    return count;
}


// ==============================================
// Helping functions
// ==============================================


int compareDates(int* x, int* y) {
    if(x[2] != y[2]) return x[2] - y[2];
    if(x[1] != y[1]) return x[1] - y[1];
    return x[0] - y[0];
}



// ==============================================
// Menu functions
// ==============================================


int printMenuBSTorHashing() {
    int choice;

    printf("\n=======MENU A=======\n");
    printf("1 => BST\n");
    printf("2 => Hashing\n");
    printf("3 => Exit\n");
    printf("Choice: ");
    scanf("%d", &choice);

    return choice;
}

int printMenuBST_datecumulative() {
    int choice;

    printf("\n=======MENU A=======\n");
    printf("1 => BST by Date\n");
    printf("2 => BST by Cumulative\n");
    printf("3 => Exit\n");
    printf("Choice: ");
    scanf("%d", &choice);

    return choice;
}

int printMenuA() {
    int choice;

    printf("\n=======MENU A=======\n");
    printf("1 => In-order Display\n");
    printf("2 => Search by date\n");
    printf("3 => Modify cumulative\n");
    printf("4 => Delete by date\n");
    printf("5 => Exit\n");
    printf("Choice: ");
    scanf("%d", &choice);

    return choice;
}

int printMenuB() {
    int choice;

    printf("\n=======MENU=======\n");
    printf("1 => Display Minimum Cumulative Dates\n");
    printf("2 => Display Maximum Cumulative Dates\n");
    printf("3 => Exit\n");
    printf("Choice: ");
    scanf("%d", &choice);

    return choice;
}

int printMenuC() {
    int choice;

    printf("\n=======MENU=======\n");
    printf("1 => Search Cumulative by Date\n");
    printf("2 => Modify Cumulative by Date\n");
    printf("3 => Delete Record by Date\n");
    printf("4 => Exit\n");
    printf("Choice: ");
    scanf("%d", &choice);

    return choice;
}


// ==============================================
// A functions
// ==============================================

NodeA* createNodeA(Record* value);
NodeA* insertA(NodeA* node, Record* value);
int heightA(NodeA* node);
int balanceFactorA(NodeA* node);
void updateHeightA(NodeA* node);
NodeA* rotateRightA(NodeA* node);
NodeA* rotateLeftA(NodeA* node);
NodeA* balanceA(NodeA* node);
NodeA* buildTreeA(Record* data, int n);
void swapNodesA(NodeA* to, NodeA* from);
void inorderA(NodeA* root);
NodeA* searchA(NodeA* root, int date[3]);
NodeA* modifyA(NodeA* root, int date[3], long long value);
NodeA* minNodeA(NodeA* node);
NodeA* deleteNodeA(NodeA* root, int date[3]);


// Node related functions


NodeA* createNodeA(Record* value) {
    NodeA* newNode = malloc(sizeof(NodeA));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->record  = value;
    newNode->left  = NULL;
    newNode->right = NULL;
    newNode->height = 1;
    return newNode;
}


NodeA* insertA(NodeA* node, Record* value) {
    // Βρήκαμε την κατάλληλη κενή θέση → δημιουργούμε και τοποθετούμε νέο κόμβο 
    if (node == NULL) {
        return createNodeA(value);
    }

    int cmp = compareDates(value->date, node->record->date);
    if (cmp<0) {
        node->left = insertA(node->left, value);
    } else if (cmp>0) {
        node->right = insertA(node->right, value);
    } else {
        return node;
    }

    return balanceA(node);
}


// Tree related functions


int heightA(NodeA* node) {
    return node ? node->height : 0;
}

int balanceFactorA(NodeA* node) {
    return node ? heightA(node->left) - heightA(node->right) : 0;
}

void updateHeightA(NodeA* node) {
    if(node) {
        node->height = 1 + (heightA(node->left) > heightA(node->right) 
            ? heightA(node->left) : heightA(node->right));
    }
}

NodeA* rotateRightA(NodeA* node) {
    NodeA* x = node->left;
    NodeA* y = x->right;
    x->right = node;
    node->left = y;
    updateHeightA(node);
    updateHeightA(x);
    return x;
}

NodeA* rotateLeftA(NodeA* node) {
    NodeA* x = node->right;
    NodeA* y = x->left;
    x->left = node;
    node->right = y;
    updateHeightA(node);
    updateHeightA(x);
    return x;
}

NodeA* balanceA(NodeA* node) {
    updateHeightA(node);
    int bf = balanceFactorA(node);

    //left leaning
    if (bf>1) {
        if(balanceFactorA(node->left) < 0) {
            node->left = rotateLeftA(node->left);
        }
        return rotateRightA(node);
    }

    //right leaning
    if (bf<-1) {
        if(balanceFactorA(node->right) > 0) {
            node->right = rotateRightA(node->right);
        }
        return rotateLeftA(node);
    }

    return node;
}

NodeA* buildTreeA(Record* data, int n) {
    NodeA* root = NULL;

    for(int i = 0; i < n; i++) {
        root = insertA(root, &data[i]);
    }

    return root;
}

// helping functions

void swapNodesA(NodeA* to, NodeA* from) {
    Record* temp = to->record;
    to->record = from->record;
    from->record = temp;
}

//Menu functions

void inorderA(NodeA* root) {
    if(!root) return;

    inorderA(root->left);

    printf("Date: %-d/%d/%d \t Cumulative: %lld\n", 
        root->record->date[0], root->record->date[1], root->record->date[2],
        root->record->cumulative);
        
    inorderA(root->right);
}

NodeA* searchA(NodeA* root, int date[3]) {
    if(!root) return NULL;

    int cmp = compareDates(date, root->record->date);

    if(cmp == 0) return root;

    if(cmp < 0) return searchA(root->left, date);

    return searchA(root->right, date);
}

NodeA* modifyA(NodeA* root, int date[3], long long value) {
    NodeA* node = searchA(root, date);

    if (node) {
        node->record->cumulative = value;
    } else {
        printf("Date not found.\n");
    }

    return root;
}

NodeA* minNodeA(NodeA* node) {
    while(node->left) {
        node = node->left;
    }

    return node;
}

NodeA* deleteNodeA(NodeA* root, int date[3]) {
    if(!root) {
        printf("No nodes present.\n");
        return NULL;
    }

    int cmp = compareDates(date, root->record->date);

    if (cmp < 0) {
        root->left = deleteNodeA(root->left, date);
    } else if (cmp > 0) {
        root->right = deleteNodeA(root->right, date);
    } else {
        if(!root->left || !root->right) {
            NodeA* temp = root->left ? root->left : root->right;
            free(root);
            return temp;
        }

        NodeA* junior = minNodeA(root->right);
        swapNodesA(root, junior);
        root->right = deleteNodeA(root->right, junior->record->date);
    }

    return balanceA(root);
}


// ==============================================
// B functions      
// ==============================================

EqNode* createEqNode(Record* record);
void insertEqNode_End(NodeB* node, Record* value);
NodeB* createNodeB(Record* value);
NodeB* insertB(NodeB* node, Record* value);
int heightB(NodeB* node);
int balanceFactorB(NodeB* node);
void updateHeightB(NodeB* node);
NodeB* rotateRightB(NodeB* node);
NodeB* rotateLeftB(NodeB* node);
NodeB* balanceB(NodeB* node);
NodeB* buildTreeB(Record* data, int n);
void swapNodesB(NodeB* to, NodeB* from);
NodeB* findMinB(NodeB* root);
NodeB* findMaxB(NodeB* root);
void printDates_MinB(NodeB* root);
void printDates_MaxB(NodeB* root);

// Node related functions


EqNode* createEqNode(Record* record) {
    EqNode* new = malloc(sizeof(EqNode));
    if(new == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    new->value = record;
    new->next = NULL;
    return new;
}

void insertEqNode_End(NodeB* node, Record* value) {
    EqNode* new = createEqNode(value);
    
    if(node->equal == NULL) {
        node->equal = new;
        return;
    }

    EqNode* temp = node->equal;
    while(temp->next != NULL){
        temp = temp->next;
    }
    temp->next = new;
}



NodeB* createNodeB(Record* value) {
    NodeB* newNode = malloc(sizeof(NodeB));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->record  = value;
    newNode->left  = NULL;
    newNode->right = NULL;
    newNode->height = 1;
    return newNode;
}

NodeB* insertB(NodeB* node, Record* value) {
    // Βρήκαμε την κατάλληλη κενή θέση → δημιουργούμε και τοποθετούμε νέο κόμβο 
    if (node == NULL) {
        return createNodeB(value);
    }

    int cmp = compareDates(value->date, node->record->date);
    if (cmp<0) {
        node->left = insertB(node->left, value);
    } else if (cmp>0) {
        node->right = insertB(node->right, value);
    } else {
        return node;
    }

    return balanceB(node);
}


// Tree related functions


int heightB(NodeB* node) {
    return node ? node->height : 0;
}

int balanceFactorB(NodeB* node) {
    return node ? heightB(node->left) - heightB(node->right) : 0;
}

void updateHeightB(NodeB* node) {
    if(node) {
        node->height = 1 + (heightB(node->left) > heightB(node->right) 
            ? heightB(node->left) : heightB(node->right));
    }
}

NodeB* rotateRightB(NodeB* node) {
    NodeB* x = node->left;
    NodeB* y = x->right;
    x->right = node;
    node->left = y;
    updateHeightB(node);
    updateHeightB(x);
    return x;
}

NodeB* rotateLeftB(NodeB* node) {
    NodeB* x = node->right;
    NodeB* y = x->left;
    x->left = node;
    node->right = y;
    updateHeightB(node);
    updateHeightB(x);
    return x;
}

NodeB* balanceB(NodeB* node) {
    updateHeightB(node);
    int bf = balanceFactorB(node);

    //left leaning
    if (bf>1) {
        if(balanceFactorB(node->left) < 0) {
            node->left = rotateLeftB(node->left);
        }
        return rotateRightB(node);
    }

    //right leaning
    if (bf<-1) {
        if(balanceFactorB(node->right) > 0) {
            node->right = rotateRightB(node->right);
        }
        return rotateLeftB(node);
    }

    return node;
}

NodeB* buildTreeB(Record* data, int n) {
    NodeB* root = NULL;

    for(int i = 0; i < n; i++) {
        root = insertB(root, &data[i]);
    }

    return root;
}

// helping functions

void swapNodesB(NodeB* to, NodeB* from) {
    Record* temp = to->record;
    to->record = from->record;
    from->record = temp;
}

void printEqList(NodeB* node) {
    if(!node) return;

    Record* r = node->record;
    printf("Cumulative: %lld\n \tDate: %d/%d/%d\n", r->cumulative, r->date[0], 
        r->date[1], r->date[2]);

    EqNode* eq = node->equal;
    while(eq!= NULL) {
        r = eq->value;

        printf("Cumulative: %lld\n \tDate: %d/%d/%d\n", r->cumulative, r->date[0], 
            r->date[1], r->date[2]);
        eq = eq->next;
    }
}

// B Menu Functions

NodeB* findMinB(NodeB* root) {
    if(!root) return NULL;

    while(root->left) {
        root = root->left;
    }
    return root;
}

NodeB* findMaxB(NodeB* root) {
    if(!root) return NULL;

    while(root->right) {
        root = root->right;
    }
    return root;
}

void printDates_MinB(NodeB* root) {
    NodeB* min = findMinB(root);

    if (!min) {
        printf("The tree is empty.\n");
        return;
    }

    printf("The minimum Cumulative is: %lld", min->record->cumulative);
    printEqList(min);
}

void printDates_MaxB(NodeB* root) {
    NodeB* max = findMaxB(root);

    if (!max) {
        printf("The tree is empty.\n");
        return;
    }

    printf("The maximum Cumulative is: %lld\n", max->record->cumulative);
    printEqList(max);
}


// ==============================================
// C functions
// ==============================================

int hash_date(char* date, int m);
HashMap* createMap(int m);
void destroyMap(HashMap* map);
void insertToMap(HashMap* map, RecordC* value);
int searchByDate(HashMap* map, char* date);
void modifyByDate(HashMap* map, char* date);
void deleteByDate(HashMap* map, char* date);

// Hash functions

int hash_date(char* date, int m) {
    unsigned long sum = 0;
    for(int i = 0; i < DATEC-1; i ++) {
        sum += date[i];
    }
    return (int)(sum % (unsigned long)m);
}

HashMap* createMap(int m) {
    HashMap* map = malloc(sizeof(HashMap));
    if(!map) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    map->m = m;
    map->totalRecords = 0;
    map->buckets = calloc(m,sizeof(NodeC *));
    if(!map->buckets) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    return map;
}

void destroyMap(HashMap* map) {
    for(int i = 0; i < map->m; i++) {
        NodeC* now = map->buckets[i];
        while(now) {
            NodeC* temp = now->next;
            free(now);
            now = temp;
        }
    }

    free(map->buckets);
    free(map);
}

void insertToMap(HashMap* map, RecordC* value) {
    int index = hash_date(value->date, map->m);

    NodeC* newNode = malloc(sizeof(NodeC));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    newNode->record = value;
    newNode->next = map->buckets[index];
    map->buckets[index] = newNode;
    map->totalRecords++;
}

// C menu functions


int searchByDate(HashMap* map, char* date) {
    int index = hash_date(date, map->m);
    NodeC *now = map->buckets[index];
    bool found = false;

    printf("\n========== Results for date: %s ==========\n", date);

    while(now) {
        if(strcmp(now->record->date, date) == 0) {
            printf("\tCumulative: %lld\n", now->record->cumulative);
            found = true;
        }
        now = now->next;
    }

    if(!found) {
        printf("[!] No Record in this date: %s\n",date);
    }
}

void modifyByDate(HashMap* map, char* date) {
    int index = hash_date(date, map->m);
    NodeC *now = map->buckets[index]; 
    bool found = false;
    long long newCumulative;

    while(now) {
        if(strcmp(now->record->date, date) == 0) {
            printf("Current record : \t Cumulative: %lld\n", now->record->cumulative);

            printf("New Cumulative: ");
            if (scanf("%lld", &newCumulative) != 1) {
                printf("Value not acceptable.\n");
                while(getchar() != '\n');
                now = now->next;
                continue;
            }

            now->record->cumulative = newCumulative;
            printf("Value updated : \tCumulative = %lld\n", newCumulative);
            found = true;
        }
        now = now->next;
    }

    if(!found) {
        printf("[!] No Record in this date: %s\n",date);
    }
}

void deleteByDate(HashMap* map, char* date) {
    int index = hash_date(date, map->m);
    NodeC *now = map->buckets[index]; 
    NodeC *before = NULL; 
    bool found = false;

    while(now) {
        if(strcmp(now->record->date, date) == 0) {
            NodeC* deleting = now;
            if(before) {
                before->next = now->next;
            } else {
                map->buckets[index] = now->next;
            }

            now = now->next;
            free(deleting);
            map->totalRecords--;
            found = true;
        } else {
            before = now;
            now = now->next;
        }
        
    }

    if(!found) {
        printf("[!] No Record in this date: %s\n",date);
    }
}


// ==============================================
// Main function      
// ==============================================


void runBST_date() {
    const char *filename = "effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv";
    
    //load data
    Record *original = malloc(MAX_ROWS*sizeof(Record));
    
    if (!original) {
        printf("Memory Error\n");
        return;
    }
    
    int n = load_csv(filename, original);
    if (n<=0) {
        free(original);
        return;
    }

    NodeA* root = buildTreeA(original, n);

    int choice = 0;
    int date[3];
    long long cumulative;

    while(choice!=5) {

        choice = printMenuA();

        switch(choice) {
            case 1:
                inorderA(root);
                break;
            case 2:
                printf("Enter date (DD/MM/YYYY): ");
                scanf("%d/%d/%d", &date[0], &date[1], &date[2]);
                if(date[0]>31 || date[0]<1) {
                    printf("Day is invalid.\n");
                    break;
                }
                if(date[1]>12 || date[1]<1) {
                    printf("Month is invalid.\n");
                    break;
                }
                if(date[2]>2021|| date[2]<2015) {
                    printf("Year out of bounds.\n");
                    break;
                }
                NodeA* found = searchA(root, date);
                if(found) {
                    printf("Cumulative: %lld\n", found->record->cumulative);
                } else {
                    printf("Date not found.\n");
                }
                break;
            case 3:
                printf("Enter date (DD/MM/YYYY): ");
                scanf("%d/%d/%d", &date[0], &date[1], &date[2]);
                if(date[0]>31 || date[0]<1) {
                    printf("Day is invalid.\n");
                    break;
                }
                if(date[1]>12 || date[1]<1) {
                    printf("Month is invalid.\n");
                    break;
                }
                if(date[2]>2021|| date[2]<2015) {
                    printf("Year out of bounds.\n");
                    break;
                }
                printf("New Cumulative value: ");
                scanf("%lld", &cumulative);
                root = modifyA(root, date, cumulative);
                break;
            case 4:
                printf("Enter date (DD/MM/YYYY): ");
                scanf("%d/%d/%d", &date[0], &date[1], &date[2]);
                if(date[0]>31 || date[0]<1) {
                    printf("Day is invalid.\n");
                    break;
                }
                if(date[1]>12 || date[1]<1) {
                    printf("Month is invalid.\n");
                    break;
                }
                if(date[2]>2021|| date[2]<2015) {
                    printf("Year out of bounds.\n");
                    break;
                }
                root = deleteNodeA(root, date);
                break;
            case 5:
                printf("Ending program. . .\n");
                break;
            default:
                printf("Invalid choice.\n");
        }
    }

    free(original);

    return;
}

void runBST_cumulative() {
    const char *filename = "effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv";
    
    //load data
    Record *original = malloc(MAX_ROWS*sizeof(Record));
    
    if (!original) {
        printf("Memory Error\n");
        return;
    }
    
    int n = load_csv(filename, original);
    if (n<=0) {
        free(original);
        return;
    }

    NodeB* root = buildTreeB(original, n);

    int choice = 0;
    int date[3];
    long long cumulative;

    while(choice!=3) {

        choice = printMenuB();

        switch(choice) {
            case 1:
                printDates_MinB(root);
                break;
            case 2:
                printDates_MaxB(root);
                break;
            case 3:
                printf("Ending program. . .\n");
                break;
            default:
                printf("Invalid choice.\n");
        }
    }

    free(original);

    return;
}

void runHashing() {
    const char *filename = "effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv";
    
    //load data
    RecordC *original = malloc(MAX_ROWS*sizeof(RecordC));
    
    if (!original) {
        printf("Memory Error\n");
        return;
    }
    
    int n = load_csv_C(filename, original);
    if (n<=0) {
        free(original);
        return;
    }

    HashMap* map = createMap(DATEC);

    for(int i=0; i<n; i++) {
        insertToMap(map, &original[i]);
    }

    int choice = 0;
    char date[DATEC+4];

    while(choice!=4) {

        choice = printMenuC();
        getchar();

        if(choice == 1 || choice == 2 || choice ==3) {
            printf("Enter the date (e.g. 13/03/2018): ");
            if(!fgets(date, sizeof(date), stdin)) continue;
            date[strcspn(date, "\r\n")] = 0;
        }

        switch(choice) {
            case 1:
                searchByDate(map,date);
                break;
            case 2:
                modifyByDate(map,date);
                break;
            case 3:
                deleteByDate(map,date);
                break;
            case 4:
                printf("Ending program. . .\n");
                break;
            default:
                printf("Invalid choice.\n");
        }
    }

    destroyMap(map);
    free(original);

    return;
}

int main(){

    int choice = 0;

    while(choice!=3) {

        choice = printMenuBSTorHashing();

        switch(choice) {
            case 1:

                choice = printMenuBST_datecumulative();

                switch(choice) {
                    case 1:
                        runBST_date();
                        break;
                    case 2:
                        runBST_cumulative();
                        break;
                    default:
                        printf("Invalid choice.\n");
                }
                break;
            case 2:
                runHashing();
                break;
            case 3:
                printf("Ending program. . .\n");
                break;
            default:
                printf("Invalid choice.\n");
        }
    }

    return 0;    
}