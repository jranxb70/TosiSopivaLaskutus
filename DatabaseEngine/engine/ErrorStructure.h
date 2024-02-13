#ifndef ERROR_STRUCTURE_H
#define ERROR_STRUCTURE_H

#include <Windows.h>
#include <sql.h>
#include <sqlext.h>

typedef struct SQLErrorDetails
{
	SQLCHAR			sqlstate[6];
	SQLINTEGER		native_error;
	SQLCHAR			message[SQL_MAX_MESSAGE_LENGTH];
	SQLSMALLINT		message_len;
} SQLERRORDETAILS;

#endif // ERROR_STRUCTURE_H