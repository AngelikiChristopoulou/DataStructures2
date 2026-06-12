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

#define TRIALS 50000   // for benchmarking

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
void counting_sort(Record *input, int n, Record *output, bool *works) {
    if (n <= 0) return;

    /* Step 1: scan the whole array once to find the key range [mn, mx].
       Shifting all keys by -mn lets us use a zero-based count array. */
    long long mn = input[0].cumulative, mx = input[0].cumulative;
    for (int i = 1; i < n; i++) {
        if (input[i].cumulative < mn) mn = input[i].cumulative;
        if (input[i].cumulative > mx) mx = input[i].cumulative;
    }
    printf("  Cumulative min=%lld  max=%lld\n", mn, mx);

    long long range = mx - mn + 1;

    /* Step 2: guard against an unreasonably large count array.
       10 million entries ~ 40 MB, which is already generous. */
    if (range > 10000000LL) {
        printf("  Value range too large for Counting Sort.\n");
        printf("  Counting Sort cannot run on this dataset.\n");
        memcpy(output, input, n * sizeof(Record));
        *works = false;
        return;
    }

    /* Step 3: count occurrences of each shifted key */
    int *cnt = calloc((size_t)range, sizeof(int));
    if (!cnt) { printf("  Memory allocation failed\n"); return; }

    for (int i = 0; i < n; i++)
        cnt[input[i].cumulative - mn]++;

    /* Step 4: convert counts to prefix sums.
       cnt[k] now holds the index just past the last output slot for key k. */
    for (long long i = 1; i < range; i++)
        cnt[i] += cnt[i-1];

    /* Step 5: fill output right-to-left to preserve stability —
       equal-key records keep their original relative order. */
    for (int i = n-1; i >= 0; i--) {
        long long idx = input[i].cumulative - mn;
        cnt[idx]--;
        output[cnt[idx]] = input[i];
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
void merge_step(Record *arr, int l, int mid, int r) {
    int a = l, b = mid+1, out = l;

    /* Compare the front elements of both halves and take the smaller one */
    while (a <= mid && b <= r) {
        if (arr[a].cumulative <= arr[b].cumulative)
            temp_buffer[out++] = arr[a++];
        else
            temp_buffer[out++] = arr[b++];
    }

    /* Drain whichever half still has elements remaining */
    while (a <= mid) temp_buffer[out++] = arr[a++];
    while (b <= r)   temp_buffer[out++] = arr[b++];

    /* Write the merged result back over the original segment */
    for (int i = l; i <= r; i++) arr[i] = temp_buffer[i];
}

void merge_sort(Record *arr, int left, int right) {
    if (left >= right) return;          /* Base case: single element is already sorted */
    int mid = (left + right) / 2;
    merge_sort(arr, left, mid);     /* Sort left half */
    merge_sort(arr, mid+1, right);   /* Sort right half */
    merge_step(arr, left, mid, right);  /* Merge the two sorted halves */
}

void run_exercise1(Record *original, int n) {
    printf("\n========================================\n");
    printf("  EXERCISE 1: Counting Sort vs Merge Sort\n");
    printf("  (sorting by Cumulative field, ascending)\n");
    printf("========================================\n");

    bool count_works = true;

    /* Work on separate copies so both algorithms start from the same input */
    Record *count_in   = malloc(n * sizeof(Record));
    Record *count_out  = malloc(n * sizeof(Record));
    Record *merge_data = malloc(n * sizeof(Record));
    if (!count_in || !count_out || !merge_data) { printf("Memory error\n"); count_works = false; return; }

    memcpy(count_in,   original, n * sizeof(Record));
    memcpy(merge_data, original, n * sizeof(Record));

    /* --- Counting Sort --- */
    clock_t t0 = clock();
    counting_sort(count_in, n, count_out, &count_works);
    double count_time = (double)(clock() - t0) / CLOCKS_PER_SEC;

    if (count_works) {
        printf("\nCounting Sort - first 10 records:\n");
        print_records(count_out, n, 10);
        printf("Counting Sort time : %.6f sec  |  O(n+k)\n\n", count_time);
    } else {
        printf("Counting Sort: Did not Sort the Data.\n\n");
    }

    /* --- Merge Sort --- */
    t0 = clock();
    merge_sort(merge_data, 0, n-1);
    double merge_time = (double)(clock() - t0) / CLOCKS_PER_SEC;

    printf("Merge Sort - first 10 records:\n");
    print_records(merge_data, n, 10);
    printf("Merge Sort time    : %.6f sec  |  O(n*logn)\n\n", merge_time);
    free(count_in); free(count_out); free(merge_data);

    if (count_works) {
        printf("Counting Sort: %.6f sec | O(n+k)\n", count_time);
    } else {
        printf("Counting Sort did not Sort the Data.\n\n");
    }
    printf("Merging Sort: %.6f sec | O(n*logn)\n", merge_time);
    if (!count_works) {
        printf("Counting did not work.\n");
    }
    if (count_time < merge_time && count_works) {
        printf("Fastest:  Counting Sort\n");
    } else {
        printf("Fastest:  Merging Sort\n");
    }
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

    Record *heap_data  = malloc(n * sizeof(Record));
    Record *quick_data = malloc(n * sizeof(Record));
    if (!heap_data || !quick_data) { printf("Memory error\n"); return; }

    memcpy(heap_data,  original, n * sizeof(Record));
    memcpy(quick_data, original, n * sizeof(Record));

    /* --- Heap Sort --- */
    clock_t t0 = clock();
    heap_sort(heap_data, n);
    double heap_time = (double)(clock() - t0) / CLOCKS_PER_SEC;
    printf("\nHeap Sort - first 10 records:\n");
    print_records(heap_data, n, 10);
    printf("Heap Sort time  : %.6f sec  |  O(n*logn)\n\n", heap_time);

    /* --- Quick Sort --- */
    t0 = clock();
    quick_sort(quick_data, 0, n-1);
    double quick_time = (double)(clock() - t0) / CLOCKS_PER_SEC;
    printf("Quick Sort - first 10 records:\n");
    print_records(quick_data, n, 10);
    printf("Quick Sort time : %.6f sec  |  O(n*logn)\n\n", quick_time);
    free(heap_data); free(quick_data);
    printf("Heap Sort: %.6f sec | O(n*logn)\n", heap_time);
    printf("Quick Sort: %.6f sec | O(n*logn)\n", quick_time);
    printf("Fastest: %s\n", (heap_time < quick_time) ? "Heap Sort" : "Quick Sort");
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

    while (low <= high &&
           x >= dateToInt(input[low].date) &&
           x <= dateToInt(input[high].date)) {

        long long left = dateToInt(input[low].date);
        long long right = dateToInt(input[high].date);

        if (right == left) {
            return (left == x) ? low : -1;
        }

        int pos = low +
            (double)(high - low) *
            (x - left) /
            (right - left);

        long long posKey = dateToInt(input[pos].date);

        if (posKey == x)
            return pos;

        if (posKey < x)
            low = pos + 1;
        else
            high = pos - 1;
           }

    return -1;
}

void run_exercise3(Record *sorted, int n, long long key) {
    printf("\n========================================\n");
    printf("  EXERCISE 3: Binary Search vs Interpolation Search\n");
    printf("  Searching for date\n");
    printf("========================================\n");

    heapSortByDate(sorted, n);

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

void run_exercise4(Record *sorted, int n, long long key) {
    printf("\n========================================\n");
    printf("  EXERCISE 4: BIS vs BIS*\n");
    printf("  Searching for date = %lld\n", key);
    printf("========================================\n");

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

    printf("\nFastest: %s\n",
           (t1 < t2) ? "BIS" : "BIS*");
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
