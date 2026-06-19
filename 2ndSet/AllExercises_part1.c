/*
 * ============================================================
 *  All Exercises Combined (1-4)
 *  Dataset: effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv
 * ============================================================
 *
 *  Exercise 1: Counting Sort vs Merge Sort  (sorted by Cumulative field)
 *  Exercise 2: Heap Sort   vs Quick Sort    (sorted by Value field)
 *  Exercise 3: Binary Search vs Interpolation Search (search by Value)
 *  Exercise 4: BIS vs BIS*                          (search by Value)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* Buffer size constants sized to fit the fixed-width CSV fields */
#define DIRECTION  8
#define YEAR       5
#define DATE      11
#define WEEKDAY   10
#define MAX_STR   64
#define MAX_ROWS  111440

#define TRIALS 50000

/* One row from the CSV becomes one Record.
   Value and Cumulative are kept as 64-bit integers because the
   trade figures exceed the range of a 32-bit int. */
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

/* Merge sort needs scratch space equal to the full array size.
   Allocating it once globally avoids repeated malloc/free inside
   the recursive calls, which would dominate the measured time. */
static Record *temp_buffer = NULL;


/* ============================================================
 *  SHARED UTILITY FUNCTIONS
 * ============================================================ */

/* Reads every data row from the CSV into the pre-allocated data array.
   Returns the number of rows successfully parsed. */
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

/* In-place swap of two records by position. */
void swap_Records(Record *data, int i, int j) {
    Record tmp;
    memcpy(&tmp,     &data[i], sizeof(Record));
    memcpy(&data[i], &data[j], sizeof(Record));
    memcpy(&data[j], &tmp,     sizeof(Record));
}

/* Print the first 'limit' records showing Date, Value and Cumulative.
   Useful for a quick sanity-check that the sort produced the right order. */
void print_records(Record *data, int n, int limit) {
    if (limit > n) limit = n;
    for (int i = 0; i < limit; i++)
        printf("[%3d] Date: %-12s | Value: %10lld | Cumulative: %lld\n",
               i+1, data[i].date, data[i].value, data[i].cumulative);
}

long long dateToInt(const char *date) {
    int d, m, y;
    sscanf(date, "%d%*c%d%*c%d", &d, &m, &y);
    return (long long)y * 10000 + m * 100 + d;
}

static double time_diff(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1e9;
}
/* ============================================================
 *  EXERCISE 1 – Counting Sort & Merge Sort  (by Cumulative)
 * ============================================================ */

/*
 * COUNTING SORT LOGIC:
 * Counting sort is a non-comparison sort. Instead of comparing elements
 * it counts how many times each distinct key value appears, then uses
 * those counts (as prefix sums) to place every record directly into its
 * correct output position in O(n + k) time, where k is the value range.
 *
 * Limitation: k must be small enough to allocate a count array.
 * The Cumulative field spans billions of dollars, making k enormous —
 * so the algorithm detects this and reports failure rather than crashing.
 */
void counting_sort(Record *input, int n, Record *output, bool *works)
{
    *works = true;  // IMPORTANT: initialize

    if (n <= 0) {
        *works = false;
        return;
    }

    /* Step 1: find min/max */
    long long mn = input[0].cumulative;
    long long mx = input[0].cumulative;

    for (int i = 1; i < n; i++) {
        if (input[i].cumulative < mn) mn = input[i].cumulative;
        if (input[i].cumulative > mx) mx = input[i].cumulative;
    }

    printf("  Cumulative min=%lld  max=%lld\n", mn, mx);

    long long range = mx - mn + 1;

    /* Step 2: safety check */
    if (range <= 0 || range > 10000000LL) {
        printf("  Value range too large for Counting Sort.\n");
        printf("  Falling back to copy.\n");

        memcpy(output, input, n * sizeof(Record));
        *works = false;
        return;
    }

    /* Step 3: allocate */
    int *cnt = calloc((size_t)range, sizeof(int));
    if (!cnt) {
        printf("  Memory allocation failed\n");
        memcpy(output, input, n * sizeof(Record));
        *works = false;
        return;
    }

    /* Step 4: counting */
    for (int i = 0; i < n; i++) {
        cnt[input[i].cumulative - mn]++;
    }

    /* Step 5: prefix sums */
    for (long long i = 1; i < range; i++) {
        cnt[i] += cnt[i - 1];
    }

    /* Step 6: stable placement */
    for (int i = n - 1; i >= 0; i--) {
        long long idx = input[i].cumulative - mn;
        output[--cnt[idx]] = input[i];
    }

    free(cnt);
}
/*
 * MERGE SORT LOGIC:
 * Merge sort is a divide-and-conquer algorithm.
 * It splits the array in half recursively until each sub-array has one
 * element (trivially sorted), then merges adjacent sorted pairs.
 * Each merge pass is O(n), and there are O(log n) levels → O(n log n) total.
 * It is comparison-based, so it works for any data regardless of key range.
 *
 * merge_step() handles the merge of two already-sorted halves [l..mid]
 * and [mid+1..r] into the global temp_buffer, then copies back.
 */
void merge_step(Record *arr, int l, int mid, int r)
{
    int i = l;
    int j = mid + 1;
    int k = l;

    while (i <= mid && j <= r) {
        if (arr[i].cumulative <= arr[j].cumulative)
            temp_buffer[k++] = arr[i++];
        else
            temp_buffer[k++] = arr[j++];
    }

    while (i <= mid)
        temp_buffer[k++] = arr[i++];

    while (j <= r)
        temp_buffer[k++] = arr[j++];

    for (int x = l; x <= r; x++)
        arr[x] = temp_buffer[x];
}

void merge_sort(Record *arr, int left, int right)
{
    if (left >= right) return;

    int mid = left + (right - left) / 2;

    merge_sort(arr, left, mid);
    merge_sort(arr, mid + 1, right);
    merge_step(arr, left, mid, right);
}

void run_exercise1(Record *original, int n) {

    printf("\n========================================\n");
    printf("  EXERCISE 1: Counting Sort vs Merge Sort\n");
    printf("========================================\n");

    bool count_works = true;
    struct timespec start, end;

    int sample_n = (n > 2000) ? 2000 : n;

    Record *sample = malloc(sample_n * sizeof(Record));
    Record *count_in = malloc(sample_n * sizeof(Record));
    Record *count_out = malloc(sample_n * sizeof(Record));
    Record *merge_data = malloc(sample_n * sizeof(Record));

    if (!sample || !count_in || !count_out || !merge_data) {
        printf("Memory error\n");
        return;
    }

    srand((unsigned)time(NULL));

    for (int i = 0; i < sample_n; i++)
        sample[i] = original[rand() % n];

    memcpy(count_in, sample, sample_n * sizeof(Record));
    memcpy(merge_data, sample, sample_n * sizeof(Record));

    /* ================= COUNTING SORT ================= */
    clock_gettime(CLOCK_MONOTONIC, &start);

    counting_sort(count_in, sample_n, count_out, &count_works);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double count_time = time_diff(start, end);

    if (count_works) {
        printf("\nCounting Sort (2000 sample) - first 10:\n");
        print_records(count_out, sample_n, 10);
    } else {
        printf("\nCounting Sort failed (range too large)\n");
    }

    printf("Counting Sort time : %.9f sec\n\n", count_time);

    /* ================= MERGE SORT ================= */
    clock_gettime(CLOCK_MONOTONIC, &start);

    merge_sort(merge_data, 0, sample_n - 1);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double merge_time = time_diff(start, end);

    printf("Merge Sort (2000 sample) - first 10:\n");
    print_records(merge_data, sample_n, 10);

    printf("Merge Sort time    : %.9f sec\n\n", merge_time);

    free(sample);
    free(count_in);
    free(count_out);
    free(merge_data);

    printf("==== SUMMARY ====\n");
    printf("Counting Sort: %.9f sec\n", count_time);
    printf("Merge Sort   : %.9f sec\n", merge_time);
}

/* ============================================================
 *  EXERCISE 2 – Heap Sort & Quick Sort  (by Value)
 * ============================================================ */

/*
 * HEAP SORT LOGIC:
 * Heap sort works in two phases:
 *   Phase 1 (build max-heap): rearrange the array into a max-heap so
 *     that the largest element sits at index 0.  This takes O(n).
 *   Phase 2 (extract): repeatedly swap the root (max) with the last
 *     element of the unsorted portion, shrink the heap by one, and
 *     restore the heap property (heapify). Each extraction is O(log n),
 *     giving O(n log n) total.
 * Result: sorted ascending (smallest first).
 *
 * heapify_value() enforces the heap property at node i:
 * if either child is larger than the parent, swap and continue downward.
 */
void heapify_value(Record *data, int size, int i) {
    while (1) {
        int largest = i;
        int l = 2*i+1;   /* left child index  */
        int r = 2*i+2;   /* right child index */

        /* Find which of {parent, left child, right child} holds the largest value */
        if (l < size && data[l].value > data[largest].value) largest = l;
        if (r < size && data[r].value > data[largest].value) largest = r;

        /* If the parent is already the largest, the subtree satisfies the heap property */
        if (largest == i) break;

        swap_Records(data, i, largest);
        i = largest;    /* Continue heapifying downward from the swapped position */
    }
}

void heap_sort(Record *data, int n) {
    if (n <= 0) return;

    /* Build max-heap bottom-up (start from last non-leaf node) */
    for (int i = n/2-1; i >= 0; i--)
        heapify_value(data, n, i);

    /* Repeatedly move the current max (root) to its final sorted position */
    for (int i = n-1; i > 0; i--) {
        swap_Records(data, 0, i);   /* root (max) goes to end of unsorted region */
        heapify_value(data, i, 0);  /* restore heap for the reduced array */
    }
}

/*
 * QUICK SORT LOGIC:
 * Quick sort picks a pivot element, then partitions the array so that
 * everything ≤ pivot ends up left of it and everything > pivot ends up
 * right of it. It then recurses on both sub-arrays.
 *
 * Average case is O(n log n); worst case (already-sorted input with
 * a bad pivot choice) is O(n^2). In practice it outperforms Heap Sort
 * because it has better cache locality — most comparisons touch nearby
 * memory addresses.
 *
 * partition_value() uses the first element as the pivot and the
 * two-pointer (Lomuto-style) approach to rearrange the partition.
 */
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

void quick_sort(Record *data, int lo, int hi)
{
    if (lo >= hi) return;

    int lt, gt;
    partition_value(data, lo, hi, &lt, &gt);

    quick_sort(data, lo, lt - 1);
    quick_sort(data, gt + 1, hi);
}

void run_exercise2(Record *original, int n) {
    printf("\n========================================\n");
    printf("  EXERCISE 2: Heap Sort vs Quick Sort\n");
    printf("  (sorting by Value field, ascending)\n");
    printf("========================================\n");

    struct timespec start, end;

    Record *heap_data  = malloc(n * sizeof(Record));
    Record *quick_data = malloc(n * sizeof(Record));
    if (!heap_data || !quick_data) { printf("Memory error\n"); return; }

    memcpy(heap_data,  original, n * sizeof(Record));
    memcpy(quick_data, original, n * sizeof(Record));

    /* ================= HEAP SORT ================= */
    clock_gettime(CLOCK_MONOTONIC, &start);

    heap_sort(heap_data, n);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double heap_time = time_diff(start, end);

    printf("\nHeap Sort - first 10:\n");
    print_records(heap_data, n, 10);

    /* ================= QUICK SORT ================= */
    clock_gettime(CLOCK_MONOTONIC, &start);

    quick_sort(quick_data, 0, n - 1);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double quick_time = time_diff(start, end);

    printf("\nQuick Sort - first 10:\n");
    print_records(quick_data, n, 10);

    printf("\nHeap Sort time  : %.9f sec\n", heap_time);
    printf("Quick Sort time : %.9f sec\n", quick_time);

    printf("\nFastest: %s\n",
        (heap_time < quick_time) ? "Heap Sort" : "Quick Sort");

    free(heap_data);
    free(quick_data);
}

/* ============================================================
 *  EXERCISE 3 – Binary Search & Interpolation Search
 * ============================================================ */
void swap(Record *a, Record *b) {
    Record tmp = *a;
    *a = *b;
    *b = tmp;
}

void heapifyByDate(Record *a, int n, int i) {
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

void heapSortByDate(Record *a, int n) {
    for (int i = n/2 - 1; i >= 0; i--)
        heapifyByDate(a, n, i);

    for (int i = n - 1; i > 0; i--) {
        swap(&a[0], &a[i]);
        heapifyByDate(a, i, 0);
    }
}
/*
 * BINARY SEARCH LOGIC:
 * Classic divide-and-conquer search on a sorted array.
 * Each iteration halves the search interval by comparing the target
 * against the middle element — guaranteed O(log n) regardless of the
 * data distribution.
 */
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

/*
 * INTERPOLATION SEARCH LOGIC:
 * An improvement over binary search for uniformly distributed data.
 * Instead of always probing the midpoint, it estimates where the key
 * is likely to be using linear interpolation between the endpoints:
 *
 *   pos = lo + (key - data[lo]) / (data[hi] - data[lo]) * (hi - lo)
 *
 * On uniformly distributed data this gives O(log log n) average case.
 * On skewed / clustered data the estimate can be poor and performance
 * may fall to O(n) in the worst case.
 */
int interpolationSearch(Record *input, int low, int high, long long x) {

    while (low <= high) {

        long long left = dateToInt(input[low].date);
        long long right = dateToInt(input[high].date);

        if (x < left || x > right)
            return -1;

        if (right == left)
            return (left == x) ? low : -1;

        // safer clamp to avoid invalid indices
        int pos = low + (int)(
            ((double)(x - left) * (high - low)) /
            (right - left)
        );

        if (pos < low) pos = low;
        if (pos > high) pos = high;

        long long posKey = dateToInt(input[pos].date);

        if (posKey == x) {
            while (pos > low && dateToInt(input[pos - 1].date) == x)
                pos--;
            return pos;
        }

        if (posKey < x)
            low = pos + 1;
        else
            high = pos - 1;
    }

    return -1;
}

void run_exercise3(Record *sorted, int n, long long key) {

    printf("\n========================================\n");
    printf("  EXERCISE 3: Binary vs Interpolation Search\n");
    printf("========================================\n");

    struct timespec start, end;
    volatile int sink = 0;

    /* ================= BINARY ================= */
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < TRIALS; i++)
        sink += binarySearch(sorted, n, key);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double t1 = time_diff(start, end) / TRIALS;

    /* ================= INTERPOLATION ================= */
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < TRIALS; i++)
        sink += interpolationSearch(sorted, 0, n - 1, key);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double t2 = time_diff(start, end) / TRIALS;

    int r1 = binarySearch(sorted, n, key);
    int r2 = interpolationSearch(sorted, 0, n - 1, key);

    printf("\nBinary Search time       : %.9f sec\n", t1);
    printf("Interpolation Search time: %.9f sec\n", t2);

    printf("\nResult:\n");

    if (r1 != -1)
        printf("Binary found: %s\n", sorted[r1].date);

    if (r2 != -1)
        printf("Interpolation found: %s\n", sorted[r2].date);

    printf("\nFastest: %s\n",
        (t1 < t2) ? "Binary Search" : "Interpolation Search");

    (void)sink;
}


/* ============================================================
 *  EXERCISE 4 – BIS & BIS*
 * ============================================================ */

/*
 * BIS (Binomial Interpolation Search) LOGIC:
 * BIS combines interpolation (to make an initial position estimate) with
 * a linear probe step of size sqrt(n) to quickly bracket the target key
 * before re-interpolating. This gives O(log log n) average complexity.
 *
 * After the interpolated guess (nx), if the key lies to the right:
 *   - Jump ahead in multiples of sqrt(range): nx+1*step, nx+2*step, ...
 *   - Stop when we overshoot the key or hit the array boundary.
 *   - The new search window is [nx+(i-1)*step+1 .. nx+i*step].
 * Mirror logic applies when the key lies to the left.
 *
 * The linear increment (i++) means the probe covers steps 1,2,3,4,...
 * In the worst case this scans O(sqrt(n)) positions before bracketing.
 */
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


/*
 * BIS* (BIS improved) LOGIC:
 * BIS* is identical to BIS except for one key change: instead of
 * incrementing the probe counter linearly (i++), it doubles it (i*=2).
 * This exponential probing (1, 2, 4, 8, ...) is the same idea as
 * exponential/galloping search, and it tightens the worst-case bracket
 * from O(sqrt(n)) positions down to O(log(sqrt(n))) = O(log n / 2),
 * improving the worst-case complexity compared to standard BIS.
 *
 * The trade-off: after doubling we overshoot further, so the narrowed
 * window is [nx+(i/2)*step .. nx+i*step] — slightly wider than BIS's
 * tighter bracket, but the total number of probes is much smaller.
 */
int bis_star(Record *a, int low, int high, long long key) {
    while (low <= high) {

        long long leftKey = dateToInt(a[low].date);
        long long rightKey = dateToInt(a[high].date);

        if (key < leftKey || key > rightKey)
            return -1;

        if (low == high)
            return (leftKey == key) ? low : -1;

        int next;

        if (rightKey == leftKey) {
            next = low;
        } else {
            double ratio =
                (double)(key - leftKey) /
                (double)(rightKey - leftKey);

            next = low + (int)(ratio * (high - low));
        }

        /* clamp (VERY IMPORTANT) */
        if (next < low) next = low;
        if (next > high) next = high;

        long long nextKey = dateToInt(a[next].date);

        if (nextKey == key)
            return next;

        int step = (int)sqrt(high - low + 1);
        if (step < 1) step = 1;

        int jump = 1;

        if (key > nextKey) {

            while (next + jump * step <= high &&
                   dateToInt(a[next + jump * step].date) < key) {

                if (jump > (1 << 20)) break; // safety guard
                jump *= 2;
                   }

            int start = next + (jump / 2) * step;
            if (start > high) start = high;

            low = start + 1;
        }
        else {

            while (next - jump * step >= low &&
                   dateToInt(a[next - jump * step].date) > key) {

                if (jump > (1 << 20)) break; // safety guard
                jump *= 2;
                   }

            int end = next - (jump / 2) * step;
            if (end < low) end = low;

            high = end - 1;
        }
    }

    return -1;
}

void run_exercise4(Record *sorted, int n, long long key)
{
    printf("\n========================================\n");
    printf("  EXERCISE 4: BIS vs BIS*\n");
    printf("========================================\n");

    struct timespec start, end;
    volatile int sink = 0;

    int r1 = bis(sorted, 0, n - 1, key);
    int r2 = bis_star(sorted, 0, n - 1, key);

    printf("\nBIS result  : %s\n",
        (r1 != -1) ? sorted[r1].date : "NOT FOUND");

    printf("BIS* result : %s\n",
        (r2 != -1) ? sorted[r2].date : "NOT FOUND");

    /* ================= AVERAGE CASE ================= */
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < 2000; i++) {
        int pos = rand() % n;
        long long k = dateToInt(sorted[pos].date);
        sink += bis(sorted, 0, n - 1, k);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double bis_avg = time_diff(start, end);

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < 2000; i++) {
        int pos = rand() % n;
        long long k = dateToInt(sorted[pos].date);
        sink += bis_star(sorted, 0, n - 1, k);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double bisstar_avg = time_diff(start, end);

    /* ================= WORST CASE ================= */
    long long left = dateToInt(sorted[n/3].date);
    long long right = dateToInt(sorted[n/3 + 1].date);
    long long missing_key = (left + right) / 2;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < 2000; i++)
        sink += bis(sorted, 0, n - 1, missing_key);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double bis_worst = time_diff(start, end);

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < 2000; i++) {
        int pos = (i * i + 31 * i) % n;
        long long k = dateToInt(sorted[pos].date);
        sink += bis_star(sorted, 0, n - 1, k);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double bisstar_worst = time_diff(start, end);

    printf("\nAVERAGE CASE:\n");
    printf("BIS   : %.9f sec\n", bis_avg);
    printf("BIS*  : %.9f sec\n", bisstar_avg);

    printf("\nWORST CASE:\n");
    printf("BIS   : %.9f sec\n", bis_worst);
    printf("BIS*  : %.9f sec\n", bisstar_worst);

    printf("\nRESULT:\n");
    printf("Avg Faster  : %s\n",
        (bis_avg < bisstar_avg) ? "BIS" : "BIS*");

    printf("Worst Faster : %s\n",
        (bis_worst < bisstar_worst) ? "BIS" : "BIS*");

    (void)sink;
}

/* ============================================================
 *  MAIN
 * ============================================================ */

int main(void) {
    const char *filename =
        "effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv";

    /* Allocate all large buffers upfront.
       111440 records × ~320 bytes per Record ≈ 35 MB total across three arrays. */
    Record *original   = malloc(MAX_ROWS * sizeof(Record));
    Record *sorted_val = malloc(MAX_ROWS * sizeof(Record)); /* pre-sorted by Value for Ex.3&4 */
    temp_buffer        = malloc(MAX_ROWS * sizeof(Record)); /* scratch space for Merge Sort    */

    if (!original || !sorted_val || !temp_buffer) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    /* Load the CSV once — all exercises share the same original data */
    int n = load_csv(filename, original);
    if (n <= 0) {
        free(original); free(sorted_val); free(temp_buffer);
        return 1;
    }
    printf("Loaded %d records from CSV.\n", n);

    memcpy(sorted_val, original, n * sizeof(Record));
    heap_sort(sorted_val, n);

    run_exercise1(original, n);

    run_exercise2(original, n);


    char input[16];
    printf("\nEnter Date (Exercises 3 & 4) (dd/mm/yyyy): ");
    scanf("%s", input);

    long long search_val = dateToInt(input);

    run_exercise3(sorted_val, n, search_val);

    run_exercise4(sorted_val, n, search_val);

    printf("\n========================================\n");
    printf("  All exercises complete.\n");
    printf("========================================\n");
    printf("\nPress Enter to exit...\n");
    getchar(); getchar();

    free(original);
    free(sorted_val);
    free(temp_buffer);
    return 0;
}
