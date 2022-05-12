#include "include/linkedlist.h"
#include "include/utils.h"

/* Adds a node to the end of the list in head */
void FE_List_Add(FE_List **head, void *data)
{
    FE_List *new = xmalloc(sizeof(FE_List));
    new->data = data;
    if (head) {
        new->next = *head;
    } else {
        new->next = 0;
    }
    *head = new;
}

/* Removes a node from the list by searching to find matching data*/
int FE_List_Remove(FE_List **head, void *data)
{
    FE_List *prev = 0;
    FE_List *curr = *head;
    while (curr) {
        if (curr->data == data) {
            if (prev) {
                prev->next = curr->next;
            } else {
                *head = curr->next;
            }
            free(curr);
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }
    warn("Could not find node to remove");
    return -1;
}

/* Destroys all nodes in the list * - NOTE does not free object data */
int FE_List_Destroy(FE_List **head)
{
    if (!head)
        return 0;

    FE_List *curr = *head;
    while (curr) {
        FE_List *next = curr->next;
        free(curr);
        curr = next;
    }
    *head = 0;
    return 1;
}
