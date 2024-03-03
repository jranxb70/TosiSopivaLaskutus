#include <stdio.h>
#include "DatabaseEngine.h"
#include "Utilities.h"
#include "cJSON.h"


/**
* This data is global due to another function must be able to free the memory.
*/
char* json_data = NULL;
cJSON* root = NULL;

#define SQL_RESULT_LEN       240
#define SQL_RETURN_CODE_LEN 1000

SQLHENV henv;  // Environment handle
SQLHDBC hdbc;  // Connection handle

SQLCHAR result[SQL_RESULT_LEN];
SQLCHAR retcode[SQL_RETURN_CODE_LEN];

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

    (*dbErr)->errorCode = 0;
    (*dbErr)->failedFunction = ErrFuncNone;

    // Allocate an environment handle
    SQLRETURN retHandle = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (retHandle != SQL_SUCCESS && retHandle != SQL_SUCCESS_WITH_INFO) {
        fprintf(stderr, "Error allocating environment handle\n");
        SQLRETURN retHandle = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
        (*dbErr)->errorCode = retHandle;
        (*dbErr)->failedFunction = ErrFuncSQLAllocHandleA;
        goto error;
    }

    // Set the ODBC version
    SQLRETURN retEnvAttr = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (retEnvAttr != SQL_SUCCESS && retEnvAttr != SQL_SUCCESS_WITH_INFO) {
        fprintf(stderr, "Error setting ODBC version\n");
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        (*dbErr)->errorCode = retEnvAttr;
        (*dbErr)->failedFunction = ErrFuncSQLSetEnvAttrA;
        goto error;
    }

    //// Allocate a connection handle
    SQLRETURN retConHandle = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (retConHandle != SQL_SUCCESS && retConHandle != SQL_SUCCESS_WITH_INFO) {
        fprintf(stderr, "Error allocating connection handle\n");
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        (*dbErr)->errorCode = retConHandle;
        (*dbErr)->failedFunction = ErrFuncSQLAllocHandleB;
        goto error;
    }

    if (-1 == (err = getWorkingDir(&workingDirectory)))
    {
        //ret = ERROR_CODE;
        goto error;
    }

    if (-1 == (err = getConnectionString(&workingDirectory, fileName, &connectionStringW)))
    {
        //ret = ERROR_CODE;
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
            ret = ERROR_DBOPEN_FAILED;
            goto error;
        }
    }
    else
    {
        ret = ERROR_CONNECTION_STRING_UNAVAILABLE;
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
int free_json_data() {
    int done = 744;

    free(json_data);
    return done;
}

void free_sql_error_details()
{
    DeleteList(&s);
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

    SQLErrorUtil(retcode, hstmt, &s);

    *errorList = s;

    root = cJSON_CreateObject();
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
                SQLCHAR     firstName[64];
                SQLCHAR     lastName[64];
                SQLCHAR     address[64];
                SQLCHAR     zip[64];
                SQLCHAR     city[64];

                while (SQLFetch(hstmt) == SQL_SUCCESS) 
                {
                    SQLGetData(hstmt, 1, SQL_C_SLONG,   &customerId,     0,                     NULL);
                    SQLGetData(hstmt, 2, SQL_C_CHAR,    firstName,      sizeof(firstName),      NULL);
                    SQLGetData(hstmt, 3, SQL_C_CHAR,    lastName,       sizeof(lastName),       NULL);
                    SQLGetData(hstmt, 4, SQL_C_CHAR,    address,        sizeof(address),        NULL);
                    SQLGetData(hstmt, 5, SQL_C_CHAR,    zip,            sizeof(zip),            NULL);
                    SQLGetData(hstmt, 6, SQL_C_CHAR,    city,           sizeof(city),           NULL);

                    printf("Customer ID: %d, First name: %s, Last name: %s, Address: %s, Address: %s, City: %s\n", 
                            customerId,
                            firstName, 
                            lastName, 
                            address,
                            zip,
                            city);

                    cJSON_AddNumberToObject(root, "customer_id", customerId);
                    cJSON_AddStringToObject(root, "first_name", firstName);
                    cJSON_AddStringToObject(root, "last_name", lastName);
                    cJSON_AddStringToObject(root, "address", address);
                    cJSON_AddStringToObject(root, "zip", zip);
                    cJSON_AddStringToObject(root, "city", city);

                    invoices = cJSON_AddArrayToObject(root, "invoices");
                }
            }
            else if (strcmp((char*)columnName, "invoice_id") == 0) 
            {
                // Process the invoice data
                SQLINTEGER invoiceId;
                SQLCHAR bankReference[64];
                while (SQLFetch(hstmt) == SQL_SUCCESS) 
                {
                    SQLGetData(hstmt, 1, SQL_C_SLONG, &invoiceId, 0, NULL);
                    SQLGetData(hstmt, 4, SQL_C_CHAR, bankReference, sizeof(bankReference), NULL);
                    printf("Invoice ID: %d, Bank Reference: %s\n", invoiceId, bankReference);

                    cJSON* invoice = cJSON_CreateObject();
                    cJSON_AddNumberToObject(invoice, "invoice_id", invoiceId);
                    cJSON_AddStringToObject(invoice, "bank_reference", bankReference);
                    invoice_lines = cJSON_AddArrayToObject(invoice, "invoice_lines");

                    cJSON_AddItemToArray(invoices, invoice);
                }
            }
            else if (strcmp((char*)columnName, "invoice_line_id") == 0) 
            {
                // Process the invoice line data
                SQLINTEGER lineId;
                SQLINTEGER invoiceId;
                SQLCHAR productName[64];
                while (SQLFetch(hstmt) == SQL_SUCCESS) {
                    SQLGetData(hstmt, 1, SQL_C_SLONG, &lineId, 0, NULL);
                    SQLGetData(hstmt, 2, SQL_C_SLONG, &invoiceId, 0, NULL);
                    SQLGetData(hstmt, 3, SQL_C_CHAR, productName, sizeof(productName), NULL);
                    printf("  Line ID: %d, Product Name: %s\n", lineId, productName);

                    ///////////////////////////////////////////

                    cJSON* invoice_line = cJSON_CreateObject();
                    cJSON_AddNumberToObject(invoice_line, "invoice_line_id", lineId);
                    cJSON_AddStringToObject(invoice_line, "product_name", productName);

                    cJSON* invoices = cJSON_GetObjectItem(root, "invoices");
                    int arraySizeInvoices = cJSON_GetArraySize(invoices);

                    cJSON* invoice = NULL;
                    for (int ind = 0; ind < arraySizeInvoices; ind++)
                    {
                        invoice = cJSON_GetArrayItem(invoices, ind);
                        cJSON* it = cJSON_GetObjectItem(invoice, "invoice_id");
                        double wtf = cJSON_GetNumberValue(it);
                        if (invoiceId == (int) wtf)
                        {
                            cJSON* invoice_lines = cJSON_GetObjectItem(invoice, "invoice_lines");
                            cJSON_AddItemToArray(invoice_lines, invoice_line);
                            break;
                        }
                    }
                    }
                    }
        } while (SQLMoreResults(hstmt) == SQL_SUCCESS);
    }

    char* archieCruzPlusIida = cJSON_Print(root);

    printf("%s", archieCruzPlusIida);

    *jsonString = archieCruzPlusIida;

    cJSON_Delete(root);

    // Free the statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    dbClose();
}

//{

//        "customerId": 1,
//        "first_name": "Kostos",
// //     "last_name":  "Kouruhousu",
//        "invoices" : [
        //        {
        //            "invoiceId": 100,
        //                "amount" : 200.00,
        //                "date" : "2024-02-19",
        //                "lines" : [
        //            {
        //                "lineId": 1,
        //                    "description" : "Product 1",
        //                    "quantity" : 2,
        //                    "price" : 100.00
        //            }
//                ]
//        },

//}

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
    _In_ char*                  invoiceline_product,
    _In_ int                    invoiceline_quantity,
    _In_ double                 invoiceline_price)
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

    SQLHSTMT hstmt;
    SQLINTEGER id_invoice;

    char query[1024];
    size_t bufferCount = 1024;     //const _BufferCount

    sprintf_s(query, bufferCount, "{CALL dbo.AddInvoiceLine (?, ?, ?, ?)}");

    if (SQL_SUCCEEDED(ret) || !open_database)
    {
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
void addInvoice(
    _In_ bool                   open_database,
    _In_ int                    customer_id,
    _In_ SQL_TIMESTAMP_STRUCT   invoice_date,
    _In_ char*                  invoice_bankreference,
    _In_ double                 invoice_subtotal,
    _In_ double                 invoice_tax,
    _In_ double                 invoice_total,
    _Out_ int*                  invoice_idOut)
{
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
        return;
        }
        ret = err->errorCode;
        free(err);
    }

    SQLHSTMT hstmt;
    SQLINTEGER id_invoice;

    char query[1024];
    size_t bufferCount = 1024;     //const _BufferCount, AddInvoice
    
    sprintf_s(query, bufferCount, "{? = CALL dbo.AddInvoice (?, ?, ?, ?, ?, ?)}");

    if (SQL_SUCCEEDED(ret) || !open_database)
    {
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
    if (open_database)
    {
    dbClose();
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
    dbClose();
}


/**
* This functions adds three tables into a database.
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
    dbClose();
}

/**
* 
*/
void addNewInvoiceData(_In_ char* arr, _In_ int size)
{
    // Parse the JSON string
    cJSON* root = cJSON_Parse(arr);

    if (root)
    {
        int invoice_id = -1;

        //double invoice_subtotal = 0.0;
        //double invoice_tax = 0.00;
        //double invoice_total = 0.00;

        //char* product = "kalja";
        //int quantity = 10;
        //double price = 0.84;

        //char* product2 = "siideri";
        //int quantity2 = 8;
        //double price2 = 1.64;

        //char* product3 = "lonkero";
        //int quantity3 = 6;
        //double price3 = 2.04;

        bool open_database =                false;
        cJSON* id =                         cJSON_GetObjectItem(root, "customer_id");
        int customer_id =                   id->valueint;

        cJSON* bankRef =                    cJSON_GetObjectItem(root, "bank_reference");

        cJSON* invoice_subtotal =           cJSON_GetObjectItem(root, "invoice_subtotal");
        cJSON* invoice_tax =                cJSON_GetObjectItem(root, "invoice_tax");
        cJSON* invoice_total =              cJSON_GetObjectItem(root, "invoice_total");
        cJSON* invoice_date =               cJSON_GetObjectItem(root, "invoice_date");

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
                return;
            }
            ret = err->errorCode;
            free(err);
        }

        SQL_TIMESTAMP_STRUCT myTimestamp;
        stringToTimestamp(invoice_date->valuestring, &myTimestamp);

        addInvoice(
            _In_(bool)                     open_database,
            _In_(int)                      customer_id,
            _In_                           myTimestamp,
            _In_(char*)                    bankRef->valuestring,
            _In_(double)                   invoice_subtotal->valuedouble,
            _In_(double)                   invoice_tax->valuedouble,
            _In_(double)                   invoice_total->valuedouble,
            _Out_(int*)                    &invoice_id);

        cJSON* invoiceLines =       cJSON_GetObjectItem(root, "invoice_lines");

        int size =                  cJSON_GetArraySize(invoiceLines);

        cJSON* item = NULL;

        for (int index = 0; index < size; index++)
        {
            item =                  cJSON_GetArrayItem(invoiceLines, index);
            cJSON* product_name =   cJSON_GetObjectItem(item, "product_name");
            cJSON* quantity =       cJSON_GetObjectItem(item, "quantity");
            cJSON* price =          cJSON_GetObjectItem(item, "price");

            addInvoiceLine(
                _In_(bool)                   open_database,
                _In_(int)                    invoice_id,
                _In_(char*)                  product_name->valuestring, // invoiceline_product
                _In_(int)                    quantity->valueint,        // invoiceline_quantity
                _In_(double)                 price->valuedouble);       // invoiceline_price
        }

        if (open_database_general)
        {
            dbClose();
        }

        cJSON_Delete(root);
    }
}

