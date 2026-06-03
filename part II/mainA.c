/* Η εφαρμογή διαβάζει αρχικά το αρχείο και δημιουργεί ένα Δένδρο BST (Ισοζυγισμένο Δυαδικό 
* Δέντρο )  στο οποίο κάθε κόμβος του διατηρεί την εγγραφή (Date, Cumulative της ημέρας αυτής). 
* Το BST  διατάσσεται ως προς την ΗΜΕΡΟΜΗΝΙΑ (Date) και υλοποιείται με δυναμική διαχείριση 
* μνήμης. Προτιμότερες επιλογές AVL, red-black, (2,4) δέντρα. Μετά την δημιουργία του BST  η 
* εφαρμογή εμφανίζει ένα μενού με τις ακόλουθες επιλογές: 
* 1. Απεικόνιση του BST  με ενδο-διατεταγμένη διάσχιση. Κάθε απεικόνιση θα πρέπει να περιέχει 
* μια επικεφαλίδα με τους τίτλους των στοιχείων των εγγραφών που απεικονίζονται.  
* 2. Αναζήτηση της τιμής Cumulative βάσει ΗΜΕΡΟΜΗΝΙΑΣ (Date) που θα δίνεται από το 
* χρήστη. 
* 3. Τροποποίηση του περιεχομένου του πεδίου Cumulative που αντιστοιχεί σε συγκεκριμένη 
* ΗΜΕΡΟΜΗΝΙΑ (Date).  
* 4. Διαγραφή μιας εγγραφής που αντιστοιχεί σε συγκεκριμένη ΗΜΕΡΟΜΗΝΙΑ (Date). 
* 5. Έξοδος από την εφαρμογή.  
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

typedef struct Node {
    Record* record;
    struct Node* left;
    struct Node* right;
    int height;
} Node;

typedef struct EqNode {
    Node* current;
    Node* next;
} EqNode;


// ==============================================
// Function Callings
// ==============================================


Node* createNode(Record* value);
Node* insert(Node* node, Record* value);
int height(Node* node);
int balanceFactor(Node* node);
void updateHeight(Node* node);
Node* rotateRight(Node* node);
Node* rotateLeft(Node* node);
Node* balance(Node* node);
int compareDates(int* x, int* y);
void swapNodes(Node* to, Node* from);
int load_csv(const char *filename, Record *data);

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
    return newNode;
}

EqNode* createEqNode(Node* previous, Node* this) {
    EqNode* new = malloc(sizeof(EqNode));

    
}

Node* insert(Node* node, Record* value) {
    // Βρήκαμε την κατάλληλη κενή θέση → δημιουργούμε και τοποθετούμε νέο κόμβο 
    if (node == NULL) {
        return createNode(value);
    }

    int cmp = compareDates(value->date, node->record->date);
    if (cmp<0) {
        node->left = insert(node->left, value);
    } else if (cmp>0) {
        node->right = insert(node->right, value);
    } else {
        return node;
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


int compareDates(int* x, int* y) {
    if(x[2] != y[2]) return x[2] - y[2];
    if(x[1] != y[1]) return x[1] - y[1];
    return x[0] - y[0];
}

void swapNodes(Node* to, Node* from) {
    Record* temp = to->record;
    to->record = from->record;
    from->record = temp;
}

int printMenu() {
    int choice;

    printf("\n=======MENU=======\n");
    printf("1 => In-order Display\n");
    printf("2 => Search by date\n");
    printf("3 => Modify cumulative\n");
    printf("4 => Delete by date\n");
    printf("5 => Exit\n");
    printf("Choice: ");
    scanf("%d", &choice);

    return choice;
}


// ==============================================
// Menu functions
// ==============================================

void inorder(Node* root) {
    if(!root) return;

    inorder(root->left);

    printf("Date: %-d/%d/%d \t Cumulative: %lld\n", 
        root->record->date[0], root->record->date[1], root->record->date[2],
        root->record->cumulative);
        
    inorder(root->right);
}

Node* search(Node* root, int date[3]) {
    if(!root) return NULL;

    int cmp = compareDates(date, root->record->date);

    if(cmp == 0) return root;

    if(cmp < 0) return search(root->left, date);

    return search(root->right, date);
}

Node* modify(Node* root, int date[3], long long value) {
    Node* node = search(root, date);

    if (node) {
        node->record->cumulative = value;
    } else {
        printf("Date not found.\n");
    }

    return root;
}

Node* minNode(Node* node) {
    while(node->left) {
        node = node->left;
    }

    return node;
}

Node* deleteNode(Node* root, int date[3]) {
    if(!root) {
        printf("No nodes present.\n");
        return NULL;
    }

    int cmp = compareDates(date, root->record->date);

    if (cmp < 0) {
        root->left = deleteNode(root->left, date);
    } else if (cmp > 0) {
        root->right = deleteNode(root->right, date);
    } else {
        if(!root->left || !root->right) {
            Node* temp = root->left ? root->left : root->right;
            free(root);
            return temp;
        }

        Node* junior = minNode(root->right);
        swapNodes(root, junior);
        root->right = deleteNode(root->right, junior->record->date);
    }

    return balance(root);
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

    while(choice!=5) {

        choice = printMenu();

        switch(choice) {
            case 1:
                inorder(root);
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
                Node* found = search(root, date);
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
                root = modify(root, date, cumulative);
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
                root = deleteNode(root, date);
                break;
            case 5:
                printf("Ending program. . .\n");
                break;
            default:
                printf("Invalid choice.\n");
        }
    }

    free(original);

    return 0;
}