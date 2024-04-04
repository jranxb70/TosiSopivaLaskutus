#include <time.h>
#include "DatabaseEngine.h"
#include "Utilities.h"


#define SQL_RESULT_LEN                  240
#define SQL_RETURN_CODE_LEN             1000

#define LEN_COMPANY_NAME                31
#define LEN_FIRST_NAME                  51
#define LEN_LAST_NAME                   51
#define LEN_ADDRESS                     101
#define LEN_ZIP                         6
#define LEN_CITY                        51 
#define LEN_PHONE                       21
#define LEN_EMAIL                       101
#define LEN_BANK_REF                    21
#define LEN_DATE                        31
#define LEN_DUE_DATE                    21
#define LEN_DESCRIPTION                 1025
#define LEN_USER_NAME                   51
#define LEN_PASSWORD                    51

#define INVALID_SWITCH_VALUE            -101
#define INVALID_DATE_VALUE              -102
#define INVALID_SORTING_VALUE           -103

SQLHENV henv;  // Environment handle
SQLHDBC hdbc;  // Connection handle

SQLCHAR result[SQL_RESULT_LEN];
SQLCHAR retcode[SQL_RETURN_CODE_LEN];

/**
* This data is global due to another function must be able to free the memory.
*/
char* global_json_data = NULL;

/*char* g_customer_json_data = NULL;
char* g_invoice_json_data = NULL;
char* g_company_json_data = NULL;
char* g_invoices_by_customer_json_data = NULL;*/


/**
* This function opens the connection to a database using an ODBC driver.
* 
* @param fileName: A name of the file that must contain the connection string
* @param dbErr: Holds information for what phase went wrong and with what err code.
*/
void dbOpen(
    _In_ char* fileName,
    _Out_ DBERROR** dbErr)
{
    char* workingDirectory = NULL;
    char* connectionStringW = NULL;

    *dbErr = NULL;

    int err = 0;

    *dbErr = (DBERROR*)malloc(sizeof(DBERROR));
    if ((*dbErr) == NULL)
    {
        printf("Memory allocating error.");
        return;
    }

    (*dbErr)->errorCode = 0;
    (*dbErr)->errorInt = 0;
    (*dbErr)->failedFunction = ErrFuncNone;

    // Allocate an environment handle
    SQLRETURN retHandle = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (retHandle != SQL_SUCCESS && retHandle != SQL_SUCCESS_WITH_INFO) {
        fprintf(stderr, "Error allocating environment handle\n");
        SQLRETURN retHandle = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
        (*dbErr)->errorCode = retHandle;
        (*dbErr)->failedFunction = ErrFuncSQLAllocHandleA;
        goto exit;
    }

    // Set the ODBC version
    SQLRETURN retEnvAttr = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (retEnvAttr != SQL_SUCCESS && retEnvAttr != SQL_SUCCESS_WITH_INFO) {
        fprintf(stderr, "Error setting ODBC version\n");
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        (*dbErr)->errorCode = retEnvAttr;
        (*dbErr)->failedFunction = ErrFuncSQLSetEnvAttrA;
        goto exit;
    }

    //// Allocate a connection handle
    SQLRETURN retConHandle = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (retConHandle != SQL_SUCCESS && retConHandle != SQL_SUCCESS_WITH_INFO) {
        fprintf(stderr, "Error allocating connection handle\n");
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        (*dbErr)->errorCode = retConHandle;
        (*dbErr)->failedFunction = ErrFuncSQLAllocHandleB;
        goto exit;
    }

    if (-1 == (err = getWorkingDir(&workingDirectory)))
    {
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        (*dbErr)->errorInt = ERROR_WORKING_DIRECTORY_ERROR;
        (*dbErr)->failedFunction = ErrFunc_getWorkingDirA;
        goto exit;
    }

    if (-1 == (err = getConnectionString(&workingDirectory, fileName, &connectionStringW)))
    {
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        (*dbErr)->errorInt = ERROR_CONNECTION_STRING_UNAVAILABLE;
        (*dbErr)->failedFunction = ErrFunc_getConnectionStringA;
        goto exit;
    }

    if (!!connectionStringW)
    {
        //Connect to the database
        SQLRETURN retConnect = SQLDriverConnect(hdbc, NULL, (SQLCHAR*)connectionStringW, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
        if (retConnect != SQL_SUCCESS && retConnect != SQL_SUCCESS_WITH_INFO) {
            fprintf(stderr, "Error connecting to SQL Server\n");
            SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
            SQLFreeHandle(SQL_HANDLE_ENV, henv);
            (*dbErr)->errorCode = retConnect;
            (*dbErr)->failedFunction = ErrFuncSQLDriverConnectA;
            goto exit;
        }
    }
    else
    {
        printf("ERROR: The connection string is unavaible.");
    }

exit:
    free(connectionStringW);
    free(workingDirectory);
}

/**
* This function closes the ODBC connection to the Sql Server database.
*/
void dbClose()
{
    // Disconnect and free handles
    SQLRETURN retValDisconnect = SQLDisconnect(hdbc);
    if (retValDisconnect != SQL_SUCCESS && retValDisconnect != SQL_SUCCESS_WITH_INFO) {
        fprintf(stderr, "Error freeing environment handle\n");
        retValDisconnect = ERROR_CODE;
    }
    SQLRETURN retValHDBC = SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    if (retValHDBC != SQL_SUCCESS && retValHDBC != SQL_SUCCESS_WITH_INFO) {
        fprintf(stderr, "Error freeing environment handle\n");
        retValHDBC = ERROR_CODE;
    }
    // Free the environment handle
    SQLRETURN retVal = SQLFreeHandle(SQL_HANDLE_ENV, henv);
    if (retVal != SQL_SUCCESS && retVal != SQL_SUCCESS_WITH_INFO) {
        fprintf(stderr, "Error freeing environment handle\n");
        retVal = ERROR_CODE;
    }
}

/**
* This function ...
*/
int free_json_data(/*int selector*/)
{
    int done = 744;

    //if (selector == 1)
    //{
        free(global_json_data);
        global_json_data = NULL;
        done = 11;
   // }
    /*else if (selector == 2)
    {
        free(g_customer_json_data);
        g_customer_json_data = NULL;
        done = 12;
    }
    else if (selector == 3)
    {
        free(g_invoice_json_data);
        g_invoice_json_data = NULL;
        done = 13;
    }
    else if (selector == 4)
    {
        free(g_company_json_data);
        g_company_json_data = NULL;
        done = 14;
    }
    else if (selector == 5)
    {
        free(g_invoices_by_customer_json_data);
        g_invoices_by_customer_json_data = NULL;
        done = 15;
    }*/
    //else
    //{
    //    done = -123;
    //}

    return done;
}

void free_sql_error_details()
{
    DeleteList(&internalErrorList);
    internalErrorList = NULL;
}

/**
 * return -1, if function fails, 0 if no record is affected, 1, if a record is deleted.
*/
int deleteCustomer(_In_ long customer_id)
{
    char fileName[21] = "connectionstring.txt";
    DBERROR* err = NULL;
    dbOpen(fileName, &err);

    if (err->errorCode < 0)
    {
        free(err);
        return -1;
    }
    int ret = err->errorCode;
    free(err);
    err = NULL;

    int countDeleted = -1;

    char query[1024];
    size_t bufferCount = 1024;
    sprintf_s(query, bufferCount, "EXEC dbo.DeleteCustomer ?, ?");

    SQLHSTMT hstmt;
    SQLINTEGER id = customer_id;
    SQLINTEGER result = -3;

    SQLCHAR sqlstate[6];
    SQLINTEGER native_error;
    SQLCHAR message_text[SQL_MAX_MESSAGE_LENGTH];
    SQLSMALLINT text_length;

    if (SQL_SUCCEEDED(ret)) 
    {
        // Allocate a statement handle
        SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

        // Prepare the SQL statement
        ret = SQLPrepare(hstmt, query, SQL_NTS);

        ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &id,     0, NULL);
        ret = SQLBindParameter(hstmt, 2, SQL_PARAM_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &result, 0, NULL);


        if (SQL_SUCCEEDED(ret))
        {
            // Execute the query
            ret = SQLExecute(hstmt);

            if (SQL_SUCCEEDED(ret))
            {
                printf("Status: %d\n", result);

                countDeleted = result;
            }
            else
            {
                SQLRETURN retDiagRec = SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate, &native_error, message_text, sizeof(message_text), &text_length);
                printf("Error executing the stored procedure: %s\n", message_text);
            }
        }
        else
        {
            SQLRETURN retDiagRec = SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate, &native_error, message_text, sizeof(message_text), &text_length);
            printf("Error connecting to database: %s\n", message_text);
        }

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    }
    else 
    {
        // Get the return code
        SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate, &native_error, message_text, sizeof(message_text), &text_length);
        printf("Error connecting to database: %s\n", message_text);
    }

    dbClose();

    return countDeleted;
}

int addDBUser(
    _In_ char* login, 
    _In_ char* password, 
    _In_ char* email)
{
    char fileName[21] = "connectionstring.txt";
    DBERROR* err = NULL;
    dbOpen(fileName, &err);

    if (err->errorCode < 0)
    {
        free(err);
        return -2;
    }
    int ret = err->errorCode;
    free(err);
    err = NULL;

    char* login_decoded = NULL;
    decodeUTF8Encoding(login, &login_decoded);

    char* password_decoded = NULL;
    decodeUTF8Encoding(password, &password_decoded);

    char* email_decoded = NULL;
    decodeUTF8Encoding(email, &email_decoded);

    char query[1024];
    size_t bufferCount = 1024;
    sprintf_s(query, bufferCount, "{CALL dbo.AddDBUser (?, ?, ?)}");

    SQLCHAR sqlstate[6];
    SQLINTEGER native_error;
    SQLCHAR message_text[SQL_MAX_MESSAGE_LENGTH];
    SQLSMALLINT text_length;

    SQLHSTMT hstmt;

    if (SQL_SUCCEEDED(ret)) {
        // Allocate a statement handle
        SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, login_decoded, 0, NULL);
        SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 100, 0, password_decoded, 0, NULL);
        SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, email_decoded, 0, NULL);

        // Prepare the SQL statement
        ret = SQLPrepare(hstmt, query, SQL_NTS);
        if (SQL_SUCCEEDED(ret))
        {
            // Execute the query
            ret = SQLExecute(hstmt);

            if (SQL_SUCCEEDED(ret))
            {
                if (SQLMoreResults(hstmt) == SQL_NO_DATA)
                {
                    printf("SQl statement executed successfully");
                    ret = 0;
                }
            }
            else
            {
                SQLRETURN retussi = SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate, &native_error, message_text, sizeof(message_text), &text_length);

                if (native_error == 2627)
                {
                    ret = -1; // record is a duplicate
                }
                printf("(Warning) Function addDBUser: %s\n", message_text);
            }
            // Free the statement handle
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        }
    }
    else 
    {
        // Get the return code
        SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate, &native_error, message_text, sizeof(message_text), &text_length);
        printf("Error opening the connecting to database: %s\n", message_text);
    }

    free(login_decoded);
    free(password_decoded);
    free(email_decoded);

    dbClose();
    return ret;
}

int getDBUser(_In_ char* login, _In_ char* user_password)
{
    char fileName[21] = "connectionstring.txt";
    DBERROR* err = NULL;
    dbOpen(fileName, &err);

    if (err->errorCode < 0)
    {
        free(err);
        return -2;
    }
    int ret = err->errorCode;
    free(err);
    err = NULL;

    char* login_converted = NULL;
    decodeUTF8Encoding(login, &login_converted);

    char* password_converted = NULL;
    decodeUTF8Encoding(user_password, &password_converted);

    int grant_access = -3;

    char query[1024];
    size_t bufferCount = 1024;
    sprintf_s(query, bufferCount, "{CALL dbo.GetDBUserRow (?)}");

    SQLHSTMT hstmt;

    SQLCHAR sqlstate[6];
    SQLINTEGER native_error;
    SQLCHAR message_text[SQL_MAX_MESSAGE_LENGTH];
    SQLSMALLINT text_length;

    if (SQL_SUCCEEDED(ret)) {
        // Allocate a statement handle
        SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, login_converted, 0, NULL);

        // Prepare the SQL statement
        ret = SQLPrepare(hstmt, query, SQL_NTS);
        if (SQL_SUCCEEDED(ret))
        {
            // Execute the query
            ret = SQLExecute(hstmt);

            if (SQL_SUCCEEDED(ret))
            {
                int recordsFound = 0;
                SQLCHAR     login[LEN_USER_NAME];
                SQLCHAR     email[LEN_EMAIL];
                SQLCHAR     password[LEN_PASSWORD];


                while (SQLFetch(hstmt) == SQL_SUCCESS)
                {
                    recordsFound = 1;
                    SQLGetData(hstmt, 1, SQL_C_CHAR, login, sizeof(login), NULL);
                    SQLGetData(hstmt, 2, SQL_C_CHAR, email, sizeof(email), NULL);
                    SQLGetData(hstmt, 3, SQL_C_CHAR, password, sizeof(password), NULL);
                }

                if (recordsFound == 1)
                {
                    if (strcmp((char*)password, user_password) == 0)
                    {
                        grant_access = 1;
                    }
                    else
                    {
                        grant_access = 0;
                    }
                }
                else
                {
                    grant_access = 0;
                }
            }
            else
            {
                SQLRETURN retussi = SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate, &native_error, message_text, sizeof(message_text), &text_length);
                printf("Error executing the stored procedure: %s\n", message_text);
            }
        }
        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    }
    else {
        // Get the return code
        SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, 1, NULL, NULL, retcode, SQL_RETURN_CODE_LEN, NULL);
        printf("Error connecting to database: %s\n", retcode);
    }

    free(login_converted);
    free(password_converted);

    dbClose();
    return grant_access;
}


void updateCustomer(
    _In_ int customer_id, 
    _In_ char* customer_firstName, 
    _In_ char* customer_lastName,
    _In_ char* customer_address,
    _In_ char* customer_zip,
    _In_ char* customer_city,
    _In_ char* customer_phone,
    _In_ char* customer_email)
{
    char fileName[21] = "connectionstring.txt";
    DBERROR* err = NULL;
    dbOpen(fileName, &err);

    if (err->errorCode < 0)
    {
        free(err);
        return;
    }
    int ret = err->errorCode;
    free(err);
    err = NULL;

    char* customer_firstName_converted = NULL;
    decodeUTF8Encoding(customer_firstName, &customer_firstName_converted);

    char* customer_lastName_converted = NULL;
    decodeUTF8Encoding(customer_lastName, &customer_lastName_converted);

    char* customer_address_converted = NULL;
    decodeUTF8Encoding(customer_address, &customer_address_converted);

    char* customer_city_converted = NULL;
    decodeUTF8Encoding(customer_city, &customer_city_converted);
    
    char query[1024];
    size_t bufferCount = 1024;
    sprintf_s(query, bufferCount,
        "{CALL dbo.UpdateCustomer (?, ?, ?, ?, ?, ?, ?, ?)}");

    SQLHSTMT hstmt;
    SQLINTEGER id = customer_id;

    if (SQL_SUCCEEDED(ret)) {
        // Allocate a statement handle
        SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0,   0, &id,                             0, NULL);
        SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT,  SQL_C_CHAR,  SQL_VARCHAR, 50,  0, customer_firstName_converted,    0, NULL);
        SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT,  SQL_C_CHAR,  SQL_VARCHAR, 50,  0, customer_lastName_converted,     0, NULL);
        SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT,  SQL_C_CHAR,  SQL_VARCHAR, 100, 0, customer_address_converted,      0, NULL);
        SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT,  SQL_C_CHAR,  SQL_VARCHAR, 6,   0, customer_zip,                    0, NULL);
        SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT,  SQL_C_CHAR,  SQL_VARCHAR, 50,  0, customer_city_converted,         0, NULL);

        SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT,  SQL_C_CHAR,  SQL_VARCHAR, 20,  0, customer_phone,                  0, NULL);
        SQLBindParameter(hstmt, 8, SQL_PARAM_INPUT,  SQL_C_CHAR,  SQL_VARCHAR, 100, 0, customer_email,                  0, NULL);

        // Prepare the SQL statement
        ret = SQLPrepare(hstmt, query, SQL_NTS);
        if (SQL_SUCCEEDED(ret))
        {
            // Execute the query
            ret = SQLExecute(hstmt);

            if (SQL_SUCCEEDED(ret))
            {
                if (SQLMoreResults(hstmt) == SQL_NO_DATA)
                {
                    printf("SQL operations ready");
                }
            }
            // Free the statement handle
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        }
    }
    else {
        // Get the return code
        SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, 1, NULL, NULL, retcode, SQL_RETURN_CODE_LEN, NULL);
        printf("Error connecting to database: %s\n", retcode);
    }

    free(customer_lastName_converted);
    free(customer_firstName_converted);
    free(customer_address_converted);
    free(customer_city_converted);

    dbClose();
}

void queryCustomers(_Out_ char** jsonString, _Out_ node_t** errorList)
{
    char fileName[21] = "connectionstring.txt";
    DBERROR* err = NULL;
    dbOpen(fileName, &err);

    if (err->errorCode < 0)
    {
        free(err);
        return;
    }
    free(err);
    err = NULL;

    SQLHSTMT hstmt;
    SQLRETURN retcode;

    // Allocate statement handle
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    // Prepare stored procedure call
    char call[256];
    sprintf(call, "{call GetCustomers}");

    // Execute stored procedure
    retcode = SQLExecDirect(hstmt, (SQLCHAR*)call, SQL_NTS);

    SQLErrorUtil(retcode, hstmt, &internalErrorList);

    *errorList = internalErrorList;

    cJSON* root = cJSON_CreateObject();

    cJSON_AddArrayToObject(root, "customers");

    // Process the invoice data
    SQLINTEGER  customerId;
    SQLCHAR     firstName[LEN_FIRST_NAME];
    SQLCHAR     lastName[LEN_LAST_NAME];
    SQLCHAR     address[LEN_ADDRESS];
    SQLCHAR     zip[LEN_ZIP];
    SQLCHAR     city[LEN_CITY];
    SQLCHAR     phone[LEN_PHONE];
    SQLCHAR     email[LEN_EMAIL];

    while (SQLFetch(hstmt) == SQL_SUCCESS)
    {
        SQLGetData(hstmt, 1, SQL_C_SLONG, &customerId, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_CHAR, firstName, sizeof(firstName), NULL);
        SQLGetData(hstmt, 3, SQL_C_CHAR, lastName, sizeof(lastName), NULL);
        SQLGetData(hstmt, 4, SQL_C_CHAR, address, sizeof(address), NULL);
        SQLGetData(hstmt, 5, SQL_C_CHAR, zip, sizeof(zip), NULL);
        SQLGetData(hstmt, 6, SQL_C_CHAR, city, sizeof(city), NULL);

        SQLLEN phoneLen, emailLen;

        SQLGetData(hstmt, 7, SQL_C_CHAR, phone, sizeof(phone), &phoneLen);
        SQLGetData(hstmt, 8, SQL_C_CHAR, email, sizeof(email), &emailLen);

        cJSON* customer = cJSON_CreateObject();

        cJSON_AddNumberToObject(customer, "customer_id", customerId);
        cJSON_AddStringToObject(customer, "first_name",  firstName);
        cJSON_AddStringToObject(customer, "last_name",   lastName);
        cJSON_AddStringToObject(customer, "address",     address);
        cJSON_AddStringToObject(customer, "zip",         zip);
        cJSON_AddStringToObject(customer, "city",        city);

        // null email null phone situation

        if (phoneLen == SQL_NULL_DATA) 
        {
            cJSON_AddStringToObject(customer, "phone", "N/A");
        }
        else
        {
            cJSON_AddStringToObject(customer, "phone", phone);
        }
        if (emailLen == SQL_NULL_DATA)
        {
            cJSON_AddStringToObject(customer, "email", "N/A");
        }
        else
        {
            cJSON_AddStringToObject(customer, "email", email);
        }

        cJSON* customer_array = NULL;
        customer_array = cJSON_GetObjectItem(root, "customers");
        cJSON_AddItemReferenceToArray(customer_array, customer);
    }

    *jsonString = cJSON_Print(root);
    global_json_data = *jsonString;

    cJSON_Delete(root);

    // Free the statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    dbClose();
}


void queryInvoiceById(_In_ int invoice_id, _Out_ char** jsonString, _Out_ node_t** errorList)
{
    char fileName[21] = "connectionstring.txt";
    DBERROR* err = NULL;
    dbOpen(fileName, &err);

    if (err->errorCode < 0)
    {
        free(err);
        return;
    }
    free(err);
    err = NULL;

    SQLHSTMT hstmt;
    SQLRETURN retcode;

    // Allocate statement handle
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    // Prepare stored procedure call
    char call[256];
    sprintf(call, "{call GetInvoiceData2(?)}");

    // Bind the parameter
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &invoice_id, 0, NULL);

    // Execute stored procedure
    retcode = SQLExecDirect(hstmt, (SQLCHAR*)call, SQL_NTS);

    SQLErrorUtil(retcode, hstmt, &internalErrorList);

    *errorList = internalErrorList;

    cJSON* root = cJSON_CreateObject();
    cJSON* invoice = NULL;

    // Process the rows and print to screen
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
    {
        do {
            SQLCHAR columnName[64];
            SQLSMALLINT columnNameLength;

            SQLRETURN ret = SQLDescribeCol(hstmt, 1, columnName, sizeof(columnName), &columnNameLength, NULL, NULL, NULL, NULL);

            //cJSON* invoice_lines = NULL;

            if (strcmp((char*)columnName, "customer_id") == 0)
            {
                // Process the invoice data
                SQLINTEGER  customerId;
                SQLCHAR     firstName[LEN_FIRST_NAME];
                SQLCHAR     lastName[LEN_LAST_NAME];
                SQLCHAR     address[LEN_ADDRESS];
                SQLCHAR     zip[LEN_ZIP];
                SQLCHAR     city[LEN_CITY];
                SQLCHAR     phone[LEN_PHONE];
                SQLCHAR     email[LEN_EMAIL];

                SQLINTEGER  invoiceId;
                SQLCHAR     date[LEN_DATE];
                SQLCHAR     invoice_bank_reference[LEN_BANK_REF];
                SQLDOUBLE   invoice_subtotal;
                SQLDOUBLE   invoice_tax;
                SQLDOUBLE   invoice_total;

                SQLCHAR     invoice_due_date[LEN_DUE_DATE];

                while (SQLFetch(hstmt) == SQL_SUCCESS)
                {
                    SQLGetData(hstmt, 1,  SQL_C_SLONG,  &customerId,            0,                              NULL);
                    SQLGetData(hstmt, 2,  SQL_C_CHAR,   firstName,              sizeof(firstName),              NULL);
                    SQLGetData(hstmt, 3,  SQL_C_CHAR,   lastName,               sizeof(lastName),               NULL);
                    SQLGetData(hstmt, 4,  SQL_C_CHAR,   address,                sizeof(address),                NULL);
                    SQLGetData(hstmt, 5,  SQL_C_CHAR,   zip,                    sizeof(zip),                    NULL);
                    SQLGetData(hstmt, 6,  SQL_C_CHAR,   city,                   sizeof(city),                   NULL);

                    SQLLEN phoneLen, emailLen;

                    SQLGetData(hstmt, 7,  SQL_C_CHAR,   phone,                  sizeof(phone),                  &phoneLen);
                    SQLGetData(hstmt, 8,  SQL_C_CHAR,   email,                  sizeof(email),                  &emailLen);

                    SQLGetData(hstmt, 9,  SQL_C_SLONG,  &invoiceId,             0,                              NULL);
                    SQLGetData(hstmt, 10, SQL_C_CHAR,   date,                   sizeof(date),                   NULL);

                    SQLGetData(hstmt, 11, SQL_C_CHAR,   invoice_bank_reference, sizeof(invoice_bank_reference), NULL);

                    SQLGetData(hstmt, 12, SQL_C_DOUBLE, &invoice_subtotal,      0,                              NULL);
                    SQLGetData(hstmt, 13, SQL_C_DOUBLE, &invoice_tax,           0,                              NULL);
                    SQLGetData(hstmt, 14, SQL_C_DOUBLE, &invoice_total,         0,                              NULL);

                    SQLLEN invoice_due_dateLen;

                    SQLGetData(hstmt, 15, SQL_C_CHAR,   invoice_due_date,       sizeof(invoice_due_date),       &invoice_due_dateLen);

                    if (phoneLen == SQL_NULL_DATA)
                    {
                        strcpy_s(phone, sizeof(phone), "N/A");
                    }

                    if (emailLen == SQL_NULL_DATA)
                    {
                        strcpy_s(email, sizeof(email), "N/A");
                    }

                    printf("Customer ID: %d, First name: %s, Last name: %s, Address: %s, Address: %s, City: %s Phone: %s, Email: %s\n",
                        customerId,
                        firstName,
                        lastName,
                        address,
                        zip,
                        city,
                        phone,
                        email);

                    cJSON_AddNumberToObject(root, "customer_id", customerId);
                    cJSON_AddStringToObject(root, "first_name",  firstName);
                    cJSON_AddStringToObject(root, "last_name",   lastName);
                    cJSON_AddStringToObject(root, "address",     address);
                    cJSON_AddStringToObject(root, "zip",         zip);
                    cJSON_AddStringToObject(root, "city",        city);


                    // null email null phone situation

                    if (phoneLen == SQL_NULL_DATA)
                    {
                        cJSON_AddStringToObject(root, "phone", "N/A");
                    }
                    else
                    {
                        cJSON_AddStringToObject(root, "phone", phone);
                    }
                    if (emailLen == SQL_NULL_DATA)
                    {
                        cJSON_AddStringToObject(root, "email", "N/A");
                    }
                    else
                    { 
                        cJSON_AddStringToObject(root, "email", email);
                    }

                    cJSON_AddNumberToObject(root, "invoice_id", invoiceId);
                    cJSON_AddStringToObject(root, "invoice_date", date);

                    cJSON_AddStringToObject(root, "invoice_bank_reference", invoice_bank_reference);

                    cJSON_AddNumberToObject(root, "invoice_subtotal", invoice_subtotal);
                    cJSON_AddNumberToObject(root, "invoice_tax", invoice_tax);
                    cJSON_AddNumberToObject(root, "invoice_total", invoice_total);

                    if (invoice_due_dateLen == SQL_NULL_DATA)
                    {
                        cJSON_AddStringToObject(root, "invoice_due_date", "N/A");
                    }
                    else
                    {
                        cJSON_AddStringToObject(root, "invoice_due_date", invoice_due_date);
                    }

                    cJSON_AddArrayToObject(root, "invoice_lines");
                }
            }
            else if (strcmp((char*)columnName, "InvoiceLineId") == 0)
            {
                fetchInvoiceLineDataAsJson(&hstmt, &root);
            }
        } while (SQLMoreResults(hstmt) == SQL_SUCCESS);
    }

    char* archieCruzPlusIida = cJSON_Print(root);

    printf("%s", archieCruzPlusIida);

    *jsonString = archieCruzPlusIida;
    /*g_invoice_json_data*/ global_json_data = *jsonString;

    cJSON_Delete(root);

    // Free the statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    dbClose();
}

void getCompany(_In_ int company_id, _Out_ char** jsonStringCompany)
{
    char fileName[21] = "connectionstring.txt";
    DBERROR* err = NULL;
    dbOpen(fileName, &err);

    if (err->errorCode < 0)
    {
        free(err);
        return;
    }
    free(err);
    err = NULL;

    SQLHSTMT hstmt;
    SQLRETURN retcode;

    SQLINTEGER sql_company_id = company_id;

    // Allocate statement handle
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    // Prepare stored procedure call
    char call[256];
    sprintf(call, "{call GetCompany(?)}");

    // Bind the parameter
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &sql_company_id, 0, NULL);

    // Execute stored procedure
    retcode = SQLExecDirect(hstmt, (SQLCHAR*)call, SQL_NTS);

    cJSON* root = cJSON_CreateObject();
    cJSON* company_name = NULL;

    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
    {
        do 
        {
            SQLINTEGER  companyId;
            SQLCHAR     companyName[LEN_COMPANY_NAME];

            SQLCHAR     address[LEN_ADDRESS];
            SQLCHAR     zip[LEN_ZIP];
            SQLCHAR     city[LEN_CITY];
            SQLCHAR     phone[LEN_PHONE];
            SQLCHAR     businessId[LEN_EMAIL];

            while (SQLFetch(hstmt) == SQL_SUCCESS)
            {
                SQLGetData(hstmt, 1, SQL_C_SLONG, &companyId, 0, NULL);
                SQLGetData(hstmt, 2, SQL_C_CHAR, companyName, sizeof(companyName), NULL);
                SQLGetData(hstmt, 3, SQL_C_CHAR, address, sizeof(address), NULL);
                SQLGetData(hstmt, 4, SQL_C_CHAR, zip, sizeof(zip), NULL);
                SQLGetData(hstmt, 5, SQL_C_CHAR, city, sizeof(city), NULL);
                SQLGetData(hstmt, 6, SQL_C_CHAR, phone, sizeof(phone), NULL);
                SQLGetData(hstmt, 7, SQL_C_CHAR, businessId, sizeof(businessId), NULL);

                //char* decodedCompanyName = NULL;
                //decodeUTF8Encoding(companyName, &decodedCompanyName);
                //printf("decodedCharArray: %s", decodedCompanyName);

                cJSON_AddNumberToObject(root, "company_id", companyId);
                company_name = cJSON_AddStringToObject(root, "company_name", companyName);
                cJSON_AddStringToObject(root, "company_address", address);
                cJSON_AddStringToObject(root, "company_zip", zip);
                cJSON_AddStringToObject(root, "company_city", city);
                cJSON_AddStringToObject(root, "company_phone", phone);
                cJSON_AddStringToObject(root, "company_business_id", businessId);

                //free(decodedCompanyName);
            }
        } while (SQLMoreResults(hstmt) == SQL_SUCCESS);
    }
    char* archieCruzPlusIida = cJSON_Print(root);

    printf("%s", archieCruzPlusIida);

    *jsonStringCompany = archieCruzPlusIida;
    /*g_company_json_data*/ global_json_data = *jsonStringCompany;

    cJSON_Delete(root);

    // Free the statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    dbClose();
    return;
}

/**
* This function ...
*
* @param customer_id: ...
*/
void queryInvoicesByCustomer(_In_ int customer_id, _Out_ char** jsonString, _Out_ node_t** errorList)
{
    char fileName[21] = "connectionstring.txt";
    DBERROR* err = NULL;
    dbOpen(fileName, &err);

    if (err->errorCode < 0)
    {
        free(err);
        return;
    }
    free(err);
    err = NULL;

    SQLHSTMT hstmt;
    SQLRETURN retcode;

    // Allocate statement handle
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    // Prepare stored procedure call
    char call[256];
    sprintf(call, "{call GetCustomerInvoices(?)}");

    // Bind the parameter
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &customer_id, 0, NULL);

    // Execute stored procedure
    retcode = SQLExecDirect(hstmt, (SQLCHAR*)call, SQL_NTS);

    SQLErrorUtil(retcode, hstmt, &internalErrorList);

    *errorList = internalErrorList;

    cJSON* root = cJSON_CreateObject();
    cJSON* invoices = NULL;

    // Process the rows and print to screen
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) 
    {
        do {
            SQLCHAR columnName[64];
            SQLSMALLINT columnNameLength;

            SQLRETURN ret = SQLDescribeCol(hstmt, 1, columnName, sizeof(columnName), &columnNameLength, NULL, NULL, NULL, NULL);

            cJSON* invoice_lines = NULL;

            if (strcmp((char*)columnName, "customer_id") == 0) 
            {
                // Process the invoice data
                SQLINTEGER  customerId;
                SQLCHAR     firstName[LEN_FIRST_NAME];
                SQLCHAR     lastName[LEN_LAST_NAME];
                SQLCHAR     address[LEN_ADDRESS];
                SQLCHAR     zip[LEN_ZIP];
                SQLCHAR     city[LEN_CITY];
                SQLCHAR     phone[LEN_PHONE];
                SQLCHAR     email[LEN_EMAIL];

                while (SQLFetch(hstmt) == SQL_SUCCESS) 
                {
                    SQLGetData(hstmt, 1, SQL_C_SLONG,   &customerId,    0,                      NULL);
                    SQLGetData(hstmt, 2, SQL_C_CHAR,    firstName,      sizeof(firstName),      NULL);
                    SQLGetData(hstmt, 3, SQL_C_CHAR,    lastName,       sizeof(lastName),       NULL);
                    SQLGetData(hstmt, 4, SQL_C_CHAR,    address,        sizeof(address),        NULL);
                    SQLGetData(hstmt, 5, SQL_C_CHAR,    zip,            sizeof(zip),            NULL);
                    SQLGetData(hstmt, 6, SQL_C_CHAR,    city,           sizeof(city),           NULL);

                    SQLLEN phoneLen, emailLen;

                    SQLGetData(hstmt, 7, SQL_C_CHAR,    phone,          sizeof(phone),          &phoneLen);
                    SQLGetData(hstmt, 8, SQL_C_CHAR,    email,          sizeof(email),          &emailLen);

                    if (phoneLen == SQL_NULL_DATA)
                    {
                        strcpy_s(phone, sizeof(phone), "N/A");
                    }

                    if (emailLen == SQL_NULL_DATA)
                    {
                        strcpy_s(email, sizeof(email), "N/A");
                    }

                    printf("Customer ID: %d, First name: %s, Last name: %s, Address: %s, Address: %s, City: %s\n", 
                            customerId,
                            firstName, 
                            lastName, 
                            address,
                            zip,
                            city);

                    cJSON_AddNumberToObject(root, "customer_id", customerId);
                    cJSON_AddStringToObject(root, "first_name",  firstName);
                    cJSON_AddStringToObject(root, "last_name",   lastName);
                    cJSON_AddStringToObject(root, "address",     address);
                    cJSON_AddStringToObject(root, "zip",         zip);
                    cJSON_AddStringToObject(root, "city",        city);

                    // null email null phone situation

                    if (phoneLen == SQL_NULL_DATA)
                    {
                        cJSON_AddStringToObject(root, "phone", "N/A");
                    }
                    else
                    {
                        cJSON_AddStringToObject(root, "phone", phone);
                    }
                    if (emailLen == SQL_NULL_DATA)
                    {
                        cJSON_AddStringToObject(root, "email", "N/A");
                    }
                    else
                    {
                        cJSON_AddStringToObject(root, "email", email);
                    }

                    invoices = cJSON_AddArrayToObject(root, "invoices");
                }
            }
            else if (strcmp((char*)columnName, "invoice_id") == 0) 
            {
                // Process the invoice data
                SQLINTEGER invoiceId;
                SQLCHAR dateStr[LEN_DATE];
                SQLCHAR bankReference[LEN_BANK_REF];

                SQLDOUBLE invoice_subtotal;
                SQLDOUBLE invoice_tax;
                SQLDOUBLE invoice_total;

                SQLCHAR     invoice_due_date[LEN_DUE_DATE];

                while (SQLFetch(hstmt) == SQL_SUCCESS) 
                {
                    SQLGetData(hstmt, 1, SQL_C_SLONG, &invoiceId, 0, NULL);
                    SQLGetData(hstmt, 3, SQL_C_CHAR, dateStr, sizeof(dateStr), NULL); 
                    SQLGetData(hstmt, 4, SQL_C_CHAR, bankReference, sizeof(bankReference), NULL);

                    SQLGetData(hstmt, 5, SQL_C_DOUBLE, &invoice_subtotal, 0, NULL);
                    SQLGetData(hstmt, 6, SQL_C_DOUBLE, &invoice_tax, 0, NULL);
                    SQLGetData(hstmt, 7, SQL_C_DOUBLE, &invoice_total, 0, NULL);

                    SQLLEN invoice_due_date_len;

                    SQLGetData(hstmt, 8, SQL_C_CHAR, invoice_due_date, sizeof(invoice_due_date), &invoice_due_date_len);

                    printf("Invoice ID: %d, Bank Reference: %s\n", invoiceId, bankReference);

                    cJSON* invoice = cJSON_CreateObject();
                    cJSON_AddNumberToObject(invoice, "invoice_id", invoiceId);
                    cJSON_AddStringToObject(invoice, "invoice_date", dateStr);
                    cJSON_AddStringToObject(invoice, "bank_reference", bankReference);

                    cJSON_AddNumberToObject(invoice, "invoice_subtotal", invoice_subtotal);
                    cJSON_AddNumberToObject(invoice, "invoice_tax", invoice_tax);
                    cJSON_AddNumberToObject(invoice, "invoice_total", invoice_total);

                    if (invoice_due_date_len == SQL_NULL_DATA)
                    {

                        cJSON_AddStringToObject(invoice, "invoice_due_date", "N/A");
                    }
                    else
                    {
                        cJSON_AddStringToObject(invoice, "invoice_due_date", invoice_due_date);
                    }

                    invoice_lines = cJSON_AddArrayToObject(invoice, "invoice_lines");

                    cJSON_AddItemReferenceToArray(invoices, invoice);
                }
            }
            else if (strcmp((char*)columnName, "invoice_line_id") == 0)
            {
                fetchInvoiceLineDataAsJson(&hstmt, &root);
            }
        } while (SQLMoreResults(hstmt) == SQL_SUCCESS);
    }

    char* archieCruzPlusIida = cJSON_Print(root);

    printf("%s", archieCruzPlusIida);

    *jsonString = archieCruzPlusIida;
    /*g_invoices_by_customer_json_data*/ global_json_data = *jsonString;

    cJSON_Delete(root);

    // Free the statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    dbClose();
}

int queryInvoices(
    _In_ long	    procedure_switch,
    _In_ char*      start_date,
    _In_ char*      end_date,
    _In_ long	    sorting,
    _Out_ char**    jsonString)
{
    char fileName[21] = "connectionstring.txt";
    DBERROR* err = NULL;
    dbOpen(fileName, &err);

    if (err->errorCode < 0)
    {
        free(err);
        return -2;
    }
    free(err);
    err = NULL;

    if (procedure_switch != 1 && procedure_switch != 2 && procedure_switch != 3)
    {
        dbClose();
        return INVALID_SWITCH_VALUE;
    }

    if (procedure_switch == 3 && (start_date == NULL || end_date == NULL))
    {
        dbClose();
        return INVALID_DATE_VALUE;
    }

    if (procedure_switch == 3 && (sorting != -1 && sorting != 1))
    {
        dbClose();
        return INVALID_SORTING_VALUE;
    }

    SQLHSTMT hstmt;
    SQLRETURN retcode;

    // Allocate statement handle
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    // Prepare stored procedure call
    char call[256];
    sprintf(call, "{call SelectInvoices(?, ?, ?, ?)}");

    SQL_TIMESTAMP_STRUCT   sql_start_date;
    sql_start_date.year = 1970;
    sql_start_date.month = 1;
    sql_start_date.day = 1;
    sql_start_date.hour = 0;
    sql_start_date.minute = 0;
    sql_start_date.second = 0;
    sql_start_date.fraction = 0;

    SQL_TIMESTAMP_STRUCT   sql_end_date;
    sql_end_date.year = 1970;
    sql_end_date.month = 1;
    sql_end_date.day = 1;
    sql_end_date.hour = 0;
    sql_end_date.minute = 0;
    sql_end_date.second = 0;
    sql_end_date.fraction = 0;
    int resRet = -9;

    if (start_date != NULL)
    {
        resRet = stringToTimestamp(start_date, &sql_start_date);
    }

    if (end_date != NULL)
    {
        resRet = stringToTimestamp(end_date, &sql_end_date);
    }

    SQLINTEGER sql_procudere_switch =   procedure_switch;
    SQLINTEGER sql_sorting =            sorting;

    SQLCHAR sqlstate[6];
    SQLINTEGER native_error;
    SQLCHAR message_text[SQL_MAX_MESSAGE_LENGTH];
    SQLSMALLINT text_length;

    // Bind the parameter
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG,            SQL_INTEGER,          0, 0, &sql_procudere_switch,              0, NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP,   SQL_TYPE_TIMESTAMP,   0, 7, &sql_start_date,                    0, NULL);
    SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP,   SQL_TYPE_TIMESTAMP,   0, 7, &sql_end_date,                      0, NULL);
    SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG,            SQL_INTEGER,          0, 0, &sql_sorting,                       0, NULL);

    // Execute stored procedure
    retcode = SQLExecDirect(hstmt, (SQLCHAR*)call, SQL_NTS);

    cJSON* root = cJSON_CreateObject();

    if (SQL_SUCCEEDED(retcode))
    {
        // Process the invoice data
        cJSON* invoices = cJSON_AddArrayToObject(root, "invoices");

        do
        {
            SQLINTEGER      invoice_id;
            SQLINTEGER      customer_id;
            SQLCHAR         invoice_date[LEN_DATE];
            SQLCHAR         invoice_bankreference[LEN_BANK_REF];

            SQLDOUBLE       invoice_subtotal;
            SQLDOUBLE       invoice_tax;
            SQLDOUBLE       invoice_total;

            SQLCHAR         invoice_due_date[LEN_DUE_DATE];
            SQLDOUBLE       invoice_outstanding_balance;

            while (SQLFetch(hstmt) == SQL_SUCCESS)
            {
                SQLGetData(hstmt, 1, SQL_C_SLONG, &invoice_id, 0, NULL);
                SQLGetData(hstmt, 2, SQL_C_SLONG, &customer_id, 0, NULL);

                SQLGetData(hstmt, 3, SQL_C_CHAR, invoice_date, sizeof(invoice_date), NULL);
                SQLGetData(hstmt, 4, SQL_C_CHAR, invoice_bankreference, sizeof(invoice_bankreference), NULL);

                SQLGetData(hstmt, 5, SQL_C_DOUBLE, &invoice_subtotal, 0, NULL);
                SQLGetData(hstmt, 6, SQL_C_DOUBLE, &invoice_tax, 0, NULL);
                SQLGetData(hstmt, 7, SQL_C_DOUBLE, &invoice_total, 0, NULL);

                SQLLEN invoice_due_dateLen;

                SQLGetData(hstmt, 8, SQL_C_CHAR, invoice_due_date, sizeof(invoice_due_date), &invoice_due_dateLen);
                SQLGetData(hstmt, 9, SQL_C_DOUBLE, &invoice_outstanding_balance, 0, NULL);

                if (invoice_due_dateLen == SQL_NULL_DATA)
                {
                    strcpy_s(invoice_due_date, sizeof(invoice_due_date), "N/A");
                }

                cJSON* invoice = cJSON_CreateObject();

                cJSON_AddNumberToObject(invoice, "invoice_id", invoice_id);
                cJSON_AddNumberToObject(invoice, "customer_id", customer_id);
                cJSON_AddStringToObject(invoice, "invoice_date", invoice_date);
                cJSON_AddStringToObject(invoice, "invoice_bankreference", invoice_bankreference);

                cJSON_AddNumberToObject(invoice, "invoice_subtotal", invoice_subtotal);
                cJSON_AddNumberToObject(invoice, "invoice_tax", invoice_tax);
                cJSON_AddNumberToObject(invoice, "invoice_total", invoice_total);

                cJSON_AddStringToObject(invoice, "invoice_due_date", invoice_due_date);
                cJSON_AddNumberToObject(invoice, "invoice_outstanding_balance", invoice_outstanding_balance);

                cJSON_AddItemToArray(invoices, invoice);
            }
        } while (SQLMoreResults(hstmt) == SQL_SUCCESS);
    }
    else
    {
        SQLRETURN retussi = SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate, &native_error, message_text, sizeof(message_text), &text_length);
        printf("Error executing the stored procedure: %s\n", message_text);
    }

    // Free the statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    dbClose();

    char* archieCruzPlusIida = cJSON_Print(root);

    printf("%s", archieCruzPlusIida);

    *jsonString = archieCruzPlusIida;
    /*g_invoice_json_data*/ global_json_data  = *jsonString;

    cJSON_Delete(root);
    return 0;
}

/**
* This function ...
*
* @param invoice_id: ...
* @param invoiceline_product: ...
* @param invoiceline_quantity: ...
* @param invoiceline_price: ...
*/
void addInvoiceLine(
    _In_ bool                   open_database,
    _In_ int                    invoice_id,
    //_In_ char* invoiceline_product,
    _In_ int                    product_item_id,
    _In_ int                    invoiceline_quantity,
    _In_ double                 invoiceline_price,
    _In_ char*                  product_description)
{
    char fileName[21] = "connectionstring.txt";
    DBERROR* err = NULL;
    int ret = -10;

    if (open_database)
    {
        dbOpen(fileName, &err);

        if (err->errorCode < 0)
        {
            free(err);
        return;
        }
        ret = err->errorCode;
        free(err);
    }

    char* product_description_decoded = NULL;
    decodeUTF8Encoding(product_description, &product_description_decoded);

    SQLHSTMT hstmt;
    SQLINTEGER id_invoice;

    char query[1024];
    size_t bufferCount = 1024;     //const _BufferCount

    sprintf_s(query, bufferCount, "{CALL dbo.AddInvoiceLine (?, ?, ?, ?, ?)}");

    if (SQL_SUCCEEDED(ret) || !open_database)
    {
        // Allocate a statement handle
        SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,   0, 0, &invoice_id,           0, NULL);
        SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,   0, 0, &product_item_id,      0, NULL);
        SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,   0, 0, &invoiceline_quantity, 0, NULL);
        SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 10, 2, &invoiceline_price,    0, NULL);

        SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 1024, 0, product_description_decoded, 0, NULL);
        // Prepare the SQL statement
        ret = SQLPrepare(hstmt, query, SQL_NTS);
        if (SQL_SUCCEEDED(ret))
        {
            // Execute the query
            ret = SQLExecute(hstmt);

            SQLCHAR sqlstate[6];
            SQLINTEGER native_error;
            SQLCHAR message_text[SQL_MAX_MESSAGE_LENGTH];
            SQLSMALLINT text_length;

            SQLRETURN retussi = SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate, &native_error, message_text, sizeof(message_text), &text_length);

            if (SQL_SUCCEEDED(ret))
            {
                if (SQLMoreResults(hstmt) == SQL_NO_DATA)
                {
                }
            }
            // Free the statement handle
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        }
    }
    else {
        // Get the return code
        SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, 1, NULL, NULL, retcode, SQL_RETURN_CODE_LEN, NULL);
        printf("Error connecting to database: %s\n", retcode);
    }

    free(product_description_decoded);

    if (open_database)
    {
        dbClose();
    }
}

/**
* This function adds a new invoice into the database.
*
* @param customer_id:
* @param invoice_date:
* @param invoice_bankreference:
* @param invoice_subtotal:
* @param invoice_tax:
* @param invoice_total:
* @param invoice_idOut:
*/
int addInvoice(
    _In_ bool                   open_database,
    _In_ int                    customer_id,
    _In_ SQL_TIMESTAMP_STRUCT   invoice_date,
    _In_ char*                  invoice_bankreference,
    _In_ double                 invoice_subtotal,
    _In_ double                 invoice_tax,
    _In_ double                 invoice_total,
    _In_ SQL_DATE_STRUCT        invoice_due_date,
    _Out_ int*                  invoice_idOut, 
    _Out_ node_t**              errorList)
{
    int result_sql_sp_execute = -8;
    *invoice_idOut = 0;

    char fileName[21] = "connectionstring.txt";
    DBERROR* err = NULL;
    int ret = -10;

    if (open_database)
    {
        dbOpen(fileName, &err);

        if (err->errorCode < 0)
        {
            free(err);
            ret = -1;
            return ret;
        }
        ret = err->errorCode;
        free(err);
    }

    SQLHSTMT hstmt;
    SQLINTEGER id_invoice;

    char query[1024];
    size_t bufferCount = 1024;     //const _BufferCount, AddInvoice
    
    sprintf_s(query, bufferCount, "{? = CALL dbo.AddInvoice (?, ?, ?, ?, ?, ?, ?)}");

    if (SQL_SUCCEEDED(ret) || !open_database)
    {
        // Allocate a statement handle
        SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

        SQLINTEGER id_invoice;

        SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG,          SQL_INTEGER,               0,  0, &id_invoice,            0, NULL);
        
        SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT,  SQL_C_SLONG,          SQL_INTEGER,               0,  0, &customer_id,           0, NULL);
        SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT,  SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP,        0,  7, &invoice_date,          0, NULL);
        SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT,  SQL_C_CHAR,           SQL_VARCHAR,               20, 0, invoice_bankreference,  0, NULL);
        SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT,  SQL_C_DOUBLE,         SQL_DECIMAL,               10, 2, &invoice_subtotal,      0, NULL);
        SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT,  SQL_C_DOUBLE,         SQL_DECIMAL,               10, 2, &invoice_tax,           0, NULL);
        SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT,  SQL_C_DOUBLE,         SQL_DECIMAL,               10, 2, &invoice_total,         0, NULL);

        SQLBindParameter(hstmt, 8, SQL_PARAM_INPUT,  SQL_C_TYPE_DATE,      SQL_TYPE_DATE,             0,  0, &invoice_due_date,      0, NULL);

        // Prepare the SQL statement
        ret = SQLPrepare(hstmt, query, SQL_NTS);
        if (SQL_SUCCEEDED(ret))
        {
            // Execute the query
            ret = SQLExecute(hstmt);

            SQLErrorUtil(ret, hstmt, &internalErrorList);

            *errorList = internalErrorList;

             result_sql_sp_execute = -5;

#ifdef _DEBUG
            time_t current_time;
            time(&current_time);

            struct tm* time_info;
            time_info = localtime(&current_time);

            char timeString[20]; // Space for "HH:MM:SS\0"
            strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", time_info);

            SQLCHAR sqlstate[6] = "";
            SQLINTEGER native_error = 0;
            SQLCHAR message_text[SQL_MAX_MESSAGE_LENGTH] = "";
            SQLSMALLINT text_length = 0;;

            SQLRETURN retussi = SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate, &native_error, message_text, sizeof(message_text), &text_length);

            const char* file_path = "error_log.txt";

            // Open the file for writing
            FILE* file = fopen(file_path, "w");
            if (!file) {
                perror("Error opening file");
                return -2;
            }

            // Write the error message to the file
            fprintf(file, "Observation time: %s\n", timeString);
            fprintf(file, "SQL State: %s\n", sqlstate);
            fprintf(file, "Native Error: %d\n", native_error);
            fprintf(file, "Message Text: %.*s\n", text_length, message_text);

            // Close the file
            fclose(file);

            printf("Error message saved to %s\n", file_path);

#endif // _DEBUG

            if (SQL_SUCCEEDED(ret))
            {
                result_sql_sp_execute = -3;
                if (SQLMoreResults(hstmt) == SQL_NO_DATA)
                {
                    printf("%d", id_invoice);
                    *invoice_idOut = id_invoice;
                    result_sql_sp_execute = 0;
                }
            }
            // Free the statement handle
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        }
    }
    else {
        // Get the return code
        SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, 1, NULL, NULL, retcode, SQL_RETURN_CODE_LEN, NULL);
        printf("Error connecting to database: %s\n", retcode);
    }
    if (open_database)
    {
        dbClose();
    }
    return result_sql_sp_execute;
}

/**
* This function adds a new customer into the database.
*
* @param customer_firstName:
* @param customer_lastName:
* @param customer_address:
* @param customer_zip:
* @param customer_city:
* @param customer_id:
*/
void addCustomer(
    _In_ char* customer_firstName,
    _In_ char* customer_lastName,
    _In_ char* customer_address,
    _In_ char* customer_zip,
    _In_ char* customer_city,
    _In_ char* customer_phone,
    _In_ char* customer_email,
    _Out_ int* customer_id)
{
    *customer_id = 0;

    char fileName[21] = "connectionstring.txt";
    DBERROR* err = NULL;
    dbOpen(fileName, &err);

    if (err->errorCode < 0)
    {
        free(err);
        return;
    }
    int ret = err->errorCode;
    free(err);

    char* customer_firstName_decoded = NULL;
    decodeUTF8Encoding(customer_firstName, &customer_firstName_decoded);

    char* customer_lastName_decoded = NULL;
    decodeUTF8Encoding(customer_lastName, &customer_lastName_decoded);

    char* customer_address_decoded = NULL;
    decodeUTF8Encoding(customer_address, &customer_address_decoded);

    char* customer_city_decoded = NULL;
    decodeUTF8Encoding(customer_city, &customer_city_decoded);

    char query[1024];
    size_t bufferCount = 1024;     
    sprintf_s(query, bufferCount,
        "{? = CALL dbo.AddCustomer (?, ?, ?, ?, ?, ?, ?)}");

    SQLHSTMT hstmt;
    SQLINTEGER id;

    if (SQL_SUCCEEDED(ret)) {
        // Allocate a statement handle
        SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

        SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &id,                          0, NULL);
        SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT,  SQL_C_CHAR, SQL_VARCHAR,  50, 0, customer_firstName_decoded,   0, NULL);
        SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT,  SQL_C_CHAR, SQL_VARCHAR,  50, 0, customer_lastName_decoded,    0, NULL);
        SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT,  SQL_C_CHAR, SQL_VARCHAR, 100, 0, customer_address_decoded,     0, NULL);
        SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT,  SQL_C_CHAR, SQL_VARCHAR, 100, 0, customer_zip,                 0, NULL);
        SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT,  SQL_C_CHAR, SQL_VARCHAR,  50, 0, customer_city_decoded,        0, NULL);
        SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT,  SQL_C_CHAR, SQL_VARCHAR,  20, 0, customer_phone,               0, NULL);
        SQLBindParameter(hstmt, 8, SQL_PARAM_INPUT,  SQL_C_CHAR, SQL_VARCHAR, 100, 0, customer_email,               0, NULL);

        // Prepare the SQL statement
        ret = SQLPrepare(hstmt, query, SQL_NTS);
        if (SQL_SUCCEEDED(ret))
        {
            // Execute the query
            ret = SQLExecute(hstmt);

            if (SQL_SUCCEEDED(ret))
            {
                if (SQLMoreResults(hstmt) == SQL_NO_DATA)
                {
                    printf("%d", id);
                    *customer_id = id;
                }
            }
            // Free the statement handle
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        }
    }
    else {
        // Get the return code
        SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, 1, NULL, NULL, retcode, SQL_RETURN_CODE_LEN, NULL);
        printf("Error connecting to database: %s\n", retcode);
    }

    free(customer_firstName_decoded);
    free(customer_lastName_decoded);
    free(customer_address_decoded);
    free(customer_city_decoded);

    dbClose();
}


void DisplayError(SQLCHAR* sqlState, SQLINTEGER nativeError, SQLCHAR* message, SQLSMALLINT msgLen) {
    printf("SQLSTATE: %s\n", sqlState);
    printf("Native Error Code: %d\n", nativeError);
    printf("Error Message: %.*s\n", msgLen, message);
}

int getCustomerCharOut(
    _In_ int					customer_id,
    _Out_ char**                customer_data)
{
    cJSON* customer_data_pointer = NULL;
    int err = getCustomer(customer_id, &customer_data_pointer);
    if (!(err < 0))
    {
        *customer_data = cJSON_Print(customer_data_pointer);
        /*g_customer_json_data*/ global_json_data = *customer_data;
    }
    return err;
}

/**
*
*/
int getCustomer(_In_ int customer_id, _Out_ cJSON** customer_data)
{
    char fileName[21] = "connectionstring.txt";
    DBERROR* err = NULL;
    dbOpen(fileName, &err);
    *customer_data = NULL;

    if (err->errorCode < 0)
    {
        int errValueSQL = (int)err->errorCode;
        free(err);
        return errValueSQL;
    }
    else if (err->errorInt < 0)
    {
        int errValue = err->errorInt;
        free(err);
        return errValue;
    }
    int ret = err->errorCode;
    free(err);

    char query[1024];
    size_t bufferCount = 1024;
    sprintf_s(query, bufferCount,
        "{CALL dbo.GetCustomer (?)}");

    SQLHSTMT hstmt;
    SQLINTEGER id;

    if (SQL_SUCCEEDED(ret)) 
    {
        SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,   0, 0, &customer_id,       0, NULL);

        // Prepare the SQL statement
        ret = SQLPrepare(hstmt, query, SQL_NTS);
        if (SQL_SUCCEEDED(ret))
        {
            // Execute the query
            ret = SQLExecute(hstmt);
            // Check for success or info
            if (ret == SQL_SUCCESS_WITH_INFO || ret == SQL_ERROR)
            {
                SQLLEN numRecs = 0;
                SQLGetDiagField(SQL_HANDLE_STMT, hstmt, 0, SQL_DIAG_NUMBER, &numRecs, 0, 0);

                // Retrieve diagnostic records
                for (SQLSMALLINT i = 1; i <= numRecs; ++i)
                {
                    SQLCHAR sqlState[6];
                    SQLINTEGER nativeError;
                    SQLCHAR errorMsg[SQL_MAX_MESSAGE_LENGTH];
                    SQLSMALLINT msgLen;

                    ret = SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, i, sqlState, &nativeError, errorMsg, sizeof(errorMsg), &msgLen);
                    if (ret != SQL_NO_DATA) {
                        DisplayError(sqlState, nativeError, errorMsg, msgLen);
                    }
                }
                if (ret = SQL_ERROR)
                    goto exit;
            }

            *customer_data = cJSON_CreateObject();

            // Fetch results
            while (SQLFetch(hstmt) == SQL_SUCCESS) 
            {
                SQLCHAR customer_first_name[LEN_FIRST_NAME];
                SQLCHAR customer_last_name[LEN_LAST_NAME];
                SQLCHAR customer_address[LEN_ADDRESS];
                SQLCHAR customer_zip[LEN_ZIP];
                SQLCHAR customer_city[LEN_CITY];
                SQLCHAR customer_phone[LEN_CITY];
                SQLCHAR customer_email[LEN_EMAIL];

                SQLLEN len_customer_first_name;
                SQLLEN len_customer_last_name;
                SQLLEN len_customer_address;
                SQLLEN len_customer_zip;
                SQLLEN len_customer_city;

                SQLLEN len_customer_phone;
                SQLLEN len_customer_email;

                int ret1 = SQLGetData(hstmt, 2, SQL_C_CHAR, customer_first_name, sizeof(customer_first_name), &len_customer_first_name);
                int ret2 = SQLGetData(hstmt, 3, SQL_C_CHAR, customer_last_name,  sizeof(customer_last_name),  &len_customer_last_name);
                int ret3 = SQLGetData(hstmt, 4, SQL_C_CHAR, customer_address,    sizeof(customer_address),    &len_customer_address);
                int ret4 = SQLGetData(hstmt, 5, SQL_C_CHAR, customer_zip,        sizeof(customer_zip),        &len_customer_zip);
                int ret5 = SQLGetData(hstmt, 6, SQL_C_CHAR, customer_city,       sizeof(customer_city),       &len_customer_city);
                int ret6 = SQLGetData(hstmt, 7, SQL_C_CHAR, customer_phone,      sizeof(customer_phone),      &len_customer_phone);
                int ret7 = SQLGetData(hstmt, 8, SQL_C_CHAR, customer_email,      sizeof(customer_email),      &len_customer_email);

                if (SQL_SUCCEEDED(ret1) && SQL_SUCCEEDED(ret2) && SQL_SUCCEEDED(ret3) && SQL_SUCCEEDED(ret4) && SQL_SUCCEEDED(ret5) && SQL_SUCCEEDED(ret6) && SQL_SUCCEEDED(ret7))
                {
                    cJSON_AddNumberToObject(*customer_data,   "customer_id",         customer_id);
                    cJSON_AddItemToObject(  *customer_data,   "customer_first_name", cJSON_CreateString(customer_first_name));
                    cJSON_AddItemToObject(  *customer_data,   "customer_last_name",  cJSON_CreateString(customer_last_name));
                    cJSON_AddItemToObject(  *customer_data,   "customer_address",    cJSON_CreateString(customer_address));
                    cJSON_AddItemToObject(  *customer_data,   "customer_zip",        cJSON_CreateString(customer_zip));
                    cJSON_AddItemToObject(  *customer_data,   "customer_city",       cJSON_CreateString(customer_city));

                    // null email null phone situation

                    if (len_customer_phone == SQL_NULL_DATA)
                    {
                        cJSON_AddItemToObject(*customer_data, "customer_phone", cJSON_CreateString("N/A"));
                    }
                    else
                    {
                        cJSON_AddItemToObject(*customer_data, "customer_phone", cJSON_CreateString(customer_phone));
                    }
                    if (len_customer_email == SQL_NULL_DATA)
                    {
                        cJSON_AddItemToObject(*customer_data, "customer_email", cJSON_CreateString("N/A"));
                    }
                    else
                    {
                        cJSON_AddItemToObject(*customer_data, "customer_email", cJSON_CreateString(customer_email));
                    }
                }
                else
                {
                    ret = -1;
                }
            }
        }
    }
exit:
    // Free the statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    dbClose();
    return ret;
}


/**
* This function adds three tables into a database.
* The tables are: a customer, an invoice and an invoice_line tables.
*/
void createTables()
{
    char fileName[21] = "connectionstring.txt";
    DBERROR* err = NULL;
    dbOpen(fileName, &err);

    if (err->errorCode < 0)
    {
        free(err);
        return;
    }
    int ret = err->errorCode;
    free(err);

    // Assume you have a stored procedure named "CreateTablesIfNotExist"
    SQLCHAR* storedProcedureCall = (SQLCHAR*) "EXEC [dbo].[CreateTablesIfNotExist]";
    
    SQLHSTMT hstmt;
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    if (SQL_SUCCEEDED(ret)) {
        ret = SQLPrepare(hstmt, storedProcedureCall, SQL_NTS);

        if (SQL_SUCCEEDED(ret))
        {
            ret = SQLExecute(hstmt);
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    dbClose();
}

/**
* This function adds a new invoice with invoice lines to database.
* 
* @param invoicing_data_json The invoicing data as a string. This must be in proper json data containing every mandatory items.
* @param length The length of the invoicing_data_json string
*/
int addNewInvoiceData(_In_ char* invoicing_data_json, _In_ int length)
{
#ifdef _DEBUG
    const char* file_path = "invoice_data.json";

    // Open the file for writing
    FILE* file = fopen(file_path, "w");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    // Write the JSON data to the file
    fprintf(file, "%s\n", invoicing_data_json);

    // Close the file
    fclose(file);

    printf("Data saved to %s\n", file_path);
#endif // _DEBUG

    // Parse the JSON string
    cJSON* root = cJSON_Parse(invoicing_data_json);

    if (root)
    {
        int customer_id = -1;
        int invoice_id = -1;

        bool open_database =                false;

        cJSON* id =                         cJSON_GetObjectItem(root, "customer_id");
        if (!id)
        {
            goto exit;
        }

        cJSON* bankRef =                    cJSON_GetObjectItem(root, "bank_reference");
        if (!bankRef)
        {
            goto exit;
        }

        cJSON* invoice_subtotal =           cJSON_GetObjectItem(root, "invoice_subtotal");
        if (!invoice_subtotal)
        {
            goto exit;
        }

        cJSON* invoice_tax =                cJSON_GetObjectItem(root, "invoice_tax");
        if (!invoice_tax)
        {
            goto exit;
        }

        cJSON* invoice_total =              cJSON_GetObjectItem(root, "invoice_total");
        if (!invoice_total)
        {
            goto exit;
        }

        cJSON* invoice_date =               cJSON_GetObjectItem(root, "invoice_date");
        if (!invoice_date)
        {
            goto exit;
        }
        cJSON* invoice_due_date = cJSON_GetObjectItem(root, "invoice_due_date");
        if (!invoice_due_date)
        {
            goto exit;
        }

        char fileName[21] = "connectionstring.txt";
        DBERROR* err = NULL;
        int ret = -10;

        bool open_database_general = true;

        if (open_database_general)
        {
            dbOpen(fileName, &err);

            if (err->errorCode < 0)
            {
                free(err);
                return -1;
            }
            ret = err->errorCode;
            free(err);
        }

        SQL_TIMESTAMP_STRUCT myTimestamp;
        stringToTimestamp(invoice_date->valuestring, &myTimestamp);

        SQL_DATE_STRUCT invoiceDueDate;
        stringToDate(invoice_due_date->valuestring, &invoiceDueDate);

        node_t* errs = NULL;

        int val = addInvoice(
            _In_(bool)                     open_database,
            _In_(int)                      id->valueint,
            _In_                           myTimestamp,
            _In_(char*)                    bankRef->valuestring,
            _In_(double)                   invoice_subtotal->valuedouble,
            _In_(double)                   invoice_tax->valuedouble,
            _In_(double)                   invoice_total->valuedouble,
            _In_                           invoiceDueDate,
            _Out_(int*)                    &invoice_id, 
                                           &errs);

        if (errs != NULL)
        {
            cJSON_Delete(root);
            free_sql_error_details();
            return val;
        }
        free_sql_error_details();

        cJSON* invoiceLines =       cJSON_GetObjectItem(root, "invoice_lines");

        int size =                  cJSON_GetArraySize(invoiceLines);

        cJSON* item = NULL;

        for (int index = 0; index < size; index++)
        {
            item =                  cJSON_GetArrayItem(invoiceLines, index);
            if (!item)
            {
                goto exit;
            }

            cJSON* product_item_id =   cJSON_GetObjectItem(item, "product_item_id");
            if (!product_item_id)
            {
                goto exit;
            }

            cJSON* quantity =       cJSON_GetObjectItem(item, "quantity");
            if (!quantity)
            {
                goto exit;
            }

            cJSON* price =          cJSON_GetObjectItem(item, "price");
            if (!price)
            {
                goto exit;
            }

            cJSON* product_description = cJSON_GetObjectItem(item, "product_description");
            if (!product_description)
            {
                goto exit;
            }

            addInvoiceLine(
                _In_(bool)                   open_database,
                _In_(int)                    invoice_id,
                _In_(int)                    product_item_id->valueint, // invoiceline_product
                _In_(int)                    quantity->valueint,        // invoiceline_quantity
                _In_(double)                 price->valuedouble,      // invoiceline_price
                                             product_description->valuestring);
        }

        if (open_database_general)
        {
            dbClose();
        }

        cJSON_Delete(root);
        return 0;
    }
exit:
    return -1;
}

