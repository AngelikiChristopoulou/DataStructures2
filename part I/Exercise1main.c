#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // used to count the time each algorithm takes

#define DIRECTION 8
#define YEAR 5
#define DATE 11
#define WEEKDAY 10
#define MAX_STR 64
#define MAX_ROWS 111440

// Δομ΄η Εγγραφ΄ης 
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


// Φόρτωση στοιχείων
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

// Counting Sort βασή cumulative
void counting_sort(Record *input, int recordings, Record *output) {
    if (recordings <= 0) return;
    

    //Step 1: find min + max value
    long long min = input[0].cumulative;
    long long max = input[0].cumulative;
    for (int i=1; i<recordings; i++) {
        if (input[i].cumulative<min) {
            min = input[i].cumulative;
        }
        if (input[i].cumulative>max) {
            max = input[i].cumulative;
        }
    }

    long long range = max - min + 1;

    // too big check
    if (range > 10000000LL) {
        printf("WARNING: range is too wide (%lld).\n", range);
        printf("Counting sort not suited for the input");
        memcpy(output, input, recordings * sizeof(Record));
    }
}

int main() {
    int x;
    Record* data;
    x=load_csv("effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv", data);
    counting_sort(data, x, data);
    return 0;
}