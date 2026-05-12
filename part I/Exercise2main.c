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
        printf("[%3d] Date: %-12s | Cumulative: %lld\n", i+1, data[i].date, data[i].cumulative);
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
    bool sorted = false;
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

int parition(Record* data, int low, int high){
    long long piv = data[low].value; // Pivot = first element
    int i = low;
    int j = high;

    while(i < j) {
        while(data[i].value <= piv && i <= high - 1) {
            i++;
        }

        while(data[j].value > piv && j >= low + 1) {
            j--;
        }

        if(i < j) {
            swap_Records(data, i, j);
        }
    }

    swap_Records(data, low, j);
    return j;
}

void quickSort(Record* data, int low, int high) {
    if(low < high) {

        int pi = parition(data, low, high);

        quickSort(data, low, pi-1);
        quickSort(data, pi + 1, high);
    }
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


    //Free memory
    free(original);
    free(heap_sort);    
    free(quick_sort);

    return 0;
}
