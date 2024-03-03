#include "SqlErrorUtil.h"


/*
*
*/
void SQLErrorUtil(_In_ SQLRETURN retcode, _In_ SQLHSTMT hstmt, _Out_ node_t** listItems)
{
    *listItems = NULL;

    // The maximum length of sqlstate is 5 characters1.This length does not include the null - termination character, 
    // so you should actually allocate 6 characters to hold the SQLSTATE code and the null - termination character
    SQLCHAR			sqlstate[6];
    SQLINTEGER		native_error;
    SQLCHAR			message[SQL_MAX_MESSAGE_LENGTH];
    SQLSMALLINT		message_len;

    memset(sqlstate, 0, sizeof(sqlstate));
    memset(message, 0, sizeof(message));

    // Execute stored procedure
    // retcode = SQLExecDirect(hstmt, (SQLCHAR*)call, SQL_NTS);

    if (retcode == SQL_ERROR) 
    {
        while (SQLError(SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, sqlstate, &native_error, message, sizeof(message), &message_len) != SQL_NO_DATA)
        {
            printf("SQLSTATE: %s\nNative Error: %ld\nMessage: %s\n", sqlstate, (long)native_error, message);
            SQLERRORDETAILS* err_details = (SQLERRORDETAILS*)malloc(2048);

            if (err_details)
            {
                memset(err_details->sqlstate, 0, sizeof(err_details->sqlstate));
                memset(err_details->message, 0, sizeof(err_details->message));

                strcpy_s(err_details->sqlstate, 6, sqlstate);
            err_details->native_error = native_error;
            err_details->message_len = message_len;
            strcpy_s(err_details->message, SQL_MAX_MESSAGE_LENGTH, message);

            Append(listItems, err_details);
        }
    }
}
}