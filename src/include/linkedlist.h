#ifndef H_LINKEDLIST
#define H_LINKEDLIST

/* Linked list node */
typedef struct FE_List {
    void *data;
    struct FE_List *next;
} FE_List;

/* Adds a node to the end of the list given in head */
void FE_List_Add(FE_List **head, void *data);

/* Removes a node from the list from data */
int FE_List_Remove(FE_List **head, void *data);

/* Destroys all nodes in the list * - NOTE does not free object data */
int FE_List_Destroy(FE_List **head);

#endif