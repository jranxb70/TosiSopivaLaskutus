#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include "ErrorStructure.h"



typedef struct node {
    SQLERRORDETAILS* val;
    struct node* next;
} node_t;

//SQLERRORDETAILS val;

int Append(
    _Inout_ node_t**                    pHead, 
    _In_ SQLERRORDETAILS*               err_detail);

void DeleteList(
    _In_ node_t**                       pHead);

#endif // LIST_H