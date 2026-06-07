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
#define MAX_ROWS 111500


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
// Hash related functions
// ==============================================





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





// ==============================================
// Main function
// ==============================================


int main() {
    const char *filename = "effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv";
    
    //load data
    Record *original = malloc(MAX_ROWS*sizeof(Record));
    Record recordings[MAX_ROWS];
    
    if (!original) {
        printf("Memory Error\n");
        return 1;
    }
    
    int n = load_csv(filename, original);
    if (n<=0) {
        free(original);
        return 1;
    }

    int choice = 0;
    int date[3];
    long long cumulative;

    while(choice!=5) {

        choice = printMenu();

        switch(choice) {
            default:
                printf("Invalid choice.\n");
        }
    }

    free(original);

    return 0;
}