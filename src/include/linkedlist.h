#ifndef H_LINKEDLIST
#define H_LINKEDLIST

/* Linked list node */
typedef struct FE_List {
    void *data;
    struct FE_List *next;
} FE_List;


/** Adds a node to the end of the list
    *\param head The head of the list to add to
    *\param data The data to add to the list
*/
void FE_List_Add(FE_List **head, void *data);


/* Removes a node from the list from data */
int FE_List_Remove(FE_List **head, void *data);


/** Destroys all nodes in the list. This does not free the data in the list
 *\param head The head of the list to destroy
 */
int FE_List_Destroy(FE_List **head);

#endif