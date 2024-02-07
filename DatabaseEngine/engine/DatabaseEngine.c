#include <stdio.h>
#include "DatabaseEngine.h"

/**
* This data is global due to another function must be able to free the memory.
*/
char* json_data = NULL;

/**
* This function returns a path to the working directory.
* 
* @param pWorkingDir: A path to the working directory of the application
* @return: An integer value indication a success or an error in the function
*/
_Success_(return == 0)
int getWorkingDir(_Out_ char** pWorkingDir)
{
    int ret = 0;
    char cwd[1024] = "";
    *pWorkingDir = NULL;

    if (_getcwd(cwd, sizeof(cwd)) != 0) {
        printf("Current working directory: **%s**\n", cwd);
    }
    else {
        perror("_getcwd() error");
        ret = ERROR_CODE;
        goto error;
    }

    size_t count = strlen(cwd);

    if (NULL == (*pWorkingDir = (char*)malloc(sizeof(char) * count + 1)))
    {
        ret = ERROR_CODE;
        goto error;
    }

#pragma warning( push )
#pragma warning( disable : 6387 )

    strcpy_s(*pWorkingDir, (count + 1), cwd);
    return ret;

#pragma warning( pop )

error:
    free(*pWorkingDir);
    return ret;
}

/**
* This function returns a connection string.
* 
* @param workingDirectory: A path to the working directory of the application
* @param fileName: A name of the file that must contain the connection string
* @param connectionString: A connection string
* @return: An integer value indication a success or an error in the function
*/
int getConnectionString(
    _Inout_ char** workingDirectory, 
    _In_ char* fileName, 
    _Out_ char** connectionString)
{
    int ret = 0;

    *connectionString = NULL;

    size_t lenPath = strlen(*workingDirectory) + 2;
    size_t lenFileName = strlen(fileName);

    size_t lenFullPath = lenPath + lenFileName;

    char* temp = NULL;

    if (!!*workingDirectory)
    {
        if (NULL == (temp = (char*)realloc(*workingDirectory, lenPath)))
        {
            ret = ERROR_CODE;
            goto error;
        }
        *workingDirectory = temp;
        temp = NULL;

        size_t s = strlen(*workingDirectory);

        strcat_s(*workingDirectory, s + 2, "\\");

        if (NULL == (temp = (char*)realloc(*workingDirectory, lenFullPath + 1)))
        {
            ret = ERROR_CODE;
            goto error;
        }

        *workingDirectory = temp;
        temp = NULL;

        s = strlen(*workingDirectory);

        size_t ss = strlen(fileName);

        strcat_s(*workingDirectory, s + ss + 1, fileName);
    }

    readFile(workingDirectory, connectionString);

    return ret;

error:
    free(*workingDirectory);
    *workingDirectory = NULL;
    return ret;
}

/**
* This function reads a content of a file in the file system.
* 
* @param workingDirectory: A path to the working directory of the application
* @param connectionString: A connection string
* @return: An integer value indication a success or an error in the function
*/
int readFile(
    _In_ char** workingDirectory, 
    _Out_ char** connectionString)
{
    int ret = 0;
    FILE* file = NULL;

    *connectionString = NULL;

    errno_t err = fopen_s(&file, *workingDirectory, L"r, ccs=UTF-8");
    if (err != 0) {
        printf("Cannot open file.\n");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    int* integers = (int*)malloc((length) * sizeof(int));
    if (integers) {
        int b = -1;
        int i = 0;
        while ((b = fgetc(file)) != EOF) {
            // printf("%d ", b);
            integers[i] = b;
            i++;
        }
    }
    
    unsigned char iso8859_1 = 0;

    unsigned char* chars = (unsigned char*)malloc(sizeof(unsigned char) + 1);

    if (!!chars)
    {
#pragma warning( push )
#pragma warning( disable : 6011 )
        chars[0] = 'a';
#pragma warning( pop )
        chars[1] = '\0';
    }
    else
    {
        ret = ERROR_CODE;
        goto exit;
    }

    long indexOfTheISO88591Array = 0;

    for (long indexOfCodepointArray = 0; indexOfCodepointArray < length; indexOfCodepointArray++)
    {
        unsigned char utf8[] = { 0, 0 };
#pragma warning( push )
#pragma warning( disable : 6011 )
        utf8[0] = integers[indexOfCodepointArray];
#pragma warning( pop )

        // If the UTF-8 character is in the ASCII range, it's the same in ISO 8859-1
        if (utf8[0] < 128) {
            iso8859_1 = utf8[0];
        }
        // Otherwise, if it's a 2-byte UTF-8 character, we can find the ISO 8859-1 character by subtracting 194 from the first byte
        else if (utf8[0] < 224) {
#pragma warning( push )
#pragma warning( disable : 6385)
            utf8[1] = (unsigned char) integers[indexOfCodepointArray + 1];
#pragma warning( pop )
            iso8859_1 = (utf8[0] & 0x1F) << 6 | (utf8[1] & 0x3F);
        }

#pragma warning( push )
#pragma warning( disable : 28182 )
        chars[indexOfTheISO88591Array] = (unsigned char) iso8859_1;
#pragma warning( pop )

        unsigned char* tem = (unsigned char*)realloc(chars, (indexOfTheISO88591Array + 1) * sizeof(unsigned char) + 1);

        if (!tem)
        {
            free(chars);
            ret = ERROR_CODE;
            goto exit;
        }

#pragma warning( push )
#pragma warning( disable : 28182 )
        long currentIndex = ((indexOfTheISO88591Array + 1) * sizeof(unsigned char));
        tem[currentIndex] = '\0';
#pragma warning( pop )
        chars = tem;
        if (utf8[0] >= 128 && utf8[0] < 224)
        {
            indexOfCodepointArray++;
        }
        indexOfTheISO88591Array++;
    }

    *connectionString = chars;

    fclose(file);
    free(integers);

    return 0;

exit:
    fclose(file);
    free(integers);
    return ret;
}

#define SQL_RESULT_LEN 240
#define SQL_RETURN_CODE_LEN 1000

SQLHENV henv;  // Environment handle
SQLHDBC hdbc;  // Connection handle
SQLRETURN ret; // Return code


SQLCHAR result[SQL_RESULT_LEN];
SQLCHAR retcode[SQL_RETURN_CODE_LEN];

/**
* This function open the connection to a database using an ODBC driver.
* 
* @param fileName: A name of the file that must contain the connection string
* @return: An integer value indication a success or an error in the function.
*          Zero means that the function has succeeded
*/
int dbOpen(
    _In_ char* fileName) 
{
    int err = 0;

    char* workingDirectory = NULL;
    char* connectionStringW = NULL;

    // Allocate an environment handle
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        fprintf(stderr, "Error allocating environment handle\n");
        ret = ERROR_CODE;
        goto error;
    }

    // Set the ODBC version
    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        fprintf(stderr, "Error setting ODBC version\n");
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        ret = ERROR_CODE;
        goto error;
    }

    // Allocate a connection handle
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        fprintf(stderr, "Error allocating connection handle\n");
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        ret = ERROR_CODE;
        goto error;
    }

    if (-1 == (err = getWorkingDir(&workingDirectory)))
    {
        ret = ERROR_CODE;
        goto error;
    }

    if (-1 == (err = getConnectionString(&workingDirectory, fileName, &connectionStringW)))
    {
        ret = ERROR_CODE;
        goto error;
    }

    if (!!connectionStringW)
    {
        // Connect to the database
        ret = SQLDriverConnect(hdbc, NULL, (SQLCHAR*)connectionStringW, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            fprintf(stderr, "Error connecting to SQL Server\n");
            SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
            SQLFreeHandle(SQL_HANDLE_ENV, henv);
            ret = ERROR_CODE;
            goto error;
        }
    }
    else
    {
        printf("ERROR: The connection string is unavaible.");
    }

error:
    free(connectionStringW);
    free(workingDirectory);
    return ret;
}

/**
* This function closes the ODBC connection to the Sql Server database.
*/
void dbClose()
{
    // Disconnect and free handles
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

/**
* 
*/
char* getInvoicesAsjson_data(_In_ int customer_id) 
{
    // Allocate memory for JSON data
    json_data = malloc(100);  // Example: allocating 100 bytes
    if (json_data == NULL) {
        perror("Error allocating memory for JSON data");
        exit(EXIT_FAILURE);
    }

    // Populate the allocated memory with JSON data
    strcpy(json_data, "{\"key\": \"value\"}");  // Example: JSON data

    return json_data;
}

int free_json_data() {
    int done = 744;
    free(json_data);
    return done;
}

/**
* This function ...
*
* @param customer_id: ...
*/
void queryInvoicesByCustomer(_In_ int customer_id)
{
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

        // Process the rows and print to screen
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
            SQLINTEGER invoice_id;
            SQLINTEGER customer_id_out;
            SQL_TIMESTAMP_STRUCT ts;

            SQLCHAR invoice_bankreference[20];

            do {
                SQLCHAR columnName[64];
                SQLSMALLINT columnNameLength;

                SQLRETURN ret = SQLDescribeCol(hstmt, 1, columnName, sizeof(columnName), &columnNameLength, NULL, NULL, NULL, NULL);
 
                if (strcmp((char*)columnName, "invoice_id") == 0) {
                    // Process the invoice data
                    SQLINTEGER invoiceId;
                    SQLCHAR bankReference[64];
                    while (SQLFetch(hstmt) == SQL_SUCCESS) {
                        SQLGetData(hstmt, 1, SQL_C_SLONG, &invoiceId, 0, NULL);
                        SQLGetData(hstmt, 4, SQL_C_CHAR, bankReference, sizeof(bankReference), NULL);
                        printf("Invoice ID: %d, Bank Reference: %s\n", invoiceId, bankReference);
                    }
                }
                else if (strcmp((char*)columnName, "invoice_line_id") == 0) {
                    // Process the invoice line data
                    SQLINTEGER lineId;
                    SQLCHAR productName[64];
                    while (SQLFetch(hstmt) == SQL_SUCCESS) {
                        SQLGetData(hstmt, 1, SQL_C_SLONG, &lineId, 0, NULL);
                        SQLGetData(hstmt, 3, SQL_C_CHAR, productName, sizeof(productName), NULL);
                        printf("  Line ID: %d, Product Name: %s\n", lineId, productName);
                    }
                }
            } while (SQLMoreResults(hstmt) == SQL_SUCCESS);
        }

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
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
    _In_ int invoice_id, 
    _In_ char* invoiceline_product,
    _In_ int invoiceline_quantity,
    _In_ double invoiceline_price)
{
    SQLHSTMT hstmt;
    SQLINTEGER id_invoice;

    char query[1024];
    size_t bufferCount = 1024;     //const _BufferCount

    sprintf_s(query, bufferCount, "{CALL dbo.AddInvoiceLine (?, ?, ?, ?)}");

    if (SQL_SUCCEEDED(ret)) {
        // Allocate a statement handle
        SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,   0, 0, &invoice_id,           0, NULL);
        SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,   10, 0, invoiceline_product,   0, NULL);
        SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,   0, 0, &invoiceline_quantity, 0, NULL);
        SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 10, 2, &invoiceline_price,    0, NULL);
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
void addInvoice(
    _In_ int                    customer_id,
    _In_ SQL_TIMESTAMP_STRUCT   invoice_date,
    _In_ char*                  invoice_bankreference,
    _In_ double                 invoice_subtotal,
    _In_ double                 invoice_tax,
    _In_ double                 invoice_total,
    _Out_ int*                  invoice_idOut)
{
    SQLHSTMT hstmt;
    SQLINTEGER id_invoice;

    char query[1024];
    size_t bufferCount = 1024;     //const _BufferCount, AddInvoice
    
    sprintf_s(query, bufferCount, "{? = CALL dbo.AddInvoice (?, ?, ?, ?, ?, ?)}");

    if (SQL_SUCCEEDED(ret)) {
        // Allocate a statement handle
        SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

        SQLINTEGER id_invoice;

        SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &id_invoice, 0, NULL);
        
        SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG,          SQL_INTEGER,               0,  0, &customer_id,           0, NULL);
        SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP,        0,  0, &invoice_date,          0, NULL);
        SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,               20, 0, invoice_bankreference,  0, NULL);
        SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE,         SQL_DECIMAL,               10, 2, &invoice_subtotal,      0, NULL);
        SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_DOUBLE,         SQL_DECIMAL,               10, 2, &invoice_tax,           0, NULL);
        SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_DOUBLE,         SQL_DECIMAL,               10, 2, &invoice_total,         0, NULL);

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
                    printf("%d", id_invoice);
                    *invoice_idOut = id_invoice;
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
    _Out_ int* customer_id)
{
    char query[1024];
    size_t bufferCount = 1024;     
    sprintf_s(query, bufferCount,
        "{? = CALL dbo.AddCustomer (?, ?, ?, ?, ?)}");

    SQLHSTMT hstmt;
    SQLINTEGER id;

    if (SQL_SUCCEEDED(ret)) {
        // Allocate a statement handle
        SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

        SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &id,                0, NULL);
        SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT,  SQL_C_CHAR, SQL_VARCHAR,  50, 0, customer_firstName, 0, NULL);
        SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT,  SQL_C_CHAR, SQL_VARCHAR,  50, 0, customer_lastName,  0, NULL);
        SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT,  SQL_C_CHAR, SQL_VARCHAR, 100, 0, customer_address,   0, NULL);
        SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT,  SQL_C_CHAR, SQL_VARCHAR, 100, 0, customer_zip,       0, NULL);
        SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT,  SQL_C_CHAR, SQL_VARCHAR,  50, 0, customer_city,      0, NULL);

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
}

/**
* This functions adds three tables into a database.
* The tables are: a customer, an invoice and an invoice_line tables.
*/
void createTables()
{
    // Assume you have a stored procedure named "CreateTablesIfNotExist"
    SQLCHAR* storedProcedureCall = (SQLCHAR*) "EXEC [dbo].[CreateTablesIfNotExist]";
    
    SQLHSTMT hstmt;
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        // Handle error
        // ...

    }
    else {
        ret = SQLPrepare(hstmt, storedProcedureCall, SQL_NTS);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            // Handle error
            // ...

        }
        else {

            if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
                // Handle error
                // ...

            }
            else {
                // Execute the stored procedure
                //SQLINTEGER registerValue = 0;
                //SQLBindCol(hstmt, 1, SQL_C_LONG, &registerValue, sizeof(registerValue), NULL);


                ret = SQLExecute(hstmt);

                if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
                    // Handle error
                    // ...

                }
                else {
                    // Process the results if applicable
                    // ...
                    //while (SQLFetch(hstmt) == SQL_SUCCESS) {
                    //    printf("Register Value: %d\n", registerValue);
                    //}
                    //SQLBindCol(hstmt, columnNumber, valueType, targetValue, bufferLength, indicatorValue);

                    // Close the statement when done
                    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
                }
            }
        }
    }
}
