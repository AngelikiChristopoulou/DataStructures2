#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_ROWS 111440
#define DIRECTION 8
#define YEAR 5
#define DATE 11
#define WEEKDAY 10
#define MAX_STR 64
#define TRIALS 50000

// ===================== STRUCT =====================
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

// ===================== DATE CONVERSION =====================
long long dateToInt(const char *date) {
    int d, m, y;
    sscanf(date, "%d/%d/%d", &d, &m, &y);
    return (long long)y * 10000 + m * 100 + d;
}

// ===================== LOAD CSV =====================
int load_csv(const char *filename, Record *data) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    char line[512];
    int n = 0;

    fgets(line, sizeof(line), f); // skip header

    while (fgets(line, sizeof(line), f) && n < MAX_ROWS) {
        Record r;
        memset(&r, 0, sizeof(r));

        char *t = strtok(line, ","); strcpy(r.direction, t);
        t = strtok(NULL, ","); strcpy(r.year, t);
        t = strtok(NULL, ","); strcpy(r.date, t);
        t = strtok(NULL, ","); strcpy(r.weekday, t);
        t = strtok(NULL, ","); strcpy(r.country, t);
        t = strtok(NULL, ","); strcpy(r.commodity, t);
        t = strtok(NULL, ","); strcpy(r.transport_mode, t);
        t = strtok(NULL, ","); strcpy(r.measure, t);
        t = strtok(NULL, ","); r.value = atoll(t);
        t = strtok(NULL, ","); r.cumulative = atoll(t);

        data[n++] = r;
    }

    fclose(f);
    return n;
}

// ===================== SWAP =====================
void swap(Record *a, Record *b) {
    Record tmp = *a;
    *a = *b;
    *b = tmp;
}

// ===================== HEAP SORT BY DATE =====================
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

// ===================== BIS =====================
int bis(Record *a, int low, int high, long long key) {
    while (low <= high) {

        long long leftKey = dateToInt(a[low].date);
        long long rightKey = dateToInt(a[high].date);

        if (key < leftKey || key > rightKey)
            return -1;

        if (low == high)
            return (leftKey == key) ? low : -1;

        int next = low + (int)(
            ((double)(key - leftKey) /
            (rightKey - leftKey)) * (high - low)
        );

        long long nextKey = dateToInt(a[next].date);

        if (nextKey == key)
            return next;

        int step = (int)sqrt(high - low + 1);

        if (key > nextKey) {
            while (next + step <= high &&
                   dateToInt(a[next + step].date) < key)
                next += step;

            low = next + 1;
        } else {
            while (next - step >= low &&
                   dateToInt(a[next - step].date) > key)
                next -= step;

            high = next - 1;
        }
    }
    return -1;
}

// ===================== BIS* =====================
int bis_star(Record *a, int low, int high, long long key) {
    while (low <= high) {

        long long leftKey = dateToInt(a[low].date);
        long long rightKey = dateToInt(a[high].date);

        if (key < leftKey || key > rightKey)
            return -1;

        if (low == high)
            return (leftKey == key) ? low : -1;

        int next = low + (int)(
            ((double)(key - leftKey) /
            (rightKey - leftKey)) * (high - low)
        );

        long long nextKey = dateToInt(a[next].date);

        if (nextKey == key)
            return next;

        int step = (int)sqrt(high - low + 1);

        int jump = 1;

        if (key > nextKey) {
            while (next + jump * step <= high &&
                   dateToInt(a[next + jump * step].date) < key)
                jump *= 2;

            low = next + (jump/2)*step + 1;
        } else {
            while (next - jump * step >= low &&
                   dateToInt(a[next - jump * step].date) > key)
                jump *= 2;

            high = next - (jump/2)*step - 1;
        }
    }
    return -1;
}

// ===================== MAIN =====================
int main() {

    const char *file =
        "effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv";

    Record *data = malloc(MAX_ROWS * sizeof(Record));
    Record *sorted = malloc(MAX_ROWS * sizeof(Record));

    int n = load_csv(file, data);
    if (n <= 0) {
        printf("Error loading file\n");
        return 1;
    }

    printf("Loaded %d records.\n", n);

    memcpy(sorted, data, n * sizeof(Record));
    heapSort(sorted, n);

    char input[16];
    printf("Enter Date (dd/mm/yyyy): ");
    scanf("%s", input);

    long long key = dateToInt(input);

    // ===================== REAL SEARCH =====================
    int r1 = bis(sorted, 0, n-1, key);
    int r2 = bis_star(sorted, 0, n-1, key);

    if (r1 != -1) {
        printf("\nBIS FOUND:\n");
        printf("Date: %s\n", sorted[r1].date);
        printf("Value: %lld\n", sorted[r1].value);
        printf("Cumulative: %lld\n", sorted[r1].cumulative);
    } else {
        printf("\nBIS NOT FOUND\n");
    }

    if (r2 != -1) {
        printf("\nBIS* FOUND:\n");
        printf("Date: %s\n", sorted[r2].date);
        printf("Value: %lld\n", sorted[r2].value);
        printf("Cumulative: %lld\n", sorted[r2].cumulative);
    } else {
        printf("\nBIS* NOT FOUND\n");
    }

    // ===================== TIMING =====================
    volatile int sink = 0;

    clock_t start = clock();
    for (int i = 0; i < TRIALS; i++)
        sink += bis(sorted, 0, n-1, key);
    clock_t end = clock();

    double t1 = (double)(end - start) / CLOCKS_PER_SEC / TRIALS;

    clock_t start2 = clock();
    for (int i = 0; i < TRIALS; i++)
        sink += bis_star(sorted, 0, n-1, key);
    clock_t end2 = clock();

    double t2 = (double)(end2 - start2) / CLOCKS_PER_SEC / TRIALS;

    printf("\n================ RESULTS ================\n");
    printf("BIS Time  : %.12f sec\n", t1);
    printf("BIS* Time : %.12f sec\n", t2);

    printf("\nSink: %d\n", sink);

    printf("\nFastest: %s\n",
           (t1 < t2) ? "BIS" : "BIS*");

    free(data);
    free(sorted);

    return 0;
}