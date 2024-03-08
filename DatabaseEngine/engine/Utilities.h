#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <sql.h>
#include <sqlext.h>
#include "DBEngineLibraryExports.h"

#define ERROR_UTILITY -10
#define ERROR_REALLOC_FAILED -11


void save_to_file(
	const char*					content, 
	const char*					filename);

_Success_(return == 0)
DB_ENGINE_LIBRARY_EXPORT int getWorkingDir(
	_Out_   char**				pWorkingDir);

DB_ENGINE_LIBRARY_EXPORT int getConnectionString(
	_Inout_ char**				workingDirectory,
	_In_	char*				fileName,
	_Out_	char**				connectionString);

DB_ENGINE_LIBRARY_EXPORT int freeGlobalVariable(
	_In_	int					selector);

int concatToJsonData(
	_Inout_ char**				dest, 
	_In_	const char*			source);

int readFile(
	_In_	char**				workingDirectory,
	_Out_	char**				connectionString);

int parseCustomerData(
	_In_ int					customer_id,
	_In_ char*					first_name,
	_In_ char*					last_name,
	_In_ char*					address,
	_In_ char*					zip,
	_In_ char*					city,
	_Inout_ char**				dest);

int parseInvoiceLineData(
	_In_ int					invoice_id, 
	_In_ char*					product_reference,
	_Inout_ char**				dest);

void insert_string_safely(
	_Inout_ char**				dest, 
	_In_ const char*			src, 
	_In_ int					pos);

int find_index_of_invoices_opening_bracket(
	_In_ const char*			json);

int find_index_of_invoices_closing_bracketXXX(
	_In_ const char*			json);

int find_index_of_invoice_lines_opening_bracket(
	_In_ const char*			json);

int find_latest_index_of_invoice_lines_opening_bracket(const char* json);

int find_index_of_invoices_closing_bracket(const char* json, int start_index);

int split_string(int index, char** src, char** start, char** end);

int stringToTimestamp(
	const char*					inputString, 
	SQL_TIMESTAMP_STRUCT*		timestamp);

// Function declaration
#ifdef __cplusplus
extern "C" {
#endif

	DB_ENGINE_LIBRARY_EXPORT char* convertTimestampToString(const SQL_TIMESTAMP_STRUCT* timestamp);

#ifdef __cplusplus
}
#endif

#endif // UTILITIES_H