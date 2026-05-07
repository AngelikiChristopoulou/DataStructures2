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

static Record* temp_buffer = NULL;


// calling functions
int load_csv(const char *filename, Record *data);
void counting_sort(Record *input, int recordings, Record *output);
void merge(Record* array, int left, int current, int right);


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
    printf("%lld min \n %lld max", min, max);

    long long range = max - min + 1;

    // too big check
    if (range > 10000000LL) {
        printf("WARNING: range is too wide (%lld).\n", range);
        printf("Counting sort not suited for the input");
        memcpy(output, input, recordings * sizeof(Record));
        return;
    }

    // count array
    int *count = (int *)calloc((size_t)range, sizeof(int));
    if (!count) {
        printf("Memory problem\n");
        return;
    }

    //counting
    for(int i=0;i<recordings;i++) {
        count[input[i].cumulative-min]++;
    }

    //summing
    for(long long i=1; i<range; i++) {
        count[i] += count[i-1];
    }

    // building output
    for(int i=recordings-1; i>=0; i--) {
        long long index = input[i].cumulative - min;
        count[index]--;
        output[count[index]] = input[i];
    }

    free(count);
}


// Merge sort
void merge(Record* array, int left, int middle, int right){
    int l = left;
    int r = middle+1;
    int out = left;

    while( l <= middle && r <= right) {
        if (array[l].cumulative <= array[r].cumulative) {
            temp_buffer[out++] = array[r++];
        } else {
            temp_buffer[out++] = array[l++];
        }
    }

    while (l <= middle ) {
        temp_buffer[out++] = array[l++];
    }

    while (r <= right ) {
        temp_buffer[out++] = array[l++];
    }
}

void merge_sort(Record* array, int left, int right) {
    if (left >= right) {
        return;
    }

    int middle = (left + right) / 2;
    merge_sort(array, left, middle);
    merge_sort(array, middle + 1, right);
    merge(array, left, middle, right);
}

// printing
void print_records(Record *data, int recordings, int limit) {
    if (limit > recordings) {
        limit = recordings;
    }
    
    for (int i=0; i<limit; i++) {
        printf("[%3d] Date: %-12s | Cumulative: %lld\n");
    }
}

int main() {
    const char *filename = "effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv";

    //load data
    Record *original = (Record*)malloc(MAX_ROWS*sizeof(Record));
    Record *count_in = (Record*)malloc(MAX_ROWS*sizeof(Record));
    Record *merge_data = (Record*)malloc(MAX_ROWS*sizeof(Record));
    Record *count_out = (Record*)malloc(MAX_ROWS*sizeof(Record));
    temp_buffer = (Record*)malloc(MAX_ROWS*sizeof(Record));

    if (!original || !count_in || !count_out || !merge_data || !temp_buffer) {
        printf("Memory Error\n");
        return 1;
    }

    int n= load_csv(filename, original);
    if (n<=0) {
        free(original);
        return 1;
    }

    printf("Loaded %d recordings from CSV.\n", n);

    memcpy(count_in, original, n*sizeof(Record));
    memcpy(merge_data, original, n*sizeof(Record));

    // time counting sort
    clock_t count_start = clock();
    counting_sort(count_in, n, count_out);
    clock_t count_end = clock();
    double count_time = (double)(count_end - count_start) / CLOCKS_PER_SEC;

    printf("Counting first 10 recordings.\n");
    print_records(count_out, n, 10);
    printf("Counting Sort Time: $.6f seconds\n\n", count_time);

    // time merge sort
    clock_t merge_start = clock();
    merge_sort(merge_data, 0, n-1);
    clock_t merge_end = clock();
    double merge_time = (double)(merge_end - merge_start) / CLOCKS_PER_SEC;

    printf("Merging first 10 recordings.\n");
    print_records(merge_data, n, 10);
    printf("Merging Sort Time: $.6f seconds\n\n", merge_time);


    // Comparison
    printf("Counting Sort: %.6f sec | O(n+k)\n", count_time);
    printf("Merging Sort: %.6f sec | O(n*logn)\n", merge_time);
    if (count_time < merge_time) {
        printf("Fastest:  Counting Sort\n");
    } else {
        printf("Fastest:  Merginf Sort\n");
    }


    //Free memory
    free(original);
    free(count_in);    
    free(count_out);
    free(merge_data);
    free(temp_buffer);

    return 0;
}