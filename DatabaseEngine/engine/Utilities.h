#ifndef UTILITIES_H
#define UTILITIES_H

#include <Windows.h>

#define ERROR_UTILITY -10
#define ERROR_REALLOC_FAILED -11

_Success_(return == 0)
int getWorkingDir(
	_Out_ char** pWorkingDir);

int getConnectionString(
	_Inout_ char** workingDirectory,
	_In_ char* fileName,
	_Out_ char** connectionString);

int concatToJsonData(
	_Inout_ char** dest, 
	_In_ const char* source);

int readFile(
	_In_ char** workingDirectory,
	_Out_ char** connectionString);

int getInvoiceData(
	_In_ int					customer_id,
	_In_ int					invoice_id,
	_In_ char* bank_reference,
	_Inout_						char** dest);

#endif // UTILITIES_H