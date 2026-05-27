#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>


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
    
    while(1) {
        int largest = index;
        int li = 2*index + 1;
        int ri = 2*index + 2;

        if (li < size && data[li].value > data[largest].value) {
            largest = li;
        }

        if (ri < size && data[ri].value > data[largest].value) {
            largest = ri;
        }

        if(largest == index) {
            break;
        }

        swap_Records(data, largest, index);
        index = largest;
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

int bis(Record* input, int low, int high, long long key) {
    int right = high;
    int left = low;

    while (left <= right && key >= input[left].value && key <= input[right].value) {
        if (left == right) {
            return (input[left].value == key) ? left : -1;
        }
        double size = right - left + 1;

        int next = left + (int)((double)(key-input[left].value) / (input[right].value-input[left].value) * (right - left));

        if (next < left || next > right) {
            return -1;
        }

        if (key == input[next].value) {
            return next;
        } 

        if (key > input[next].value) {
            int i = 1;
            int up = (int)sqrt(size);
            while (next + i * up <= right && key > input[next + i * up].value) {
                i++;
            }
            right = (next + i * up <= right) ? next + i * up : right;
            left = next + (i-1)*up + 1;
        } 
        else {
            int i = 1;
            int up = (int)sqrt(size);
            while (next - i * up >= left && key < input[next - i*up].value){
                i++;
            }
            right = next - (i-1)*up - 1;
            left = (next - i * up >= left) ? next - i * up : left;
        }
        next = left + (int)((double)(key - input[left].value)/(input[right].value - input[left].value)*(right-left));
    }
    

    return -1;
}

// BIS*
int bis_improved(Record* input, int low, int high, long long key) {
    int right = high;
    int left = low;

    while (left <= right && key >= input[left].value && key <= input[right].value) {
        if (left == right) {
            return (input[left].value == key) ? left : -1;
        }
        double size = right - left + 1;

        int next = left + (int)((double)(key-input[left].value) / (input[right].value-input[left].value) * (right - left));

        if (next < left || next > right) {
            return -1;
        }

        if (key == input[next].value) {
            return next;
        } 

        if (key > input[next].value) {
            int i = 1;
            int up = (int)sqrt(size);
            while (next + i * up <= right && key > input[next + i * up].value) {
                i*=2;
            }
            right = (next + i * up <= right) ? next + i * up : right;
            left = next + (i/2)*up + 1;
        } 
        else {
            int i = 1;
            int up = (int)sqrt(size);
            while (next - i * up >= left && key < input[next - i*up].value){
                i*=2;
            }
            right = next - (i/2)*up;
            left = (next - i*up >= left) ? next - i*up : left;
        }
        next = left + (int)((double)(key - input[left].value)/(input[right].value - input[left].value)*(right-left));
    }

    return -1;
}



int main() {
    const char *filename = "effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv";
    
    //load data
    Record *original = malloc(MAX_ROWS*sizeof(Record)); 
    Record *sorted = malloc(MAX_ROWS*sizeof(Record));
    
    
    if (!original || !sorted) {
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
    
    long long search;
    printf("Value: ");
    scanf("%lld", &search);
    printf("\n\n");
    
    // sort by value
    heapSort(sorted, n);

    //BIS
    clock_t bis_start = clock();
    int bis_res = bis(sorted, 0, n-1, search);
    clock_t bis_end = clock();
    double bis_time = (double)(bis_end - bis_start) / CLOCKS_PER_SEC;

    if(bis_res >= 0) {
        printf("BIS found the value at index %d\n",bis_res);
    } else {
        printf("BIS: value not found\n");
    }
    printf("BIS Time: %.6f seconds\n\n", bis_time);
    
    //BIS*
    clock_t bis_imp_start = clock();
    int bis_imp_res = bis_improved(sorted, 0, n-1, search);
    clock_t bis_imp_end = clock();
    double bis_imp_time = (double)(bis_imp_end - bis_imp_start) / CLOCKS_PER_SEC;

    if(bis_imp_res >= 0) {
        printf("BIS* found the value at index %d\n",bis_res);
    } else {
        printf("BIS*: value not found\n");
    }
    printf("BIS* Time: %.6f seconds\n\n", bis_imp_time);

    
    // Comparison
    printf("BIS : %.6f sec | O(log(logn))\n", bis_time);
    printf("BIS* : %.6f sec | O(log(logn))\n", bis_imp_time);
    if (bis_time < bis_imp_time) {
        printf("Fastest:  BIS\n");
    } else {
        printf("Fastest:  BIS*\n");
    }


    //Free memory
    free(original); 
    free(sorted);

    printf("\nPress Enter to exit the program.\n");
    getchar();
    getchar();

    return 0;
}