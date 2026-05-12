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

//===========================================
// search by Value
//===========================================

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

//===========================================
// search by Value
//===========================================
// Binary Search

int binarySearch(Record* input, int n, long long x) {
    int low = 0;
    int high = n-1;

    while (low<=high) {
        int mid = low + (high - low) / 2;

        if(input[mid].value == x) {
            return mid;
        }

        if (input[mid].value<x) {
            low = mid +1;
        } else {
            high = mid-1;
        }
    }

    return -1;
}

// Inerpolation Search

int interpolationSearch(Record* input, int low, int high, long long x) {
    int pos;

    if (low <= high && x >= input[low].value && x <= input[high].value) {
        pos = low + (((double)(high-low)/(input[high].value-input[low].value))*(x-input[low].value));

        if (input[pos].value == x) {
            return pos;
        }

        if (input[pos].value <= x) {
            return interpolationSearch(input, pos+1, high, x);
        }

        if (input[pos].value > x) {
            return interpolationSearch(input, low, pos-1, x);
        }
    }

    return -1;
}

int main() {
    const char *filename = "effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv";

    //load data
    Record *original = malloc(MAX_ROWS*sizeof(Record));
    Record *binary_search = malloc(MAX_ROWS*sizeof(Record));
    Record *interpolation_search = malloc(MAX_ROWS*sizeof(Record));
    Record *sorted = malloc(MAX_ROWS*sizeof(Record));
    

    if (!original || !binary_search || !interpolation_search || !sorted) {
        printf("Memory Error\n");
        return 1;
    }

    int n= load_csv(filename, original);
    if (n<=0) {
        free(original);
        return 1;
    }

    printf("Loaded %d recordings from CSV.\n", n);

    memcpy(sorted, original, n*sizeof(Record));

    // sort by value
    heapSort(sorted, n);
    long long search;
    printf("Value: ");
    scanf("%lld", &search);
    printf("\n\n");

    memcpy(binary_search, sorted, n*sizeof(Record));
    memcpy(interpolation_search, sorted, n*sizeof(Record));

    // time binary search
    clock_t binary_start = clock();
    binarySearch(binary_search, n, search);
    clock_t binary_end = clock();
    double binary_time = (double)(binary_end - binary_start) / CLOCKS_PER_SEC;

    printf("Binary Search Time: %.6f seconds\n\n", binary_time);

    // time interpolation search
    clock_t inter_start = clock();
    interpolationSearch(interpolation_search, 0, n-1, search);
    clock_t inter_end = clock();
    double inter_time = (double)(inter_end - inter_start) / CLOCKS_PER_SEC;
    
    printf("Interpolation Search Time: %.6f seconds\n\n", inter_time);


    // Comparison
    printf("Binary Search: %.6f sec | O(logn)\n", binary_time);
    printf("Interpolation Search: %.6f sec | O(log(logn))\n", inter_time);
    if (binary_time < inter_time) {
        printf("Fastest:  Binary Search\n");
    } else {
        printf("Fastest:  Interpolation Search\n");
    }


    //Free memory
    free(original);
    free(binary_search);    
    free(interpolation_search);
    free(sorted);

    return 0;
}