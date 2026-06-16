/* 
* Υλοποιήστε το (Α) κάνοντας χρήση HASHING με αλυσίδες, αντί BST. Η συνάρτηση κατακερματισμού θα υπολογίζεται ως το 
* υπόλοιπο (modulo) της διαίρεσης του αθροίσματος των κωδικών ASCII των επιμέρους χαρακτήρων που απαρτίζουν την 
* ΗΜΕΡΟΜΗΝΙΑ με ένα περιττό αριθμό m που συμβολίζει το πλήθος των κάδων (buckets). Π.χ. για ΗΜΕΡΟΜΗΝΙΑ=”
* 15/12/2021” και m=11, ισχύει:
* 
* Hash("15/12/2021")= [ASCII(‘1’)+ ASCII(‘5’)+ ASCII(‘/’)+ ASCII(‘1’)+ ASCII(‘2’)+ ASCII(‘/’)+ ASCII(‘2’)+ ASCII(‘0’)+ 
* ASCII(‘2’)+ ASCII(‘1’)] mod 11.
* 
* Το πρόγραμμα θα εμφανίζει ένα μενού με τις ακόλουθες επιλογές:
*
* 1. Αναζήτηση Τιμής Cumulative βάσει της ΗΜΕΡΟΜΗΝΙΑΣ που θα δίνεται από το χρήστη.
* 2. Τροποποίηση των στοιχείων εγγραφής βάσει ΗΜΕΡΟΜΗΝΙΑΣ που θα δίνεται από το χρήστη. 
*     Η τροποποίηση προφανώς αφορά ΜΟΝΟ το πεδίο Cumulative.
* 3. Διαγραφή μιας εγγραφής από τον πίνακα κατακερματισμού βάσει ΗΜΕΡΟΜΗΝΙΑΣ που θα δίνεται από το χρήστη.
* 4. Έξοδος από την εφαρμογή.  
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
#define DATE 11
#define WEEKDAY 10
#define MAX_STR 64
#define MAX_ROWS 111500


//============================================ 
// Structures
//============================================ 

typedef struct {
    char direction[DIRECTION];
    char year[YEAR];
    char date[DATE];
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

    struct Node* next;
} Node;


// ==============================================
// Function Callings
// ==============================================


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

        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.date, temp, DATE-1);

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
// Hash related functions
// ==============================================


void initializeMap( hashMap* map, int arraySize) {
    map->capacity = MAX_ROWS;
    map->numOfElements = arraySize;

    map->array = (Node**)malloc(sizeof(Node*)*map->capacity);
}

int hash(char* date) {
    int sum = 0;
    for(int i = 0; i < DATE; i ++) {
        sum += date[i];
    }
    return (sum % DATE);
}


// ==============================================
// Node functions
// ==============================================


Node* createNode(Record* value) {
    Node* newNode = malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->record  = value;
    newNode->next  = NULL;
    return newNode;
}


// ==============================================
// Helping functions
// ==============================================


int compareDates(int* x, int* y) {
    if(x[2] != y[2]) return x[2] - y[2];
    if(x[1] != y[1]) return x[1] - y[1];
    return x[0] - y[0];
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


int insert(Node** table, Record)


// ==============================================
// Main function
// ==============================================


int main() {
    const char *filename = "effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv";
    
    

    
    printf("\nPress Enter to exit the program.\n");
    getchar();

    return 0;
}