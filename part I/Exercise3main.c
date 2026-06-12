#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ROWS 111440
#define MAX_STR 64
#define TRIALS 50000

typedef struct {
    char direction[16];
    char year[8];
    char date[16];
    char weekday[16];
    char country[MAX_STR];
    char commodity[MAX_STR];
    char transport_mode[MAX_STR];
    char measure[MAX_STR];
    long long value;
    long long cumulative;
} Record;

// ================= DATE -> INTEGER (KEY) =================
long long dateToInt(const char *date) {
    int d, m, y;
    sscanf(date, "%d%*c%d%*c%d", &d, &m, &y);
    return (long long)y * 10000 + m * 100 + d;
}

// ================= LOAD CSV =================
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

// ================= SWAP =================
void swap(Record *a, Record *b) {
    Record tmp = *a;
    *a = *b;
    *b = tmp;
}

// ================= HEAP SORT (by DATE) =================
void heapify(Record *a, int n, int i) {
    while (1) {
        int largest = i;
        int l = 2*i + 1;
        int r = 2*i + 2;

        if (l < n && dateToInt(a[l].date) > dateToInt(a[largest].date))
            largest = l;

        if (r < n && dateToInt(a[r].date) > dateToInt(a[largest].date))
            largest = r;

        if (largest == i) break;

        swap(&a[i], &a[largest]);
        i = largest;
    }
}

void heapSort(Record *a, int n) {
    for (int i = n/2 - 1; i >= 0; i--)
        heapify(a, n, i);

    for (int i = n - 1; i > 0; i--) {
        swap(&a[0], &a[i]);
        heapify(a, i, 0);
    }
}

// ================= BINARY SEARCH (by DATE) =================
int binarySearch(Record *a, int n, long long key) {
    int l = 0, r = n - 1;

    while (l <= r) {
        int mid = l + (r - l) / 2;
        long long midKey = dateToInt(a[mid].date);

        if (midKey == key)
            return mid;
        else if (midKey < key)
            l = mid + 1;
        else
            r = mid - 1;
    }

    return -1;
}

// ================= INTERPOLATION SEARCH (by DATE) =================
int interpolationSearch(Record *a, int l, int r, long long key) {

    while (l <= r &&
           key >= dateToInt(a[l].date) &&
           key <= dateToInt(a[r].date)) {

        long long left = dateToInt(a[l].date);
        long long right = dateToInt(a[r].date);

        if (left == right) {
            return (left == key) ? l : -1;
        }

        int pos = l +
            (double)(r - l) *
            (key - left) /
            (right - left);

        long long posKey = dateToInt(a[pos].date);

        if (posKey == key)
            return pos;

        if (posKey < key)
            l = pos + 1;
        else
            r = pos - 1;
    }

    return -1;
}

// ================= MAIN =================
int main() {

    Record *data = malloc(MAX_ROWS * sizeof(Record));
    Record *sorted = malloc(MAX_ROWS * sizeof(Record));

    int n = load_csv("effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv", data);
    if (n <= 0) {
        printf("Error loading file\n");
        return 1;
    }

    memcpy(sorted, data, n * sizeof(Record));

    heapSort(sorted, n);

    char inputDate[16];
    printf("Enter Date (dd/mm/yyyy): ");
    scanf("%s", inputDate);

    long long key = dateToInt(inputDate);

    volatile int sink = 0;

    // ================= BINARY BENCHMARK =================
    clock_t start = clock();
    for (int i = 0; i < TRIALS; i++) {
        sink += binarySearch(sorted, n, key);
    }
    clock_t end = clock();

    double t1 = (double)(end - start) / CLOCKS_PER_SEC / TRIALS;

    // ================= INTERPOLATION BENCHMARK =================
    start = clock();
    for (int i = 0; i < TRIALS; i++) {
        sink += interpolationSearch(sorted, 0, n - 1, key);
    }
    end = clock();

    double t2 = (double)(end - start) / CLOCKS_PER_SEC / TRIALS;

    // ================= RESULT =================
    int idx = binarySearch(sorted, n, key);
    int r1 = binarySearch(sorted, n-1, key);
    int r2 = interpolationSearch(sorted, 0, n-1, key);

    if (r1 != -1) {
        printf("\nBinary Search FOUND:\n");
        printf("Date: %s\n", sorted[r1].date);
        printf("Value: %lld\n", sorted[r1].value);
        printf("Cumulative: %lld\n", sorted[r1].cumulative);
    } else {
        printf("\nBinary Search NOT FOUND\n");
    }

    if (r2 != -1) {
        printf("\nInterpolation Search FOUND:\n");
        printf("Date: %s\n", sorted[r2].date);
        printf("Value: %lld\n", sorted[r2].value);
        printf("Cumulative: %lld\n", sorted[r2].cumulative);
    } else {
        printf("\nInterpolation Search NOT FOUND\n");
    }

    printf("\nBinary Search: %.10f sec\n", t1);
    printf("Interpolation Search: %.10f sec\n", t2);
    printf("\nConclution: ");
    if (t1<t2) printf("Binary Search Faster\n"); else printf("Interpolation Search Faster\n");

    free(data);
    free(sorted);

    return 0;
}