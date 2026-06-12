#include <stdio.h>
#include <stdlib.h>
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
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    char line[512];
    int n = 0;

    fgets(line, sizeof(line), f); // header

    while (fgets(line, sizeof(line), f) && n < MAX_ROWS) {
        Record r;
        memset(&r, 0, sizeof(r));

        char *t = strtok(line, ",");
        strcpy(r.direction, t);

        t = strtok(NULL, ",");
        strcpy(r.year, t);

        t = strtok(NULL, ",");
        strcpy(r.date, t);

        t = strtok(NULL, ",");
        strcpy(r.weekday, t);

        t = strtok(NULL, ",");
        strcpy(r.country, t);

        t = strtok(NULL, ",");
        strcpy(r.commodity, t);

        t = strtok(NULL, ",");
        strcpy(r.transport_mode, t);

        t = strtok(NULL, ",");
        strcpy(r.measure, t);

        t = strtok(NULL, ",");
        r.value = atoll(t);

        t = strtok(NULL, ",");
        r.cumulative = atoll(t);

        data[n++] = r;
    }

    fclose(f);
    return n;
}

// helpig functions

void swap_Records(Record* data, int i, int j) {
    Record temp;

    memcpy(&temp, &data[i], sizeof(Record));
    memcpy(&data[i], &data[j], sizeof(Record));
    memcpy(&data[j], &temp, sizeof(Record));
}

// printing
void print_records(Record *data, int recordings, int limit) {
    if (limit > recordings) {
        limit = recordings;
    }
    
    for (int i=0; i<limit; i++) {
        printf("[%3d] Date: %-12s | Value: %lld\n", i+1, data[i].date, data[i].value);
    }
}


// Heap Sort

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
        swap_Records(data, largest, index);
        heapify(data, size, largest);
    }

}

void heapSort (Record *input, int size) {
    if (size <= 0) {
        return;
    }
    int n = size-1;
    int half = size / 2 - 1;

    for(int i= half; i >= 0; i--) {
        heapify(input, size, i);
    }

    for(int i = size-1; i>0; i--) {
        swap_Records(input, 0, i);
        heapify(input, i, 0);
    }
}

// Quick Sort
void partition_value(Record *data, int lo, int hi, int *lt, int *gt)
{
    long long pivot = data[lo].value;

    int i = lo;
    *lt = lo;
    *gt = hi;

    while (i <= *gt) {
        if (data[i].value < pivot) {
            swap_Records(data, i, *lt);
            (*lt)++;
            i++;
        }
        else if (data[i].value > pivot) {
            swap_Records(data, i, *gt);
            (*gt)--;
        }
        else {
            i++;
        }
    }
}

void quickSort(Record *data, int lo, int hi)
{
    if (lo >= hi) return;

    int lt, gt;
    partition_value(data, lo, hi, &lt, &gt);

    quickSort(data, lo, lt - 1);
    quickSort(data, gt + 1, hi);
}


int main() {
    const char *filename = "effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv";

    //load data
    Record *original = malloc(MAX_ROWS*sizeof(Record));
    Record *heap_sort = malloc(MAX_ROWS*sizeof(Record));
    Record *quick_sort = malloc(MAX_ROWS*sizeof(Record));

    if (!original || !heap_sort || !quick_sort) {
        printf("Memory Error\n");
        return 1;
    }

    int n= load_csv(filename, original);
    if (n<=0) {
        free(original);
        return 1;
    }

    printf("Loaded %d recordings from CSV.\n", n);

    memcpy(heap_sort, original, n*sizeof(Record));
    memcpy(quick_sort, original, n*sizeof(Record));

    // time heap sort
    clock_t heap_start = clock();
    heapSort(heap_sort, n);
    clock_t heap_end = clock();
    double heap_time = (double)(heap_end - heap_start) / CLOCKS_PER_SEC;

    printf("Heap sort first 10 recordings.\n");
    print_records(heap_sort, n, 10);
    printf("Heap Sort Time: %.6f seconds\n\n", heap_time);

    // time quick sort
    clock_t quick_start = clock();
    quickSort(quick_sort, 0, n-1);
    clock_t quick_end = clock();
    double quick_time = (double)(quick_end - quick_start) / CLOCKS_PER_SEC;

    printf("Quick sort first 10 recordings.\n");
    print_records(quick_sort, n, 10);
    printf("Quick Sort Time: %.6f seconds\n\n", quick_time);


    // Comparison
    printf("Heap Sort: %.6f sec | O(n*logn)\n", heap_time);
    printf("Quick Sort: %.6f sec | O(n*logn)\n", quick_time);
    if (heap_time < quick_time) {
        printf("Fastest:  Heap Sort\n");
    } else {
        printf("Fastest:  Quick Sort\n");
    }

    printf("\nPress Enter to exit the program.\n");
    getchar();

    //Free memory
    free(original);
    free(heap_sort);
    free(quick_sort);

    return 0;
}
