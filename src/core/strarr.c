#include "include/strarr.h"
#include "include/mem.h"
#include "include/utils.h"
#include "lib/string.h"

FE_StrArr *FE_StrArr_Create()
{
    FE_StrArr *arr = xmalloc(sizeof(FE_StrArr));
    arr->items = 0;
    arr->data = 0;
    return arr;
}

void FE_StrArr_Destroy(FE_StrArr *arr)
{
    if (!arr) {
        warn("FE_StrArr_Destroy: arr is null");
        return;
    }
    
    for (size_t i = 0; i < arr->items; i++)
        free(arr->data[i]);
    free(arr->data);
    free(arr);
}

size_t FE_StrArr_Add(FE_StrArr *arr, char *str)
{
    if (!arr) {
        warn("FE_StrArr_Add: arr is null");
        return 0;
    }
    
    if (!str) {
        warn("FE_StrArr_Add: str is null");
        return 0;
    }
    
    if (!arr->data) {
        arr->data = xmalloc(sizeof(char *));
        arr->data[0] = mstrdup(str);
        arr->items = 1;
        return 1;
    }
    
    arr->data = xrealloc(arr->data, sizeof(char *) * (arr->items + 1));
    arr->data[arr->items] = mstrdup(str);
    return arr->items++;
}