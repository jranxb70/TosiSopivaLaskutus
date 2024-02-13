#ifndef SQLERROR_UTIL_H
#define SQLERROR_UTIL_H

#include <stdio.h>
#include <Windows.h>
#include <sql.h>
#include <sqlext.h>

#include "ErrorStructure.h"
#include "SqlErrorUtil.h"
#include "list.h"


/*
* 
*/
void SQLErrorUtil(
	_In_ SQLRETURN				retcode, 
	_In_ SQLHSTMT				hstmt,
	_Out_ node_t**				node);

#endif // SQLERROR_UTIL_H
