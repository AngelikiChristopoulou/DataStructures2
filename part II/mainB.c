/*
* Τροποποιήστε κατάλληλα τον κώδικα του (Α), ώστε το αρχείο να διαβάζεται 
* στο BST  με βάση την τιμή του πεδίου Cumulative. Το BST  διατάσσεται 
* ως προς την τιμή Cumulative ανά ΗΜΕΡΟΜΗΝΙΑ (Date) και υλοποιείται με 
* δυναμική διαχείριση μνήμης. Μετά την δημιουργία του BST η εφαρμογή 
* εμφανίζει ένα μενού με τις ακόλουθες επιλογές: 
*
* 1. Εύρεση ΗΜΕΡΑΣ/ΗΜΕΡΩΝ με την ΕΛΑΧΙΣΤΗ ΤΙΜΗ Cumulative. 
* 2. Εύρεση ΗΜΕΡΑΣ/ΗΜΕΡΩΝ με τη ΜΕΓΙΣΤΗ ΤΙΜΗ Cumulative.
*/

// oooooooooooooooooooooooo
//       Useage: AVL
// oooooooooooooooooooooooo

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h> 

#define DIRECTION 8
#define YEAR 5
#define DATE 3
#define WEEKDAY 10
#define MAX_STR 64
#define MAX_ROWS 111440


//============================================ 
// Δομή Εγγραφής
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

typedef struct EqNode {
    Record* value;
    struct EqNode* next;
} EqNode;

typedef struct Node {
    Record* record;
    struct Node* left;
    struct Node* right;
    int height;

    EqNode* equal;
} Node;



// ==============================================
// Function Callings
// ==============================================


int load_csv(const char *filename, Record *data);
EqNode* createEqNode(Record* record);
void insertEqNode_End(Node* node, Record* value);
Node* createNode(Record* value);
Node* insert(Node* node, Record* value);
int height(Node* node);
int balanceFactor(Node* node);
void updateHeight(Node* node);
Node* rotateRight(Node* node);
Node* rotateLeft(Node* node);
Node* balance(Node* node);
Node* buildTree(Record* data, int n);
void swapNodes(Node* to, Node* from);
int printMenu();
void printEqList(Node* node);
Node* findMin(Node* root);
Node* findMax(Node* root);
void printDates_Min(Node* root);
void printDates_Max(Node* root);


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


// ==============================================
// EqNode related functions
// ==============================================


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

void insertEqNode_End(Node* node, Record* value) {
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


// ==============================================
// Node related functions
// ==============================================


Node* createNode(Record* value) {
    Node* newNode = malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->record  = value;
    newNode->left  = NULL;
    newNode->right = NULL;
    newNode->height = 1;

    newNode->equal = NULL;

    return newNode;
}

Node* insert(Node* node, Record* value) {
    
    if (node == NULL) {
        return createNode(value);
    }

    long long cmp = value->cumulative - node->record->cumulative;

    if (cmp<0) {
        node->left = insert(node->left, value);
    } else if (cmp>0) {
        node->right = insert(node->right, value);
    } else {
        insertEqNode_End(node, value);
    }

    return balance(node);
}


// ==============================================
// Tree related functions      
// ==============================================


int height(Node* node) {
    return node ? node->height : 0;
}

int balanceFactor(Node* node) {
    return node ? height(node->left) - height(node->right) : 0;
}

void updateHeight(Node* node) {
    if(node) {
        node->height = 1 + (height(node->left) > height(node->right) 
            ? height(node->left) : height(node->right));
    }
}

Node* rotateRight(Node* node) {
    Node* x = node->left;
    Node* y = x->right;
    x->right = node;
    node->left = y;
    updateHeight(node);
    updateHeight(x);
    return x;
}

Node* rotateLeft(Node* node) {
    Node* x = node->right;
    Node* y = x->left;
    x->left = node;
    node->right = y;
    updateHeight(node);
    updateHeight(x);
    return x;
}

Node* balance(Node* node) {
    updateHeight(node);
    int bf = balanceFactor(node);

    //left leaning
    if (bf>1) {
        if(balanceFactor(node->left) < 0) {
            node->left = rotateLeft(node->left);
        }
        return rotateRight(node);
    }

    //right leaning
    if (bf<-1) {
        if(balanceFactor(node->right) > 0) {
            node->right = rotateRight(node->right);
        }
        return rotateLeft(node);
    }

    return node;
}

Node* buildTree(Record* data, int n) {
    Node* root = NULL;

    for(int i = 0; i < n; i++) {
        root = insert(root, &data[i]);
    }

    return root;
}


// ==============================================
// Helping functions
// ==============================================


void swapNodes(Node* to, Node* from) {
    Record* temp = to->record;
    to->record = from->record;
    from->record = temp;
}

int printMenu() {
    int choice;

    printf("\n=======MENU=======\n");
    printf("1 => Display Minimum Cumulative Dates\n");
    printf("2 => Display Maximum Cumulative Dates\n");
    printf("3 => Exit\n");
    printf("Choice: ");
    scanf("%d", &choice);

    return choice;
}

void printEqList(Node* node) {
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

// ==============================================
// Menu functions      
// ==============================================


Node* findMin(Node* root) {
    if(!root) return NULL;

    while(root->left) {
        root = root->left;
    }
    return root;
}

Node* findMax(Node* root) {
    if(!root) return NULL;

    while(root->right) {
        root = root->right;
    }
    return root;
}

void printDates_Min(Node* root) {
    Node* min = findMin(root);

    if (!min) {
        printf("The tree is empty.\n");
        return;
    }

    printf("The minimum Cumulative is: %lld", min->record->cumulative);
    printEqList(min);
}

void printDates_Max(Node* root) {
    Node* max = findMax(root);

    if (!max) {
        printf("The tree is empty.\n");
        return;
    }

    printf("The maximum Cumulative is: %lld\n", max->record->cumulative);
    printEqList(max);
}


// ==============================================
// Main function      
// ==============================================


int main() {
    const char *filename = "effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv";
    
    //load data
    Record *original = malloc(MAX_ROWS*sizeof(Record));
    
    if (!original) {
        printf("Memory Error\n");
        return 1;
    }
    
    int n = load_csv(filename, original);
    if (n<=0) {
        free(original);
        return 1;
    }

    Node* root = buildTree(original, n);

    int choice = 0;
    int date[3];
    long long cumulative;

    while(choice!=3) {

        choice = printMenu();

        switch(choice) {
            case 1:
                printDates_Min(root);
                break;
            case 2:
                printDates_Max(root);
                break;
            case 3:
                printf("Ending program. . .\n");
                break;
            default:
                printf("Invalid choice.\n");
        }
    }

    free(original);

    return 0;
}