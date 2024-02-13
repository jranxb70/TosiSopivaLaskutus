#include "list.h"

int Append(_Inout_ node_t** pHead, _In_ SQLERRORDETAILS* err_detail)
{
    if (!*pHead)
    {
        *pHead = (node_t*)malloc(sizeof(node_t));
        if (*pHead == NULL) {
            return 1;
        }
        (*pHead)->val = err_detail;
        (*pHead)->next = NULL;
    }
    else
    {
        node_t* node = *pHead;
        while (node->next != NULL)
        {
            (node) = (node)->next;
        }

        node->next = (node_t*)malloc(sizeof(node_t));
        if (node == NULL) {
            return 1;
        }
        (node)->next->val = err_detail;
        (node)->next->next = NULL;
    }
    return 0;
}

void DeleteList(_In_ node_t** pHead)
{
    node_t* currentNode = *pHead;
    node_t* nextNode;

    while (currentNode != NULL)
    {
        nextNode = currentNode->next;
        free(currentNode->val);
        free(currentNode);
        currentNode = nextNode;
    }

    *pHead = NULL;
}
