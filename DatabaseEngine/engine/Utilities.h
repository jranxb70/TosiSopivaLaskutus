#ifndef UTILITIES_H
#define UTILITIES_H

#include <Windows.h>

#define ERROR_UTILITY -10

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

#endif // UTILITIES_H