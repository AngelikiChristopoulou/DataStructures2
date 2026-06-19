/*
 * ============================================================
 *  All Exercises Combined (A-C)
 *  Dataset: effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv
 * ============================================================
 *
 *  Exercise A: AVL BST sorted by Date
 *             Supports in-order display, search, modify, and delete by Date.
 *  Exercise B: AVL BST sorted by Cumulative
 *             Supports finding days with the minimum/maximum Cumulative value.
 *             Ties are stored in a singly-linked EqNode list attached to each BST node.
 *  Exercise C: Hash table with chaining, keyed on Date string
 *             Supports search, modify, and delete by Date.
 *             Hash function = sum of ASCII codes of the date string modulo m (odd).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

/* Buffer size constants sized to fit the fixed-width CSV fields */
#define DIRECTION 8
#define YEAR 5
#define DATE 3        /* date is stored as int[3] = {day, month, year} for A and B */
#define DATEC 11      /* date is stored as a "DD/MM/YYYY" string (10 chars + '\0') for C */
#define WEEKDAY 10
#define MAX_STR 64
#define MAX_ROWS 111440


/* One row from the CSV becomes one Record.
   Value and Cumulative are kept as 64-bit integers because the
   trade figures exceed the range of a 32-bit int.
   Used by exercises A and B; date is split into int[3] so that
   compareDates() can compare year/month/day individually. */
typedef struct {
    char direction[DIRECTION];
    char year[YEAR];
    int date[DATE];       /* [0]=day  [1]=month  [2]=year */
    char weekday[WEEKDAY];
    char country[MAX_STR];
    char commodity[MAX_STR];
    char transport_mode[MAX_STR];
    char measure[MAX_STR];
    long long value;
    long long cumulative;
} Record;

/*
 * Exercise A uses a plain AVL node: each node holds exactly one Record
 * and the standard left/right/height AVL fields.
 * Exercise B needs an extra 'equal' linked list to hold all Records that
 * share the same Cumulative key, so we define a separate NodeB type.
 */
typedef struct NodeA {
    Record* record;
    struct NodeA* left;
    struct NodeA* right;
    int height;
} NodeA;

/* EqNode — a singly-linked list node used by NodeB to chain records
   that have the same Cumulative value (ties in the B-tree). */
typedef struct EqNode {
    Record* value;
    struct EqNode* next;
} EqNode;

/* NodeB — AVL node for Exercise B.
   Each BST key is a unique Cumulative value; if multiple dates share
   that value they are appended to the 'equal' linked list. */
typedef struct NodeB {
    Record* record;
    struct NodeB* left;
    struct NodeB* right;
    int height;
    EqNode* equal;   /* linked list of Records with the same Cumulative value */
} NodeB;

/* RecordC — identical to Record except the date is kept as a raw
   "DD/MM/YYYY" string so it can be passed directly to hash_date().
   Used exclusively by Exercise C. */
typedef struct {
    char direction[DIRECTION];
    char year[YEAR];
    char date[DATEC];   /* stored as "DD/MM/YYYY\0" — needed for hashing */
    char weekday[WEEKDAY];
    char country[MAX_STR];
    char commodity[MAX_STR];
    char transport_mode[MAX_STR];
    char measure[MAX_STR];
    long long value;
    long long cumulative;
} RecordC;

/* NodeC — a singly-linked list node for the hash-table chains (Exercise C).
   Each bucket in the HashMap is the head of a NodeC chain. */
typedef struct NodeC {
    RecordC* record;
    struct NodeC* next;
} NodeC;

/* HashMap — the top-level hash table structure for Exercise C.
   'buckets' is an array of 'm' chain heads (NodeC*).
   'm' should be an odd number so the modulo hash distributes evenly.
   'totalRecords' tracks how many entries are currently stored. */
typedef struct {
    NodeC **buckets;
    int m;
    int totalRecords;
} HashMap;


/* ============================================================
 *  SHARED UTILITY FUNCTIONS
 * ============================================================ */

/* Reads every data row from the CSV into the pre-allocated 'data' array.
   The header line is consumed and discarded.
   The Date column is tokenised on '/' to extract day/month/year into int[3].
   Returns the number of rows successfully parsed, or -1 on file error. */
int load_csv(const char *filename, Record *data) {
    FILE *fptr = fopen(filename, "r");
    if(!fptr) {
        printf("[ERROR] No file %s found.\n", filename);
        return -1;
    }

    char line[512]; /* temporary line buffer */
    int count = 0;

    fgets(line, sizeof(line), fptr); /* skip header row */

    while (fgets(line, sizeof(line), fptr) && count < MAX_ROWS) {
        line[strcspn(line, "\r\n")] = 0; /* strip trailing newline */

        char *temp;
        Record r;
        memset(&r, 0, sizeof(r));

        temp = strtok(line, ",");
        if(!temp) continue;
        strncpy(r.direction, temp, DIRECTION-1);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.year, temp, YEAR-1);

        /* The Date field is "DD/MM/YYYY" — split on '/' to get three integers.
           The last token switches delimiter to ',' to also consume the comma
           that follows the year in the CSV. */
        temp = strtok(NULL, "/");
        if(!temp) continue;
        r.date[0] = strtol(temp, NULL, 10);  /* day   */

        temp = strtok(NULL, "/");
        if(!temp) continue;
        r.date[1] = strtol(temp, NULL, 10);  /* month */

        temp = strtok(NULL, ",");
        if(!temp) continue;
        r.date[2] = strtol(temp, NULL, 10);  /* year  */

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

/* Three-level date comparison used to order BST nodes.
   Compares year first, then month, then day.
   Returns negative / zero / positive just like strcmp(). */
int compareDates(int* x, int* y) {
    if(x[2] != y[2]) return x[2] - y[2];  /* compare year  */
    if(x[1] != y[1]) return x[1] - y[1];  /* compare month */
    return x[0] - y[0];                    /* compare day   */
}


/* ============================================================
 *  MENU FUNCTIONS
 * ============================================================ */

/* Top-level menu: asks whether to load the file into a BST or a Hash table. */
int printMenuBSTorHashing() {
    int choice;

    printf("\n=======MENU=======\n");
    printf("1 => BST\n");
    printf("2 => Hashing\n");
    printf("3 => Exit\n");
    printf("Choice: ");
    scanf("%d", &choice);

    return choice;
}

/* Second-level BST menu: asks whether to key the BST on Date (Exercise A)
   or on Cumulative (Exercise B). */
int printMenuBST_datecumulative() {
    int choice;

    printf("\n=======BST MENU=======\n");
    printf("1 => BST by Date\n");
    printf("2 => BST by Cumulative\n");
    printf("3 => Exit\n");
    printf("Choice: ");
    scanf("%d", &choice);

    return choice;
}

/* Exercise A operations menu (BST keyed on Date). */
int printMenuA() {
    int choice;

    printf("\n=======MENU A=======\n");
    printf("1 => In-order Display\n");
    printf("2 => Search by date\n");
    printf("3 => Modify cumulative\n");
    printf("4 => Delete by date\n");
    printf("5 => Exit\n");
    printf("Choice: ");
    scanf("%d", &choice);

    return choice;
}

/* Exercise B operations menu (BST keyed on Cumulative). */
int printMenuB() {
    int choice;

    printf("\n=======MENU B=======\n");
    printf("1 => Display Minimum Cumulative Dates\n");
    printf("2 => Display Maximum Cumulative Dates\n");
    printf("3 => Exit\n");
    printf("Choice: ");
    scanf("%d", &choice);

    return choice;
}

/* Exercise C operations menu (Hash table keyed on Date string). */
int printMenuC() {
    int choice;

    printf("\n=======MENU C=======\n");
    printf("1 => Search Cumulative by Date\n");
    printf("2 => Modify Cumulative by Date\n");
    printf("3 => Delete Record by Date\n");
    printf("4 => Exit\n");
    printf("Choice: ");
    scanf("%d", &choice);

    return choice;
}


/* ============================================================
 *  EXERCISE A – AVL BST keyed on Date
 * ============================================================
 *
 * AVL TREE LOGIC:
 * An AVL tree is a self-balancing Binary Search Tree (BST).
 * After every insert or delete the tree checks the balance factor
 * (height(left) - height(right)) at each ancestor.  If that factor
 * reaches +2 or -2 the tree performs one or two rotations to restore
 * the invariant |bf| <= 1, keeping all operations at O(log n).
 *
 * The key here is the calendar date (year > month > day).
 * Each date appears at most once in the BST because every CSV row
 * with the same date accumulates its Cumulative value in a single node
 * — duplicate keys are silently dropped by insertA().
 */

/* Forward declarations for Exercise A */
NodeA* createNodeA(Record* value);
NodeA* insertA(NodeA* node, Record* value);
int heightA(NodeA* node);
int balanceFactorA(NodeA* node);
void updateHeightA(NodeA* node);
NodeA* rotateRightA(NodeA* node);
NodeA* rotateLeftA(NodeA* node);
NodeA* balanceA(NodeA* node);
NodeA* buildTreeA(Record* data, int n);
void swapNodesA(NodeA* to, NodeA* from);
void inorderA(NodeA* root);
NodeA* searchA(NodeA* root, int date[3]);
NodeA* modifyA(NodeA* root, int date[3], long long value);
NodeA* minNodeA(NodeA* node);
NodeA* deleteNodeA(NodeA* root, int date[3]);


/* Allocates and initialises a new AVL leaf node for Exercise A.
   The new node stores a pointer to the existing Record (no copy). */
NodeA* createNodeA(Record* value) {
    NodeA* newNode = malloc(sizeof(NodeA));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->record = value;
    newNode->left   = NULL;
    newNode->right  = NULL;
    newNode->height = 1;    /* a fresh leaf has height 1 */
    return newNode;
}

/* Recursively inserts 'value' into the AVL subtree rooted at 'node'.
   Duplicate dates are ignored (the existing node is returned unchanged).
   After insertion the subtree is rebalanced before returning. */
NodeA* insertA(NodeA* node, Record* value) {
    /* Found the correct empty slot — create and place a new leaf */
    if (node == NULL) {
        return createNodeA(value);
    }

    int cmp = compareDates(value->date, node->record->date);
    if (cmp < 0) {
        node->left  = insertA(node->left,  value); /* go left  (earlier date) */
    } else if (cmp > 0) {
        node->right = insertA(node->right, value); /* go right (later date)   */
    } else {
        return node; /* duplicate date — no insert */
    }

    /* Rebalance this node on the way back up the recursion */
    return balanceA(node);
}

/* Returns the stored height of 'node', or 0 for a NULL pointer. */
int heightA(NodeA* node) {
    return node ? node->height : 0;
}

/* Balance factor = height(left) - height(right).
   A value outside [-1, 1] means the node needs rotation. */
int balanceFactorA(NodeA* node) {
    return node ? heightA(node->left) - heightA(node->right) : 0;
}

/* Recomputes the cached height of 'node' from its children.
   Must be called after any structural change to a subtree. */
void updateHeightA(NodeA* node) {
    if(node) {
        int lh = heightA(node->left);
        int rh = heightA(node->right);
        node->height = 1 + (lh > rh ? lh : rh);
    }
}

/* Right rotation: lifts node->left (x) to become the new subtree root.
   Used when the tree is left-heavy (balance factor > 1). */
NodeA* rotateRightA(NodeA* node) {
    NodeA* x = node->left;   /* x becomes the new root */
    NodeA* y = x->right;     /* y is x's right child — must be rehung */
    x->right  = node;        /* old root drops to become x's right child */
    node->left = y;          /* rehang y */
    updateHeightA(node);     /* update bottom-up: node first, then x */
    updateHeightA(x);
    return x;
}

/* Left rotation: lifts node->right (x) to become the new subtree root.
   Used when the tree is right-heavy (balance factor < -1). */
NodeA* rotateLeftA(NodeA* node) {
    NodeA* x = node->right;  /* x becomes the new root */
    NodeA* y = x->left;      /* y is x's left child — must be rehung */
    x->left   = node;        /* old root drops to become x's left child */
    node->right = y;         /* rehang y */
    updateHeightA(node);     /* update bottom-up: node first, then x */
    updateHeightA(x);
    return x;
}

/* Checks the balance factor of 'node' and applies the appropriate
   single or double rotation if needed, then returns the new subtree root.
   Four cases:
     bf > 1  and left child bf >= 0  → single right rotation (LL case)
     bf > 1  and left child bf <  0  → left-right double rotation (LR case)
     bf < -1 and right child bf <= 0 → single left rotation (RR case)
     bf < -1 and right child bf >  0 → right-left double rotation (RL case) */
NodeA* balanceA(NodeA* node) {
    updateHeightA(node);
    int bf = balanceFactorA(node);

    /* Left-heavy: LL or LR case */
    if (bf > 1) {
        if (balanceFactorA(node->left) < 0) {
            node->left = rotateLeftA(node->left);  /* LR: first rotate left child left */
        }
        return rotateRightA(node);                 /* then rotate the node right */
    }

    /* Right-heavy: RR or RL case */
    if (bf < -1) {
        if (balanceFactorA(node->right) > 0) {
            node->right = rotateRightA(node->right); /* RL: first rotate right child right */
        }
        return rotateLeftA(node);                    /* then rotate the node left */
    }

    return node; /* already balanced */
}

/* Builds the entire Exercise-A AVL tree by inserting every record
   from 'data[0..n-1]' one by one and returning the final root. */
NodeA* buildTreeA(Record* data, int n) {
    NodeA* root = NULL;

    for(int i = 0; i < n; i++) {
        root = insertA(root, &data[i]);
    }

    return root;
}

/* Swaps the Record pointers between two AVL nodes.
   Used by deleteNodeA() to copy the in-order successor's data
   into the node being deleted before removing the successor leaf. */
void swapNodesA(NodeA* to, NodeA* from) {
    Record* temp = to->record;
    to->record   = from->record;
    from->record = temp;
}

/* In-order (left → root → right) traversal of the Exercise-A tree.
   Prints each node's date and Cumulative value in ascending date order. */
void inorderA(NodeA* root) {
    if(!root) return;

    inorderA(root->left);

    printf("Date: %-d/%d/%d \t Cumulative: %lld\n",
        root->record->date[0], root->record->date[1], root->record->date[2],
        root->record->cumulative);

    inorderA(root->right);
}

/* Searches the AVL tree for the node whose date matches 'date[3]'.
   Returns the matching NodeA* or NULL if not found.
   Time complexity: O(log n) because the tree is balanced. */
NodeA* searchA(NodeA* root, int date[3]) {
    if(!root) return NULL;

    int cmp = compareDates(date, root->record->date);

    if(cmp == 0) return root;             /* found */
    if(cmp <  0) return searchA(root->left,  date); /* target is earlier */
    return           searchA(root->right, date);     /* target is later   */
}

/* Locates the node for 'date' and updates its Cumulative field to 'value'.
   Prints an error if the date is not in the tree.
   The tree structure is unchanged, so the root is returned as-is. */
NodeA* modifyA(NodeA* root, int date[3], long long value) {
    NodeA* node = searchA(root, date);

    if (node) {
        node->record->cumulative = value;
    } else {
        printf("Date not found.\n");
    }

    return root;
}

/* Returns the node with the smallest key in the subtree rooted at 'node'.
   In a BST the minimum is always the leftmost node.
   Used by deleteNodeA() to find the in-order successor. */
NodeA* minNodeA(NodeA* node) {
    while(node->left) {
        node = node->left;
    }
    return node;
}

/* Removes the node matching 'date' from the AVL subtree rooted at 'root'.
   Three cases:
     1. Node not found (root is NULL) — print a message and return NULL.
     2. Node has at most one child — splice it out directly.
     3. Node has two children — replace its data with the in-order successor
        (leftmost node of the right subtree), then delete the successor leaf.
   The subtree is rebalanced on the way back up. */
NodeA* deleteNodeA(NodeA* root, int date[3]) {
    if(!root) {
        printf("No nodes present.\n");
        return NULL;
    }

    int cmp = compareDates(date, root->record->date);

    if (cmp < 0) {
        root->left  = deleteNodeA(root->left,  date); /* target is in left subtree  */
    } else if (cmp > 0) {
        root->right = deleteNodeA(root->right, date); /* target is in right subtree */
    } else {
        /* This is the node to delete */
        if(!root->left || !root->right) {
            /* Case 2: zero or one child — replace node with the surviving child */
            NodeA* temp = root->left ? root->left : root->right;
            free(root);
            return temp; /* may be NULL if it was a leaf */
        }

        /* Case 3: two children
           Find in-order successor (smallest in right subtree), copy its
           record into this node, then delete the successor from the right subtree. */
        NodeA* junior = minNodeA(root->right);
        swapNodesA(root, junior);
        root->right = deleteNodeA(root->right, junior->record->date);
    }

    return balanceA(root); /* restore AVL invariant on the way back up */
}


/* ============================================================
 *  EXERCISE B – AVL BST keyed on Cumulative
 * ============================================================
 *
 * Same AVL mechanics as Exercise A, but the BST ordering key is
 * the Cumulative value instead of the Date.
 *
 * Because multiple dates can share the same Cumulative value (ties),
 * each NodeB carries an 'equal' singly-linked list (EqNode) that holds
 * all Records with that same key.  The BST itself therefore has one
 * node per distinct Cumulative value, and ties are chained off it.
 *
 * The only menu operations needed are finding the global minimum and
 * maximum Cumulative, which correspond to the leftmost and rightmost
 * BST nodes respectively — O(log n) each.
 */

/* Forward declarations for Exercise B */
EqNode* createEqNode(Record* record);
void    insertEqNode_End(NodeB* node, Record* value);
NodeB*  createNodeB(Record* value);
NodeB*  insertB(NodeB* node, Record* value);
int     heightB(NodeB* node);
int     balanceFactorB(NodeB* node);
void    updateHeightB(NodeB* node);
NodeB*  rotateRightB(NodeB* node);
NodeB*  rotateLeftB(NodeB* node);
NodeB*  balanceB(NodeB* node);
NodeB*  buildTreeB(Record* data, int n);
void    swapNodesB(NodeB* to, NodeB* from);
NodeB*  findMinB(NodeB* root);
NodeB*  findMaxB(NodeB* root);
void    printDates_MinB(NodeB* root);
void    printDates_MaxB(NodeB* root);


/* Allocates a new EqNode that wraps the given Record pointer.
   EqNode is used to chain multiple Records with the same Cumulative value
   off a single NodeB. */
EqNode* createEqNode(Record* record) {
    EqNode* newEq = malloc(sizeof(EqNode));
    if(newEq == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newEq->value = record;
    newEq->next  = NULL;
    return newEq;
}

/* Appends a new EqNode for 'value' to the end of the equal-list of 'node'.
   Walking to the tail keeps the list in insertion order. */
void insertEqNode_End(NodeB* node, Record* value) {
    EqNode* newEq = createEqNode(value);

    if(node->equal == NULL) {
        node->equal = newEq; /* first duplicate — start the list */
        return;
    }

    /* Walk to the tail of the existing list */
    EqNode* temp = node->equal;
    while(temp->next != NULL){
        temp = temp->next;
    }
    temp->next = newEq;
}

/* Allocates and initialises a new AVL leaf node for Exercise B.
   'equal' is set to NULL; duplicates are added later via insertEqNode_End(). */
NodeB* createNodeB(Record* value) {
    NodeB* newNode = malloc(sizeof(NodeB));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->record = value;
    newNode->left   = NULL;
    newNode->right  = NULL;
    newNode->height = 1;
    newNode->equal  = NULL;
    return newNode;
}

/* Recursively inserts 'value' into the AVL subtree rooted at 'node',
   ordered by Cumulative.
   If a node with the same Cumulative already exists the new Record is
   appended to that node's EqNode list instead of creating a second BST node.
   The subtree is rebalanced before returning. */
NodeB* insertB(NodeB* node, Record* value) {
    /* Found the correct empty slot — create and place a new leaf */
    if (node == NULL) {
        return createNodeB(value);
    }

    long long cmp = value->cumulative - node->record->cumulative;
    if (cmp < 0) {
        node->left  = insertB(node->left,  value); /* smaller Cumulative → left */
    } else if (cmp > 0) {
        node->right = insertB(node->right, value); /* larger  Cumulative → right */
    } else {
        /* Same Cumulative value — attach to the equal-list of this node */
        insertEqNode_End(node, value);
        return node; /* structure unchanged, no rebalance needed */
    }

    return balanceB(node);
}

/* Returns the stored height of 'node', or 0 for a NULL pointer. */
int heightB(NodeB* node) {
    return node ? node->height : 0;
}

/* Balance factor for a NodeB: height(left) - height(right). */
int balanceFactorB(NodeB* node) {
    return node ? heightB(node->left) - heightB(node->right) : 0;
}

/* Recomputes the cached height of a NodeB from its children. */
void updateHeightB(NodeB* node) {
    if(node) {
        int lh = heightB(node->left);
        int rh = heightB(node->right);
        node->height = 1 + (lh > rh ? lh : rh);
    }
}

/* Right rotation for the Exercise-B tree (same logic as rotateRightA). */
NodeB* rotateRightB(NodeB* node) {
    NodeB* x  = node->left;
    NodeB* y  = x->right;
    x->right  = node;
    node->left = y;
    updateHeightB(node);
    updateHeightB(x);
    return x;
}

/* Left rotation for the Exercise-B tree (same logic as rotateLeftA). */
NodeB* rotateLeftB(NodeB* node) {
    NodeB* x   = node->right;
    NodeB* y   = x->left;
    x->left    = node;
    node->right = y;
    updateHeightB(node);
    updateHeightB(x);
    return x;
}

/* Rebalances a NodeB using the same four-case rotation logic as balanceA(). */
NodeB* balanceB(NodeB* node) {
    updateHeightB(node);
    int bf = balanceFactorB(node);

    /* Left-heavy */
    if (bf > 1) {
        if(balanceFactorB(node->left) < 0) {
            node->left = rotateLeftB(node->left);
        }
        return rotateRightB(node);
    }

    /* Right-heavy */
    if (bf < -1) {
        if(balanceFactorB(node->right) > 0) {
            node->right = rotateRightB(node->right);
        }
        return rotateLeftB(node);
    }

    return node; /* already balanced */
}

/* Builds the Exercise-B AVL tree from 'data[0..n-1]'.
   Records with the same Cumulative are stored in the EqNode list
   of the first-inserted node for that value. */
NodeB* buildTreeB(Record* data, int n) {
    NodeB* root = NULL;

    for(int i = 0; i < n; i++) {
        root = insertB(root, &data[i]);
    }

    return root;
}

/* Swaps the Record pointers between two NodeB nodes.
   (Currently declared but not called — would be needed by a NodeB delete.) */
void swapNodesB(NodeB* to, NodeB* from) {
    Record* temp = to->record;
    to->record   = from->record;
    from->record = temp;
}

/* Prints the primary record of 'node' and all records in its equal-list.
   Used by printDates_MinB() and printDates_MaxB() to display all dates
   that share the same extreme Cumulative value. */
void printEqList(NodeB* node) {
    if(!node) return;

    /* Print the BST node's own record first */
    Record* r = node->record;
    printf("Cumulative: %lld\n \tDate: %d/%d/%d\n",
        r->cumulative, r->date[0], r->date[1], r->date[2]);

    /* Then print all tied records from the equal-list */
    EqNode* eq = node->equal;
    while(eq != NULL) {
        r = eq->value;
        printf("Cumulative: %lld\n \tDate: %d/%d/%d\n",
            r->cumulative, r->date[0], r->date[1], r->date[2]);
        eq = eq->next;
    }
}

/* Returns the node with the smallest Cumulative in the subtree
   (leftmost node), or NULL if the tree is empty. */
NodeB* findMinB(NodeB* root) {
    if(!root) return NULL;

    while(root->left) {
        root = root->left;
    }
    return root;
}

/* Returns the node with the largest Cumulative in the subtree
   (rightmost node), or NULL if the tree is empty. */
NodeB* findMaxB(NodeB* root) {
    if(!root) return NULL;

    while(root->right) {
        root = root->right;
    }
    return root;
}

/* Finds and prints all dates that have the minimum Cumulative value.
   Includes any ties stored in the leftmost node's equal-list. */
void printDates_MinB(NodeB* root) {
    NodeB* min = findMinB(root);

    if (!min) {
        printf("The tree is empty.\n");
        return;
    }

    printf("The minimum Cumulative is: %lld\n", min->record->cumulative);
    printEqList(min);
}

/* Finds and prints all dates that have the maximum Cumulative value.
   Includes any ties stored in the rightmost node's equal-list. */
void printDates_MaxB(NodeB* root) {
    NodeB* max = findMaxB(root);

    if (!max) {
        printf("The tree is empty.\n");
        return;
    }

    printf("The maximum Cumulative is: %lld\n", max->record->cumulative);
    printEqList(max);
}


/* ============================================================
 *  EXERCISE C – Hash Table with Chaining, keyed on Date string
 * ============================================================
 *
 * HASHING WITH CHAINING LOGIC:
 * A hash table maps a key to a bucket index using a hash function,
 * giving O(1) average-case lookup, insert, and delete.
 * Collisions (different dates mapping to the same bucket) are resolved
 * by chaining: each bucket is the head of a singly-linked NodeC list.
 *
 * Hash function (as specified in the assignment):
 *   h(date) = (sum of ASCII codes of every character in "DD/MM/YYYY") mod m
 * where m is an odd integer equal to the number of buckets.
 *
 * Using RecordC instead of Record keeps the date in its original string
 * form so it can be passed directly to hash_date() and compared with
 * strcmp() during chain traversal.
 */

/* Forward declarations for Exercise C */
int      hash_date(char* date, int m);
HashMap* createMap(int m);
void     destroyMap(HashMap* map);
void     insertToMap(HashMap* map, RecordC* value);
int      searchByDate(HashMap* map, char* date);
void     modifyByDate(HashMap* map, char* date);
void     deleteByDate(HashMap* map, char* date);

/* Variant of load_csv() that fills a RecordC array.
   The only difference from load_csv() is that the Date column is read
   as a raw comma-delimited token (keeping "DD/MM/YYYY" as a string)
   rather than being split on '/' into three integers. */
int load_csv_C(const char *filename, RecordC *data) {
    FILE *fptr = fopen(filename, "r");
    if(!fptr) {
        printf("[ERROR] No file %s found.\n", filename);
        return -1;
    }

    char line[512]; /* temporary line buffer */
    int count = 0;

    fgets(line, sizeof(line), fptr); /* skip header row */

    while (fgets(line, sizeof(line), fptr) && count < MAX_ROWS) {
        line[strcspn(line, "\r\n")] = 0;

        char *temp;
        RecordC r;
        memset(&r, 0, sizeof(r));

        temp = strtok(line, ",");
        if(!temp) continue;
        strncpy(r.direction, temp, DIRECTION-1);

        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.year, temp, YEAR-1);

        /* Date is kept as a string for hashing — no splitting on '/' */
        temp = strtok(NULL, ",");
        if(!temp) continue;
        strncpy(r.date, temp, DATEC-1);

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

/* Computes the bucket index for 'date' in a table with 'm' buckets.
   Sums the ASCII values of every character in the date string and
   takes the result modulo m, as specified in the assignment.
   Casting to unsigned long prevents overflow on large ASCII sums. */
int hash_date(char* date, int m) {
    unsigned long sum = 0;
    for(int i = 0; i < DATEC-1; i++) {
        sum += (unsigned char)date[i]; /* cast avoids sign-extension issues */
    }
    return (int)(sum % (unsigned long)m);
}

/* Allocates and initialises a HashMap with 'm' empty buckets.
   'm' should be an odd number to improve hash distribution.
   calloc() zeroes the bucket array so all chain heads start as NULL. */
HashMap* createMap(int m) {
    HashMap* map = malloc(sizeof(HashMap));
    if(!map) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    map->m            = m;
    map->totalRecords = 0;
    map->buckets      = calloc(m, sizeof(NodeC *)); /* all heads initialised to NULL */
    if(!map->buckets) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    return map;
}

/* Frees all NodeC chain nodes in every bucket, then frees the bucket
   array and the HashMap itself.  Note: the RecordC records they point
   to are owned by the 'original' flat array and are freed by the caller. */
void destroyMap(HashMap* map) {
    for(int i = 0; i < map->m; i++) {
        NodeC* now = map->buckets[i];
        while(now) {
            NodeC* temp = now->next;
            free(now);    /* free the chain node wrapper */
            now = temp;
        }
    }

    free(map->buckets);
    free(map);
}

/* Inserts 'value' into the hash table using prepend-to-chain (O(1)).
   The new NodeC becomes the new head of the bucket's chain.
   Prepending is faster than appending and order within a bucket
   does not matter for this application. */
void insertToMap(HashMap* map, RecordC* value) {
    int index = hash_date(value->date, map->m);

    NodeC* newNode = malloc(sizeof(NodeC));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    newNode->record      = value;
    newNode->next        = map->buckets[index]; /* prepend to existing chain */
    map->buckets[index]  = newNode;
    map->totalRecords++;
}

/* Searches the hash table for all records matching 'date' and prints
   their Cumulative values.
   Steps: hash the date → go to that bucket → walk the chain comparing dates.
   Returns 1 if at least one match was found, 0 otherwise (unused by caller). */
int searchByDate(HashMap* map, char* date) {
    int index = hash_date(date, map->m);
    NodeC *now = map->buckets[index];
    bool found = false;

    printf("\n========== Results for date: %s ==========\n", date);

    while(now) {
        if(strcmp(now->record->date, date) == 0) {
            printf("\tCumulative: %lld\n", now->record->cumulative);
            found = true;
        }
        now = now->next;
    }

    if(!found) {
        printf("[!] No Record in this date: %s\n", date);
    }

    return found ? 1 : 0;
}

/* Locates all records with 'date' in the hash table and prompts the
   user to enter a new Cumulative value for each one.
   Invalid (non-numeric) input is rejected and that record is skipped. */
void modifyByDate(HashMap* map, char* date) {
    int index = hash_date(date, map->m);
    NodeC *now = map->buckets[index];
    bool found = false;
    long long newCumulative;

    while(now) {
        if(strcmp(now->record->date, date) == 0) {
            printf("Current record : \t Cumulative: %lld\n", now->record->cumulative);

            printf("New Cumulative: ");
            if (scanf("%lld", &newCumulative) != 1) {
                printf("Value not acceptable.\n");
                while(getchar() != '\n'); /* flush invalid input */
                now = now->next;
                continue;
            }

            now->record->cumulative = newCumulative;
            printf("Value updated : \tCumulative = %lld\n", newCumulative);
            found = true;
        }
        now = now->next;
    }

    if(!found) {
        printf("[!] No Record in this date: %s\n", date);
    }
}

/* Removes all NodeC chain nodes whose record->date equals 'date'.
   Maintains a 'before' pointer to re-link the chain around deleted nodes.
   The underlying RecordC data is not freed here (it lives in 'original'). */
void deleteByDate(HashMap* map, char* date) {
    int index    = hash_date(date, map->m);
    NodeC *now   = map->buckets[index];
    NodeC *before = NULL; /* tracks the predecessor so we can splice 'now' out */
    bool found   = false;

    while(now) {
        if(strcmp(now->record->date, date) == 0) {
            NodeC* deleting = now;

            if(before) {
                before->next = now->next;    /* bypass 'now' in the chain */
            } else {
                map->buckets[index] = now->next; /* deleted node was the head */
            }

            now = now->next;
            free(deleting);           /* free the chain-node wrapper only */
            map->totalRecords--;
            found = true;
        } else {
            before = now;
            now    = now->next;
        }
    }

    if(!found) {
        printf("[!] No Record in this date: %s\n", date);
    }
}


/* ============================================================
 *  RUNNER FUNCTIONS  (one per exercise sub-choice)
 * ============================================================ */

/* Exercise A runner: loads the CSV into an AVL BST keyed on Date,
   then enters the interactive menu loop until the user exits. */
void runBST_date() {
    const char *filename = "effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv";

    /* Allocate a flat array to hold all CSV rows; the BST nodes will
       store pointers into this array (no extra copies of the data). */
    Record *original = malloc(MAX_ROWS * sizeof(Record));
    if (!original) {
        printf("Memory Error\n");
        return;
    }

    int n = load_csv(filename, original);
    if (n <= 0) {
        free(original);
        return;
    }

    NodeA* root = buildTreeA(original, n); /* build AVL tree from all records */

    int choice = 0;
    int date[3];
    long long cumulative;

    while(choice != 5) {
        choice = printMenuA();

        switch(choice) {
            case 1:
                /* In-order traversal prints all (date, cumulative) pairs
                   in ascending date order. */
                inorderA(root);
                break;

            case 2:
                /* Search: validate user-supplied date then look it up. */
                printf("Enter date (DD/MM/YYYY): ");
                scanf("%d/%d/%d", &date[0], &date[1], &date[2]);
                if(date[0] > 31 || date[0] < 1) { printf("Day is invalid.\n");        break; }
                if(date[1] > 12 || date[1] < 1) { printf("Month is invalid.\n");      break; }
                if(date[2] > 2021 || date[2] < 2015) { printf("Year out of bounds.\n"); break; }
                {
                    NodeA* found = searchA(root, date);
                    if(found) {
                        printf("Cumulative: %lld\n", found->record->cumulative);
                    } else {
                        printf("Date not found.\n");
                    }
                }
                break;

            case 3:
                /* Modify: validate date, read new value, update the node in-place. */
                printf("Enter date (DD/MM/YYYY): ");
                scanf("%d/%d/%d", &date[0], &date[1], &date[2]);
                if(date[0] > 31 || date[0] < 1) { printf("Day is invalid.\n");        break; }
                if(date[1] > 12 || date[1] < 1) { printf("Month is invalid.\n");      break; }
                if(date[2] > 2021 || date[2] < 2015) { printf("Year out of bounds.\n"); break; }
                printf("New Cumulative value: ");
                scanf("%lld", &cumulative);
                root = modifyA(root, date, cumulative);
                break;

            case 4:
                /* Delete: validate date then remove the node and rebalance. */
                printf("Enter date (DD/MM/YYYY): ");
                scanf("%d/%d/%d", &date[0], &date[1], &date[2]);
                if(date[0] > 31 || date[0] < 1) { printf("Day is invalid.\n");        break; }
                if(date[1] > 12 || date[1] < 1) { printf("Month is invalid.\n");      break; }
                if(date[2] > 2021 || date[2] < 2015) { printf("Year out of bounds.\n"); break; }
                root = deleteNodeA(root, date);
                break;

            case 5:
                printf("Ending program. . .\n");
                break;

            default:
                printf("Invalid choice.\n");
        }
    }

    free(original); /* the flat Record array; BST nodes held pointers into it */
}

/* Exercise B runner: loads the CSV into an AVL BST keyed on Cumulative,
   then enters the interactive menu loop until the user exits. */
void runBST_cumulative() {
    const char *filename = "effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv";

    Record *original = malloc(MAX_ROWS * sizeof(Record));
    if (!original) {
        printf("Memory Error\n");
        return;
    }

    int n = load_csv(filename, original);
    if (n <= 0) {
        free(original);
        return;
    }

    /* Build the Cumulative-keyed AVL tree; ties go into each node's equal-list. */
    NodeB* root = buildTreeB(original, n);

    int choice = 0;

    while(choice != 3) {
        choice = printMenuB();

        switch(choice) {
            case 1:
                /* Walk to the leftmost node (global minimum Cumulative)
                   and print it together with all ties. */
                printDates_MinB(root);
                break;

            case 2:
                /* Walk to the rightmost node (global maximum Cumulative)
                   and print it together with all ties. */
                printDates_MaxB(root);
                break;

            case 3:
                printf("Ending program. . .\n");
                break;

            default:
                printf("Invalid choice.\n");
        }
    }

    free(original);
}

/* Exercise C runner: loads the CSV into a hash table with chaining,
   then enters the interactive menu loop until the user exits. */
void runHashing() {
    const char *filename = "effects-of-covid-19-on-trade-at-15-december-2021-provisional.csv";

    /* RecordC keeps the date as a string, required by hash_date(). */
    RecordC *original = malloc(MAX_ROWS * sizeof(RecordC));
    if (!original) {
        printf("Memory Error\n");
        return;
    }

    int n = load_csv_C(filename, original);
    if (n <= 0) {
        free(original);
        return;
    }

    /* Create the hash table.  DATEC (11) is used as the bucket count here;
       in a production system this would be a larger odd prime. */
    HashMap* map = createMap(DATEC);

    /* Insert every record into the hash table (O(n) total, O(1) each). */
    for(int i = 0; i < n; i++) {
        insertToMap(map, &original[i]);
    }

    int choice = 0;
    char date[DATEC + 4]; /* small extra margin beyond the 10-char date */

    while(choice != 4) {
        choice = printMenuC();
        getchar(); /* consume the '\n' left in stdin by scanf */

        /* Read the date string before the switch so it is available to all cases. */
        if(choice == 1 || choice == 2 || choice == 3) {
            printf("Enter the date (e.g. 13/03/2018): ");
            if(!fgets(date, sizeof(date), stdin)) continue;
            date[strcspn(date, "\r\n")] = 0; /* strip trailing newline */
        }

        switch(choice) {
            case 1:
                searchByDate(map, date);
                break;

            case 2:
                modifyByDate(map, date);
                break;

            case 3:
                deleteByDate(map, date);
                break;

            case 4:
                printf("Ending program. . .\n");
                break;

            default:
                printf("Invalid choice.\n");
        }
    }

    destroyMap(map);  /* free all chain nodes */
    free(original);   /* free the flat RecordC array */
}


/* ============================================================
 *  MAIN
 * ============================================================ */

/* Entry point.
 * Presents the top-level menu asking the user to choose between BST or Hashing.
 * If BST is chosen, a second menu asks whether to key on Date (Exercise A)
 * or Cumulative (Exercise B).
 * The chosen runner function handles all further interaction and cleanup. */
int main() {
    int choice = 0;

    while(choice != 3) {
        choice = printMenuBSTorHashing();

        switch(choice) {
            case 1:
                /* BST selected — ask which ordering key to use */
                choice = printMenuBST_datecumulative();

                switch(choice) {
                    case 1:
                        runBST_date();        /* Exercise A */
                        break;
                    case 2:
                        runBST_cumulative();  /* Exercise B */
                        break;
                    default:
                        printf("Invalid choice.\n");
                }
                break;

            case 2:
                runHashing();  /* Exercise C */
                break;

            case 3:
                printf("Ending program. . .\n");
                break;

            default:
                printf("Invalid choice.\n");
        }
    }

    return 0;
}