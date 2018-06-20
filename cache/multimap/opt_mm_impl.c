#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "multimap.h"

/* The block_size of cache. */
#define BLOCK_SIZE 64

/* The size to allocate when we need more space for value-array memory. 
 * Since each value is 4 bytes, every time we allocate for 64 bytes,
 * which is the size of cache block, and this might help cache to have
 * better hit rate. 
 */
#define LIST_SIZE 16

/* The size to allocate when we need more space for tree node memory. 
 * Since each node is 32 bytes, every time we allocate for 8 * 64 bytes,
 * which is the size of cache block, and this might help cache to have
 * better hit rate. 
 */
#define TREE_SIZE 16


/*============================================================================
 * TYPES
 *
 *   These types are defined in the implementation file so that they can
 *   be kept hidden to code outside this source file.  This is not for any
 *   security reason, but rather just so we can enforce that our testing
 *   programs are generic and don't have any access to implementation details.
 *============================================================================*/


/* Represents a key and its associated values in the multimap, as well as
 * pointers to the left and right child nodes in the multimap. */
typedef struct multimap_node {
    /* The key-value that this multimap node represents. */
    int key;

    /* Optimized version: in order to get a contiguous memory to hold all
     * the values, we change the data structure of linked list into int
     * array. In this way, we would improve the locality for cache.
     */
    int *values;

    /* The current number of elements the value array */
    int value_length;

    /* The total number of elements that the current value array memory block
     * can hold.
     */
    int value_size;

    /* This variable is used to make the multimap_node to be 32 bytes, so that
     * a block size can fit in two multimap_nodes.
     */
    int useless;

    /* The tree_index represents the relative position of a tree node in the
     * whole object pool. In this way, we can access the node by calling
     * tree_head[tree_index].
     */
    int tree_index;

    /* The left child of the multimap node.  This will reference nodes that
     * hold keys that are strictly less than this node's key. And similarly
     * we can access the left child by tree_head[left_child].
     */
    int left_child;

    /* The right child of the multimap node.  This will reference nodes that
     * hold keys that are strictly greater than this node's key. And similarly
     * we can access the right child by tree_head[right_child].
     */
    int right_child;
} multimap_node;


/* The entry-point of the multimap data structure. */
struct multimap {
    multimap_node *root;
};

/* We change the tree structure into multimap_node arrays. Before that, every
 * tree node is independently allocated, so the memory address of these node
 * are very far from each other. Here, we managed an object pool with
 * contiguous memory, in order to hold all the tree nodes. Each time we add
 * a new tree node, we get some space from the object pool to put it in.
 * The tree_head is simply the start of memory pool. Tree_length means
 * currently how many nodes are in the pool, and tree_size means how many nodes
 * in total the current pool can hold. If tree_length exceeds tree_size,
 * we reallocate a bigger pool and move everything to the new pool.
 */
multimap_node *tree_head;
int tree_length;
int tree_size;


/*============================================================================
 * HELPER FUNCTION DECLARATIONS
 *
 *   Declarations of helper functions that are local to this module.  Again,
 *   these are not visible outside of this module.
 *============================================================================*/

multimap_node * alloc_mm_node(multimap *mm);

multimap_node * find_mm_node(multimap *mm, multimap_node *root, int key,
                             int create_if_not_found);

void free_multimap_values(int *values);
void free_multimap_node(multimap_node *node);

/* Optimized version: helper functions */
void node_add_value(multimap_node *node, int value);


/*============================================================================
 * FUNCTION IMPLEMENTATIONS
 *============================================================================*/

/* Allocates a multimap node, and zeros out its contents so that we know what
 * the initial value of everything will be.
 */
multimap_node * alloc_mm_node(multimap *mm) {
    if (tree_head == NULL) {
        /* if there is currently no node, allocate one block */
        multimap_node *new_head = 
            (multimap_node *) malloc(TREE_SIZE * sizeof(multimap_node));

        tree_head = new_head;
        tree_size += TREE_SIZE;
    }
    else if (tree_length == tree_size) {
        /* if the current memory is full, allocate a bigger one */
        int new_size = (tree_size + TREE_SIZE) * sizeof(multimap_node);

        multimap_node *new_head = 
            (multimap_node *) realloc(tree_head, new_size);
        /* handles allocation error */
        if (new_head == NULL) {
            printf("Not enough memory.\n");
            exit(0);
        }

        tree_head = new_head;
        mm->root = tree_head;
        tree_size += TREE_SIZE;
    }
    /* nothing special */
    tree_length += 1;
    multimap_node *node = tree_head + tree_length - 1;
    /* clear the allocated node */
    bzero(node, sizeof(multimap_node));
    node->tree_index = tree_length - 1;
    return node;   
}    


/* This helper function searches for the multimap node that contains the
 * specified key.  If such a node doesn't exist, the function can initialize
 * a new node and add this into the structure, or it will simply return NULL.
 * The one exception is the root - if the root is NULL then the function will
 * return a new root node.
 */
multimap_node * find_mm_node(multimap *mm, multimap_node *root, int key,
                             int create_if_not_found) {
    multimap_node *node;

    /* If the entire multimap is empty, the root will be NULL. */
    if (root == NULL) {
        if (create_if_not_found) {
            root = alloc_mm_node(mm);
            root->key = key;
        }
        return root;
    }

    /* Now we know the multimap has at least a root node, so start there. */
    node = root;
    while (1) {
        if (node->key == key)
            break;

        if (node->key > key) {   /* Follow left child */
            if (node->left_child == 0 && create_if_not_found) {
                /* No left child, but caller wants us to create a new node. */
                int index = node->tree_index;
                multimap_node *new = alloc_mm_node(mm);
                new->key = key;

                /* here, note that the reallocation might move the whole block
                 * to somewhere else, thus the node pointer may be erased or
                 * modified. So we record the tree_index and access the node
                 * through tree_head.
                 */
                node = tree_head + index;
                node->left_child = new->tree_index;
            }
            node = tree_head + node->left_child;
            if (node == tree_head)
                return NULL;
        }
        else {                   /* Follow right child */
            if (node->right_child == 0 && create_if_not_found) {
                /* No right child, but caller wants us to create a new node. */
                int index = node->tree_index;
                multimap_node *new = alloc_mm_node(mm);
                new->key = key;

                /* same for right child part */
                node = tree_head + index;
                node->right_child = new->tree_index;
            }
            node = tree_head + node->right_child;
            if (node == tree_head)
                return NULL;
        }

        if (node == NULL)
            break;
    }

    return node;
}


/* Initialize a multimap data structure. */
multimap * init_multimap() {
    multimap *mm = malloc(sizeof(multimap));
    mm->root = NULL;

    /* initialize global variables */
    tree_head = NULL;
    tree_length = 0;
    tree_head = 0;
    return mm;
}


/* Release all dynamically allocated memory associated with the multimap
 * data structure.
 */
void clear_multimap(multimap *mm) {

    int i = 0;
    /* free all the values in each node */
    while (i < tree_length) {
        free(tree_head[i].values);
        i++;
    }

    /* free the whole tree */
    free(tree_head);
    tree_head = NULL;
    tree_length = 0;
    tree_size = 0;

    /* free the multimap */
    mm->root = NULL;
    // free(mm);
}

/* Add a value into the value-list of a node in a cache friendly way.
 * This function ensures that the elements in the value array have adjacent
 * memory addresses, and thus improving the locality of access.
 * We change the data structure from linked list to int array.
 * Initially we allocate a big memory block to hold multimap_value types,
 * and when the block is filled, we expand it into a even bigger coherent
 * memory block using realloc(). So, when the program is traversing value-
 * array, it goes down the contiguous memory.
 */
void node_add_value(multimap_node *node, int value) {
    
    if (node->values == NULL) {
        assert(node->value_size == 0);
        assert(node->value_length == 0);

        /* if the node has no values inside, allocate one big block */
        int *new_value_head = (int *) malloc(LIST_SIZE * sizeof(int));

        /* update node attributes */
        node->values = new_value_head;
        node->value_size += LIST_SIZE;
    }
    else if (node->value_size == node->value_length) {
        assert(node->value_length != 0);

        /* if the allocated memory is filled, allocate a larger one */     
        int new_size = (node->value_size + LIST_SIZE) * sizeof(int);

        /* use realloc to get a bigger dynamic allocation */
        int *new_value_head = (int *) realloc(node->values, new_size);

        node->values = new_value_head;
        node->value_size += LIST_SIZE;
    }
    /* no dynamic allocation needed, simply add the value */
    node->value_length += 1;
    node->values[node->value_length - 1] = value;
}


/* Adds the specified (key, value) pair to the multimap. */
void mm_add_value(multimap *mm, int key, int value) {
    multimap_node *node;

    assert(mm != NULL);

    /* Look up the node with the specified key.  Create if not found. */
    node = find_mm_node(mm, mm->root, key, /* create */ 1);

    /* If the root is NULL, find_mm_node would return a new root node. */
    if (mm->root == NULL)
        mm->root = node;

    assert(node != NULL);
    assert(node->key == key);

    /* Add the new value to the multimap node. */
    node_add_value(node, value);
}


/* Returns nonzero if the multimap contains the specified key-value, zero
 * otherwise.
 */
int mm_contains_key(multimap *mm, int key) {
    return find_mm_node(mm, mm->root, key, /* create */ 0) != NULL;
}


/* Returns nonzero if the multimap contains the specified (key, value) pair,
 * zero otherwise.
 */
int mm_contains_pair(multimap *mm, int key, int value) {
    multimap_node *node;
    int *curr;

    node = find_mm_node(mm, mm->root, key, /* create */ 0);
    if (node == NULL)
        return 0;

    /* (modified to array traversal) */
    curr = node->values;
    int i = 0;
    int length = node->value_length;
    while (i < length) {
        if (curr[i] == value)
            return 1;
        i++;
    }

    return 0;
}


/* This helper function is used by mm_traverse() to traverse every pair within
 * the multimap.
 */
void mm_traverse_helper(multimap_node *node, void (*f)(int key, int value)) {
    int *curr;

    if (node == NULL)
        return;

    if (node->left_child != 0)
        mm_traverse_helper(tree_head + node->left_child, f);

    /* (modified to array traversal) */
    curr = node->values;
    int i = 0;
    int length = node->value_length;
    while (i < length) {
        f(node->key, curr[i]);
        i++;
    }

    if (node->right_child != 0)
        mm_traverse_helper(tree_head + node->right_child, f);
}


/* Performs an in-order traversal of the multimap, passing each (key, value)
 * pair to the specified function.
 */
void mm_traverse(multimap *mm, void (*f)(int key, int value)) {
    mm_traverse_helper(mm->root, f);
}

