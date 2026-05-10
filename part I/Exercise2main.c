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


// calling functions
int load_csv(const char *filename, Record *data);


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


// Heap Sort

void switch_Records(Record* x, int x_i, Record* y, int y_i) {
    Record temp;

    memcpy(&temp, &x[x_i], sizeof(Record));
    memcpy(&x[x_i], &y[y_i], sizeof(Record));
    memcpy(&y[y_i], &temp, sizeof(Record));
}


void heapify(Record *data, int size, int index) {
    int largest = index;
    
    int li = 2*index + 1;
    int ri = 2*index + 2;

    if (li < size && data[li].value > data[largest].value) {
        largest = li;
    }

    if (ri < size && data[ri].value > data[largest].value) {
        largest = ri;
    }

    if (largest != index) {
        switch_Records(data, largest, data, index);
        heapify(data, size, largest);
    }

}

void heap_sort (Record *input, int size) {
    if (size <= 0) {
        return;
    }
    bool sorted = false;
    int n = size-1;
    int half = size / 2 - 1;

    for(int i= half; i >= 0; i--) {
        heapify(input, size, i);
    }

    for(int i = size-1; i>0; i--) {
        switch_Records(input, 0, input, i);
        heapify(input, i, 0);
    }
}

int main() {
    const char *filename = "effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv";

    Record *original = malloc(MAX_ROWS*sizeof(Record));
    Record *temp = malloc(MAX_ROWS*sizeof(Record));
    if (!original || !temp) {
        printf("Memory Error.\n");
        return 1;
    }

    int recordings= load_csv(filename, original);
    if (recordings<=0) {
        free(original);
        return 1;
    }

    printf("Loaded %d recordings from CSV.\n", recordings);

    memcpy(temp, original, recordings*sizeof(Record));
    heap_sort(temp, recordings);

    for (int i = recordings -10; i <= recordings; i++) {
        printf("i %lld, o %lld\n", original[i].value, temp[i].value);
    }


    return 0;
}
