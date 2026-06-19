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

typedef struct {
    Node **buckets;
    int m;
    int totalRecords;
}HashMap;


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

int hash_date(char* date, int m) {
    unsigned long sum = 0;
    for(int i = 0; i < DATE-1; i ++) {
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
    map->buckets = calloc(m,sizeof(Node *));
    if(!map->buckets) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    return map;
}

void destroyMap(HashMap* map) {
    for(int i = 0; i < map->m; i++) {
        Node* now = map->buckets[i];
        while(now) {
            Node* temp = now->next;
            free(now);
            now = temp;
        }
    }

    free(map->buckets);
    free(map);
}

void insertToMap(HashMap* map, Record* value) {
    int index = hash_date(value->date, map->m);

    Node* newNode = malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    newNode->record = value;
    newNode->next = map->buckets[index];
    map->buckets[index] = newNode;
    map->totalRecords++;
}


// ==============================================
// Helping functions
// ==============================================

int printMenu() {
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
// Menu functions
// ==============================================


int searchByDate(HashMap* map, char* date) {
    int index = hash_date(date, map->m);
    Node *now = map->buckets[index];
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
    Node *now = map->buckets[index]; 
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
    Node *now = map->buckets[index]; 
    Node *before = NULL; 
    bool found = false;

    while(now) {
        if(strcmp(now->record->date, date) == 0) {
            Node* deleting = now;
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

    HashMap* map = createMap(DATE);

    for(int i=0; i<n; i++) {
        insertToMap(map, &original[i]);
    }

    int choice = 0;
    char date[DATE+4];

    while(choice!=4) {

        choice = printMenu();
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

    return 0;
}