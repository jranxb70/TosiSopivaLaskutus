#ifndef DATABASE_UTILITY_H
#define DATABASE_UTILITY_H

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>
#include "DBEngineLibraryExports.h"
#include "SqlErrorUtil.h"
#include "cJSON.h"


// Function declaration
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

int fetchInvoiceLineDataAsJson(SQLHSTMT* hstmtP, cJSON** rtt);

#endif // DATABASE_UTILITY_H