#include "SqlErrorUtil.h"


/*
*
*/
void SQLErrorUtil(_In_ SQLRETURN retcode, _In_ SQLHSTMT hstmt, _Out_ node_t** listItems)
{
    SQLCHAR			sqlstate[6];
    SQLINTEGER		native_error;
    SQLCHAR			message[SQL_MAX_MESSAGE_LENGTH];
    SQLSMALLINT		message_len;

    // Execute stored procedure
    // retcode = SQLExecDirect(hstmt, (SQLCHAR*)call, SQL_NTS);

    if (retcode == SQL_ERROR) 
    {
        while (SQLError(SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, sqlstate, &native_error, message, sizeof(message), &message_len) != SQL_NO_DATA) {
            printf("SQLSTATE: %s\nNative Error: %ld\nMessage: %s\n", sqlstate, (long)native_error, message);
            //SQLERRORDETAILS* err_details = (SQLERRORDETAILS*) malloc(2048);
            //strcpy_s(err_details->sqlstate, 1024, sqlstate);
            //err_details->native_error = native_error;
            //err_details->message_len = message_len;
            //strcpy_s(err_details->message, SQL_MAX_MESSAGE_LENGTH, message);

            //Append(listItems, err_details);
        }

        for (int i = 0; i < 5; i++)
        {
            SQLERRORDETAILS* err_details = (SQLERRORDETAILS*)malloc(2048);
            strcpy_s(err_details->sqlstate, 1024, sqlstate);
            err_details->native_error = native_error;
            err_details->message_len = message_len;
            strcpy_s(err_details->message, SQL_MAX_MESSAGE_LENGTH, message);

            Append(listItems, err_details);
        }
    }
}