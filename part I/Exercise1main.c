#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // used to count the time each algorithm takes

#define DIRECTION 7
#define YEAR 4
#define DATE 8
#define WEEKDAY 9
#define MAX_STR 64
#define MAX_ROWS 111440

// Δομ΄η Εγγραφ΄ης 
typedef struct {
    char direction[DIRECTION];
    int year[YEAR];
    int date[DATE];
    char weekday[WEEKDAY];
    char country[MAX_STR];
    char commodity[MAX_STR];
    char transport_mode[MAX_STR];
    char measure[MAX_STR];
    long long value;
    long long cumulative;
} Record;


// Φόρτωση στοιχείων
int load_csv(const char *filename, Record *data) {
    FILE *fp = fopen(filename, "r");
    if(!fp) {
        printf("[ERROR] No file %s found.\n", filename);
        return -1;
    }

    char line[512]; //temp info saver
    int count = 0;

    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp) && count < MAX_ROWS) {
        line[strcspn(line, "\r\n")] = 0;

        char *token;
        Record r;
        memset(&r, 0, sizeof(r));

        token = strtok(line, ",");
        if(!token) continue;
        strncpy(r.direction, token, DIRECTION-1);

        token = strtok(line, ",");
        if(!token) continue;
        strncpy(r.date, token, MAX_STR-1);

    }
}
