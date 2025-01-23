#include "ex6.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# define INT_BUFFER 128

// ================================================
// Basic struct definitions from ex6.h assumed:
//   PokemonData { int id; char *name; PokemonType TYPE; int hp; int attack; EvolutionStatus CAN_EVOLVE; }
//   PokemonNode { PokemonData* data; PokemonNode* left, *right; }
//   OwnerNode   { char* ownerName; PokemonNode* pokedexRoot; OwnerNode *next, *prev; }
//   OwnerNode* ownerHead;
//   const PokemonData pokedex[];
// ================================================

// --------------------------------------------------------------
// 1) Safe integer reading
// --------------------------------------------------------------

void trimWhitespace(char *str)
{
    // Remove leading spaces/tabs/\r
    int start = 0;
    while (str[start] == ' ' || str[start] == '\t' || str[start] == '\r')
        start++;

    if (start > 0)
    {
        int idx = 0;
        while (str[start])
            str[idx++] = str[start++];
        str[idx] = '\0';
    }

    // Remove trailing spaces/tabs/\r
    int len = (int)strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\r'))
    {
        str[--len] = '\0';
    }
}

char *myStrdup(const char *src)
{
    if (!src)
        return NULL;
    size_t len = strlen(src);
    char *dest = (char *)malloc(len + 1);
    if (!dest)
    {
        printf("Memory allocation failed in myStrdup.\n");
        return NULL;
    }
    strcpy(dest, src);
    return dest;
}

int readIntSafe(const char *prompt)
{
    char buffer[INT_BUFFER];
    int value;
    int success = 0;

    while (!success)
    {
        printf("%s", prompt);

        // If we fail to read, treat it as invalid
        if (!fgets(buffer, sizeof(buffer), stdin))
        {
            printf("Invalid input.\n");
            clearerr(stdin);
            continue;
        }

        // 1) Strip any trailing \r or \n
        //    so "123\r\n" becomes "123"
        size_t len = strlen(buffer);
        if (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r'))
            buffer[--len] = '\0';
        if (len > 0 && (buffer[len - 1] == '\r' || buffer[len - 1] == '\n'))
            buffer[--len] = '\0';

        // 2) Check if empty after stripping
        if (len == 0)
        {
            printf("Invalid input.\n");
            continue;
        }

        // 3) Attempt to parse integer with strtol
        char *endptr;
        value = (int)strtol(buffer, &endptr, 10);

        // If endptr didn't point to the end => leftover chars => invalid
        // or if buffer was something non-numeric
        if (*endptr != '\0')
        {
            printf("Invalid input.\n");
        }
        else
        {
            // We got a valid integer
            success = 1;
        }
    }
    return value;
}

// --------------------------------------------------------------
// 2) Utility: Get type name from enum
// --------------------------------------------------------------
const char *getTypeName(PokemonType type)
{
    switch (type)
    {
    case GRASS:
        return "GRASS";
    case FIRE:
        return "FIRE";
    case WATER:
        return "WATER";
    case BUG:
        return "BUG";
    case NORMAL:
        return "NORMAL";
    case POISON:
        return "POISON";
    case ELECTRIC:
        return "ELECTRIC";
    case GROUND:
        return "GROUND";
    case FAIRY:
        return "FAIRY";
    case FIGHTING:
        return "FIGHTING";
    case PSYCHIC:
        return "PSYCHIC";
    case ROCK:
        return "ROCK";
    case GHOST:
        return "GHOST";
    case DRAGON:
        return "DRAGON";
    case ICE:
        return "ICE";
    default:
        return "UNKNOWN";
    }
}

// --------------------------------------------------------------
// Utility: getDynamicInput (for reading a line into malloc'd memory)
// --------------------------------------------------------------
char *getDynamicInput()
{
    char *input = NULL;
    size_t size = 0, capacity = 1;
    input = (char *)malloc(capacity);
    if (!input)
    {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
        if (size + 1 >= capacity)
        {
            capacity *= 2;
            char *temp = (char *)realloc(input, capacity);
            if (!temp)
            {
                printf("Memory reallocation failed.\n");
                free(input);
                return NULL;
            }
            input = temp;
        }
        input[size++] = (char)c;
    }
    input[size] = '\0';

    // Trim any leading/trailing whitespace or carriage returns
    trimWhitespace(input);

    return input;
}

// Function to print a single Pokemon node
void printPokemonNode(PokemonNode *node)
{
    if (!node)
        return;
    printf("ID: %d, Name: %s, Type: %s, HP: %d, Attack: %d, Can Evolve: %s\n",
           node->data->id,
           node->data->name,
           getTypeName(node->data->TYPE),
           node->data->hp,
           node->data->attack,
           (node->data->CAN_EVOLVE == CAN_EVOLVE) ? "Yes" : "No");
}

/* ------------------------------------------------------------
   2) Creating & Freeing Nodes
   ------------------------------------------------------------ */

/**
 * @brief Create a BST node with a copy of the given PokemonData.
 * @param data pointer to PokemonData (like from the global pokedex)
 * @return newly allocated PokemonNode*
 * Why we made it: We need a standard way to allocate BST nodes.
 */
PokemonNode *createPokemonNode(const PokemonData *data) {

    PokemonNode* root = malloc(sizeof(PokemonNode));
    if (root == NULL) {
        return NULL;
    }
    root->data = data;
    root->left = NULL;
    root->right = NULL;
    return root;
}

/**
 * @brief Create an OwnerNode for the circular owners list.
 * @param ownerName the dynamically allocated name
 * @param starter BST root for the starter Pokemon
 * @return newly allocated OwnerNode*
 * Why we made it: Each user is represented as an OwnerNode.
 */
OwnerNode *createOwner(char *ownerName, PokemonNode *starter) {

    if (ownerName == NULL)
        return NULL;

    OwnerNode *newOwnerNode = malloc(sizeof(OwnerNode));

    if (!newOwnerNode) {
        return newOwnerNode;
    }

    newOwnerNode->ownerName = ownerName;
    newOwnerNode->pokedexRoot = starter;
    newOwnerNode->next = newOwnerNode;
    newOwnerNode->prev = newOwnerNode;
    return newOwnerNode;
}



/**
 * @brief Free one PokemonNode (including name).
 * @param node pointer to node
 * Why we made it: Avoid memory leaks for single nodes.
 */
void freePokemonNode(PokemonNode *node) {
    //free(node->data);
    node->data = NULL;
    free(node);
}

/**
 * @brief Recursively free a BST of PokemonNodes.
 * @param root BST root
 * Why we made it: Clearing a user’s entire Pokedex means freeing a tree.
 */
void freePokemonTree(PokemonNode *root) {
    if (root == NULL) {
        return;
    }
    freePokemonTree(root->left);
    root->left = NULL;
    freePokemonTree(root->right);
    root->right = NULL;
    freePokemonNode(root);
}

/**
 * @brief Free an OwnerNode (including name and entire Pokedex BST).
 * @param owner pointer to the owner
 * Why we made it: Deleting an owner also frees their Pokedex & name.
 */
void freeOwnerNode(OwnerNode *owner) {
    free(owner->ownerName);
    freePokemonTree(owner->pokedexRoot);
    owner->pokedexRoot = NULL;
    free(owner);
}


/* ------------------------------------------------------------
   3) BST Insert, Search, Remove
   ------------------------------------------------------------ */

/* Implement Simple FIFO using queue for BFS */

typedef struct QueueNode {
    PokemonNode* data;
    struct QueueNode* next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
} Queue;

// Queue operations
Queue* createQueue(void) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

void enqueue(Queue* q, PokemonNode* node) {
    QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
    newNode->data = node;
    newNode->next = NULL;
    if (q->rear == NULL) {
        q->front = q->rear = newNode;
        return;
    }
    q->rear->next = newNode;
    q->rear = newNode;
}

PokemonNode* dequeue(Queue* q) {
    if (q->front == NULL)
        return NULL;

    QueueNode* temp = q->front;
    PokemonNode* node = temp->data;

    q->front = q->front->next;

    if (q->front == NULL)
        q->rear = NULL;

    free(temp);
    return node;
}

/**
 * @brief Insert a PokemonNode into BST by ID; duplicates freed.
 * @param root pointer to BST root
 * @param newNode node to insert
 * @return updated BST root
 * Why we made it: Standard BST insertion ignoring duplicates.
 */
PokemonNode *insertPokemonNode(PokemonNode *root, PokemonNode *newNode) {

    // If the tree is empty, create a new node
    if (root == NULL) {
        return newNode;
    }

    // If the ID already exists, don't insert (no duplicates allowed)
    if (newNode->data->id == root->data->id) {
        printf("Pokemon with ID %d already exists. Duplicate not allowed.\n", newNode->data->id);
        return root;
    }

    // Recursively insert into the appropriate subtree
    if (newNode->data->id < root->data->id) {
        root->left = insertPokemonNode(root->left, newNode);
    } else {
        root->right = insertPokemonNode(root->right, newNode);
    }

    return root;
}

/**
 * @brief BFS search for a Pokemon by ID in the BST.
 * @param root BST root
 * @param id ID to find
 * @return pointer to found node or NULL
 * Why we made it: BFS ensures we find nodes even in an unbalanced tree.
 */
// Queue structure for BFS
PokemonNode *searchPokemonBFS(PokemonNode *root, int id) {

    if (root == NULL) {
        return NULL;
    }

    Queue* q = createQueue();
    enqueue(q, root);

    while (q->front != NULL) {
        PokemonNode* current = dequeue(q);
        if (current->data->id == id) {
            free(q);
            return current;
        }
        if (current->left)
            enqueue(q, current->left);
        if (current->right)
            enqueue(q, current->right);
    }

    free(q);
    return NULL;  // Pokemon not found
}

PokemonNode* findMin(PokemonNode* node) {
    while (node->left != NULL)
        node = node->left;
    return node;
}

/**
 * @brief Remove node from BST by ID if found (BST removal logic).
 * @param root BST root
 * @param id ID to remove
 * @return updated BST root
 * Why we made it: We handle special cases of a BST remove (0,1,2 children).
 */
PokemonNode *removeNodeBST(PokemonNode *root, int id) {

    if (root == NULL) {
        return root;
    }

    if (id < root->data->id)
        root->left = removeNodeBST(root->left, id);
    else if (id > root->data->id)
        root->right = removeNodeBST(root->right, id);
    else {
        // Node with only one child or no child
        if (root->left == NULL) {
            PokemonNode *temp = root->right;
            freePokemonNode(root);
            return temp;
        } else if (root->right == NULL) {
            PokemonNode *temp = root->left;
            freePokemonNode(root);
            return temp;
        }

        // Node with two children
        PokemonNode *temp = findMin(root->right);
        root->data = temp->data;
        root->right = removeNodeBST(root->right, temp->data->id);
    }
    return root;
}

/**
 * @brief Combine BFS search + BST removal to remove Pokemon by ID.
 * @param root BST root
 * @param id the ID to remove
 * @return updated BST root
 * Why we made it: BFS confirms existence, then removeNodeBST does the removal.
 */
PokemonNode *removePokemonByID(PokemonNode *root, int id) {

    if (searchPokemonBFS(root, id))
        root = removeNodeBST(root, id);

    return root;
}

/* ------------------------------------------------------------
   4) Generic BST Traversals (Function Pointers)
   ------------------------------------------------------------ */

// Please notice, it's not really generic, it's just a demonstration of function pointers.
// so don't be confused by the name, but please remember that you must use it.


/**
 * @brief Generic BFS traversal: call visit() on each node (level-order).
 * @param root BST root
 * @param visit function pointer for what to do with each node
 * Why we made it: BFS plus function pointers => flexible traversal.
 */
void BFSGeneric(PokemonNode *root, VisitNodeFunc visit) {

    if (root == NULL)
        return;

    Queue* q = createQueue();
    enqueue(q, root);

    while (q->front != NULL) {
        PokemonNode* current = dequeue(q);

        // Call the visit function on the current node
        visit(current);

        if (current->left)
            enqueue(q, current->left);
        if (current->right)
            enqueue(q, current->right);
    }

    free(q);
}

/**
 * @brief A generic pre-order traversal (Root-Left-Right).
 * @param root BST root
 * @param visit function pointer
 * Why we made it: Another demonstration of function-pointer-based traversal.
 */
void preOrderGeneric(PokemonNode *root, VisitNodeFunc visit) {

    if (root == NULL) {
        return;
    }

    visit(root);

    preOrderGeneric(root->left, visit);

    preOrderGeneric(root->right, visit);
}

/**
 * @brief A generic in-order traversal (Left-Root-Right).
 * @param root BST root
 * @param visit function pointer
 * Why we made it: Great for seeing sorted order if BST is sorted by ID.
 */
void inOrderGeneric(PokemonNode *root, VisitNodeFunc visit) {

    if (root == NULL) {
        return;
    }


    preOrderGeneric(root->left, visit);

    visit(root);

    preOrderGeneric(root->right, visit);
}

/**
 * @brief A generic post-order traversal (Left-Right-Root).
 * @param root BST root
 * @param visit function pointer
 * Why we made it: Another standard traversal pattern.
 */
void postOrderGeneric(PokemonNode *root, VisitNodeFunc visit) {

    if (root == NULL) {
        return;
    }


    preOrderGeneric(root->left, visit);

    preOrderGeneric(root->right, visit);

    visit(root);
}

/* ------------------------------------------------------------
   5) Display Methods (BFS, Pre, In, Post, Alphabetical)
   ------------------------------------------------------------ */

/**
 * @brief Initialize a NodeArray with given capacity.
 * @param na pointer to NodeArray
 * @param cap initial capacity
 * Why we made it: We store pointers to PokemonNodes for alphabetical sorting.
 */
void initNodeArray(NodeArray *na, int cap) {

    if (na == NULL || cap < 1) {
        return;
    }

    na->nodes = calloc(cap, sizeof(PokemonNode*));
    if (!na->nodes)
	return;
    na->capacity = cap;
    na->size = 0;
}

void destroyNodeArray(NodeArray *na) {
     if (na->nodes)
        free(na->nodes);
     na->nodes = NULL;
     na->capacity = 0;
     na->size = 0;
}

/**
 * @brief Add a PokemonNode pointer to NodeArray, realloc if needed.
 * @param na pointer to NodeArray
 * @param node pointer to the node
 * Why we made it: We want a dynamic list of BST nodes for sorting.
 */
void addNode(NodeArray *na, PokemonNode *node) {
    if (na == NULL || node == NULL)
        return;
    if (na->size == na->capacity) {
        na->nodes = realloc(na->nodes, na->capacity * 2);
        na->capacity *= 2;
    }
    na->nodes[na->size++] = node;
}

/**
 * @brief Recursively collect all nodes from the BST into a NodeArray.
 * @param root BST root
 * @param na pointer to NodeArray
 * Why we made it: We gather everything for qsort.
 */
void collectAll(PokemonNode *root, NodeArray *na)  {

    if (root == NULL)
        return;

    collectAll(root->left, na);
    addNode(na, root);
    collectAll(root->right, na);

}

/**
 * @brief Compare function for qsort (alphabetical by node->data->name).
 * @param a pointer to a pointer to PokemonNode
 * @param b pointer to a pointer to PokemonNode
 * @return -1, 0, or +1
 * Why we made it: Sorting by name for alphabetical display.
 */
int compareByNameNode(const void *a, const void *b) {

   PokemonNode *pokemonA = *(PokemonNode **)a;
   PokemonNode *pokemonB = *(PokemonNode **)b;

   /* retunrs negatie or positive not just -1 or 1, but this should be okay for qsort */
   return strcmp(pokemonA->data->name, pokemonB->data->name);
    
}

/**
 * @brief BFS is nice, but alphabetical means we gather all nodes, sort by name, then print.
 * @param root BST root
 * Why we made it: Provide user the option to see Pokemon sorted by name.
 */
void displayAlphabetical(PokemonNode *root) {
    NodeArray na;
    initNodeArray(&na, 50);
    collectAll(root, &na);
    qsort(na.nodes,  na.size, sizeof(PokemonNode *), compareByNameNode);
    for (int i = 0; i < na.size; i++) {
        printPokemonNode(na.nodes[i]);            
    }
    destroyNodeArray(&na);
}

/**
 * @brief BFS user-friendly display (level-order).
 * @param root BST root
 * Why we made it: Quick listing in BFS order.
 */
void displayBFS(PokemonNode *root) {

    BFSGeneric(root, printPokemonNode);
}

/**
 * @brief Pre-order user-friendly display (Root->Left->Right).
 * @param root BST root
 * Why we made it: Another standard traversal for demonstration.
 */
void preOrderTraversal(PokemonNode *root) {

    preOrderGeneric(root, printPokemonNode);

}

/**
 * @brief In-order user-friendly display (Left->Root->Right).
 * @param root BST root
 * Why we made it: Good for sorted output by ID if the tree is a BST.
 */
void inOrderTraversal(PokemonNode *root) {
    inOrderGeneric(root, printPokemonNode);
}

/**
 * @brief Post-order user-friendly display (Left->Right->Root).
 * @param root BST root
 * Why we made it: Another standard traversal pattern.
 */
void postOrderTraversal(PokemonNode *root) {
    postOrderGeneric(root, printPokemonNode);
}

/* ------------------------------------------------------------
   6) Pokemon-Specific
   ------------------------------------------------------------ */

static float pokemonCalcStregth(PokemonData *data) {
    return data->hp * 1.2 + data->attack * 1.5;
}
/**
 * @brief Let user pick two Pokemon by ID in the same Pokedex to fight.
 * @param owner pointer to the Owner
 * Why we made it: Fun demonstration of BFS and custom formula for battles.
 */
void pokemonFight(OwnerNode *owner) {

    if (!owner)
        return;

    PokemonNode *root = owner->pokedexRoot;
    if (root == NULL) {
        printf("Pokedex is empty.");
    }

    int choice1 = readIntSafe("Enter ID of the first Pokemon: ");
    int choice2 = readIntSafe("Enter ID of the second Pokemon: ");

    PokemonNode *pokemon1 = searchPokemonBFS(root, choice1); 
    PokemonNode *pokemon2 = searchPokemonBFS(root, choice2);

    if (pokemon1 == NULL || pokemon2 == NULL) {
        printf("One or both Pokemon IDs not found.");
        return;
    }

    float strength1 = pokemonCalcStregth(pokemon1->data);
    float strength2 = pokemonCalcStregth(pokemon2->data);
    printf("Pokemon 1: %s (Score = %f)",pokemon1->data->name, strength1);
    printf("Pokemon 2: %s (Score = %f)",pokemon2->data->name, strength2);
    if (strength1 > strength2) {
        printf("%s wins!", pokemon1->data->name);
    } else if(strength1 < strength2) {
        printf("%s wins!", pokemon2->data->name);
    }
    else {
        printf("It’s a tie!");
    }
}

/**
 * @brief Evolve a Pokemon (ID -> ID+1) if allowed.
 * @param owner pointer to the Owner
 * Why we made it: Demonstrates removing an old ID, inserting the next ID.
 */
void evolvePokemon(OwnerNode *owner) {

    if (!owner)
        return;

    if (owner->pokedexRoot == NULL) {
        printf("Pokedex is empty.");
    }

    int choice = readIntSafe("Enter ID of Pokemon to evolve: ");
    PokemonNode *pokemon = searchPokemonBFS(owner->pokedexRoot, choice);
    if (!pokemon)
        return;
    if (pokemon->data->CAN_EVOLVE) {
        owner->pokedexRoot = removeNodeBST(owner->pokedexRoot, choice);
        owner->pokedexRoot = insertPokemonNode(owner->pokedexRoot, createPokemonNode(&pokedex[choice]));
    }
}

/**
 * @brief Prompt for an ID, BFS-check duplicates, then insert into BST.
 * @param owner pointer to the Owner
 * Why we made it: Primary user function for adding new Pokemon to an owner’s Pokedex.
 */
void addPokemon(OwnerNode *owner) {
    if (!owner)
        return;

    int choice = readIntSafe("Enter ID to add: ");
    if (choice < 1 && choice > 151)
        return;
    PokemonNode *pokemon = searchPokemonBFS(owner->pokedexRoot, choice);
    if (pokemon)
        return;
    owner->pokedexRoot = insertPokemonNode(owner->pokedexRoot, createPokemonNode(&pokedex[choice - 1]));
}

/**
 * @brief Prompt for ID, remove that Pokemon from BST by ID.
 * @param owner pointer to the Owner
 * Why we made it: Another user function for releasing a Pokemon.
 */
void freePokemon(OwnerNode *owner) {
    int choice = readIntSafe("Enter Pokemon ID to release: ");
    owner->pokedexRoot = removePokemonByID(owner->pokedexRoot, choice);
}

/* ------------------------------------------------------------
   7) Display Menu for a Pokedex
   ------------------------------------------------------------ */

/* ------------------------------------------------------------
   8) Sorting Owners (Bubble Sort on Circular List)
   ------------------------------------------------------------ */

/**
 * @brief Sort the circular owners list by name.
 * Why we made it: Another demonstration of pointer manipulation + sorting logic.
 */
void sortOwners(void) {

    if (ownerHead == NULL || ownerHead->next == ownerHead) {
        return;  // List is empty or has only one node
    }

    int swapped;
    OwnerNode *cur;
    OwnerNode *last = ownerHead->prev;  // Last node in the circular list

    do {
        swapped = 0;
        cur = ownerHead;

        while (cur != last) {
            if (strcmp(cur->ownerName, cur->next->ownerName) > 0) {
                swapOwnerData(cur, cur->next);
                swapped = 1;
            }
            cur = cur->next;
        }

        // Move last one step back
        last = last->prev;

    } while (swapped);
}

/**
 * @brief Helper to swap name & pokedexRoot in two OwnerNode.
 * @param a pointer to first owner
 * @param b pointer to second owner
 * Why we made it: Used internally by bubble sort to swap data.
 */
void swapOwnerData(OwnerNode *a, OwnerNode *b) {

    char *name = a->ownerName;
    PokemonNode *root = a->pokedexRoot;
    a->ownerName = b->ownerName;
    a->pokedexRoot = b->pokedexRoot;
    b->ownerName = name;
    b->pokedexRoot = root;
}

/* ------------------------------------------------------------
   9) Circular List Linking & Searching
   ------------------------------------------------------------ */

/**
 * @brief Insert a new owner into the circular list. If none exist, it's alone.
 * @param newOwner pointer to newly created OwnerNode
 * Why we made it: We need a standard approach to keep the list circular.
 */
void linkOwnerInCircularList(OwnerNode *newOwner) { 

    if (ownerHead == NULL) {
        ownerHead = newOwner;
    } else {
        OwnerNode* last = ownerHead->prev;
        newOwner->next = ownerHead;
        newOwner->prev = last;
        last->next = newOwner;
        ownerHead->prev = newOwner;
    }
}

/**
 * @brief Remove a specific OwnerNode from the circular list, possibly updating head.
 * @param target pointer to the OwnerNode
 * Why we made it: Deleting or merging owners requires removing them from the ring.
 */
void removeOwnerFromCircularList(OwnerNode *target) {

    if (target == NULL || ownerHead == NULL) {
        return;
    }
    if (target->next == target) {
        ownerHead = NULL;
    } else {
        target->prev->next = target->next;
        target->next->prev = target->prev;

        // If the node to unlink is the head, update the head
        if (target == ownerHead) {
            ownerHead = target->next;
        }
    }
    target->next = NULL;
    target->prev = NULL;
}

/**
 * @brief Find an owner by name in the circular list.
 * @param name string to match
 * @return pointer to the matching OwnerNode or NULL
 * Why we made it: We often need to locate an owner quickly.
 */
OwnerNode *findOwnerByName(const char *name) {

    if (ownerHead == NULL || name == NULL) {
        return NULL;
    }

    OwnerNode *cur = ownerHead;
    do {
        if (strcmp(cur->ownerName, name) == 0) {
            return cur;
    }
        cur = cur->next;
    } while (cur->next != ownerHead);
    return NULL;
}



int printAndCountOwners(void) {
    if (ownerHead == NULL) {
        return 0;
    }

    OwnerNode *cur = ownerHead;
    int cnt = 1;
    do {
        printf("%d. %s\n", cnt, cur->ownerName);
        cnt++;
        cur = cur->next;
    } while (cur != ownerHead);
    return cnt - 1;
}

OwnerNode *findOwnerByNum(int choice) {
 
    OwnerNode *cur = ownerHead;
    int cnt = 1;
    while (cnt++ < choice) {
        cur = cur->next;
    }
    return cur;

}



/* ------------------------------------------------------------
   10) Owner Menus
   ------------------------------------------------------------ */

/**
 * @brief Creates a new Pokedex (prompt for name, check uniqueness, choose starter).
 * Why we made it: The main entry for building a brand-new Pokedex.
 */
void openPokedexMenu(void) {

    printf("Your name: ");
    char *ownerName = getDynamicInput();
    int starter = readIntSafe("Choose Starter:\n1. Bulbasaur \n2. Charmander \n3. Squirtle\n");
    OwnerNode* newOwner = createOwner(ownerName, createPokemonNode(&pokedex[3 * (starter - 1)]));
    linkOwnerInCircularList(newOwner);
}

/**
 * @brief Delete an entire Pokedex (owner) from the list.
 * Why we made it: Let user pick which Pokedex to remove and free everything.
 */
void deletePokedex(void) {

    printf("=== Delete a Pokedex ===");
    int cnt = printAndCountOwners();
    int choice = readIntSafe("Choose a Pokedex to delete by number: ");
    if (choice > cnt)
        return;
    OwnerNode *owner = findOwnerByNum(choice);
    if (!owner)
        return;
    printf("Deleting %s's entire Pokedex...", owner->ownerName);
    freePokemonTree(owner->pokedexRoot);
    owner->pokedexRoot = NULL;
    printf("Pokedex deleted.");
}

/**
 * @brief Merge the second owner's Pokedex into the first, then remove the second owner.
 * Why we made it: BFS copy demonstration plus removing an owner.
 */
void mergePokedexMenu(void) {}

/* ------------------------------------------------------------
   11) Printing Owners in a Circle
   ------------------------------------------------------------ */

/**
 * @brief Print owners left or right from head, repeating as many times as user wants.
 * Why we made it: Demonstrates stepping through a circular list in a chosen direction.
 */
void printOwnersCircular(void) {

    OwnerNode *cur = ownerHead;
    if (!cur)
		return;

    printf("Enter direction (F or B): ");
    char direct = '\0';
    scanf("%c", &direct);
    int count = readIntSafe("How many prints? ");

    if (direct == 'F' || direct == 'f') {
        for (int i = 1; i <= count; i++) {
            printf("[%d] %s", i, cur->ownerName);
			cur = cur->next;
		}
	} else if (direct == 'B' || direct == 'b') {
        for (int i = 1; i <= count; i++) {
            printf("[%d] %s", i, cur->ownerName);
			cur = cur->prev;
		}
	}
}

/* ------------------------------------------------------------
   12) Cleanup All Owners at Program End
   ------------------------------------------------------------ */

/**
 * @brief Frees every remaining owner in the circular list, setting ownerHead = NULL.
 * Why we made it: Ensures a squeaky-clean exit with no leftover memory.
 */
void freeAllOwners(void) {

    if (ownerHead == NULL) {
        return;
    }

    OwnerNode *cur = ownerHead;
    OwnerNode* next;

    do {
        next = cur->next;
        freeOwnerNode(cur);
        cur = next;
    } while (cur != ownerHead);

    ownerHead = NULL;
}


// --------------------------------------------------------------
// Display Menu
// --------------------------------------------------------------
void displayMenu(OwnerNode *owner)
{
    if (!owner->pokedexRoot)
    {
        printf("Pokedex is empty.\n");
        return;
    }

    printf("Display:\n");
    printf("1. BFS (Level-Order)\n");
    printf("2. Pre-Order\n");
    printf("3. In-Order\n");
    printf("4. Post-Order\n");
    printf("5. Alphabetical (by name)\n");

    int choice = readIntSafe("Your choice: ");

    switch (choice)
    {
    case 1:
        displayBFS(owner->pokedexRoot);
        break;
    case 2:
        preOrderTraversal(owner->pokedexRoot);
        break;
    case 3:
        inOrderTraversal(owner->pokedexRoot);
        break;
    case 4:
        postOrderTraversal(owner->pokedexRoot);
        break;
    case 5:
        displayAlphabetical(owner->pokedexRoot);
        break;
    default:
        printf("Invalid choice.\n");
    }
}

// --------------------------------------------------------------
// Sub-menu for existing Pokedex
// --------------------------------------------------------------
void enterExistingPokedexMenu(void)
{
    // list owners
    printf("\nExisting Pokedexes:\n");

    int cnt = printAndCountOwners();

    int choice = readIntSafe("Choose a Pokedex by number:");
    if (choice > cnt)
        return;
    OwnerNode *cur = findOwnerByNum(choice);
    if (!cur)
        return;

    printf("\nEntering %s's Pokedex...\n", cur->ownerName);

    int subChoice;
    do
    {
        printf("\n-- %s's Pokedex Menu --\n", cur->ownerName);
        printf("1. Add Pokemon\n");
        printf("2. Display Pokedex\n");
        printf("3. Release Pokemon (by ID)\n");
        printf("4. Pokemon Fight!\n");
        printf("5. Evolve Pokemon\n");
        printf("6. Back to Main\n");

        subChoice = readIntSafe("Your choice: ");

        switch (subChoice)
        {
        case 1:
            addPokemon(cur);
            break;
        case 2:
            displayMenu(cur);
            break;
        case 3:
            freePokemon(cur);
            break;
        case 4:
            pokemonFight(cur);
            break;
        case 5:
            evolvePokemon(cur);
            break;
        case 6:
            printf("Back to Main Menu.\n");
            break;
        default:
            printf("Invalid choice.\n");
        }
    } while (subChoice != 6);
}

// --------------------------------------------------------------
// Main Menu
// --------------------------------------------------------------
void mainMenu()
{
    int choice;
    do
    {
        printf("\n=== Main Menu ===\n");
        printf("1. New Pokedex\n");
        printf("2. Existing Pokedex\n");
        printf("3. Delete a Pokedex\n");
        printf("4. Merge Pokedexes\n");
        printf("5. Sort Owners by Name\n");
        printf("6. Print Owners in a direction X times\n");
        printf("7. Exit\n");
        choice = readIntSafe("Your choice: ");

        switch (choice)
        {
        case 1:
            openPokedexMenu();
            break;
        case 2:
            enterExistingPokedexMenu();
            break;
        case 3:
            deletePokedex();
            break;
        case 4:
            mergePokedexMenu();
            break;
        case 5:
            sortOwners();
            break;
        case 6:
            printOwnersCircular();
            break;
        case 7:
            printf("Goodbye!\n");
            break;
        default:
            printf("Invalid.\n");
        }
    } while (choice != 7);
}

int main()
{
    mainMenu();
    freeAllOwners();
    return 0;
}
