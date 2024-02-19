#include <stdio.h>
#include "DatabaseEngine.h"
#include "Utilities.h"

/**
* This data is global due to another function must be able to free the memory.
*/
char* json_data = NULL;

#define SQL_RESULT_LEN 240
#define SQL_RETURN_CODE_LEN 1000

SQLHENV henv;  // Environment handle
SQLHDBC hdbc;  // Connection handle
SQLRETURN ret; // Return code


SQLCHAR result[SQL_RESULT_LEN];
SQLCHAR retcode[SQL_RETURN_CODE_LEN];

/**
* This function opens the connection to a database using an ODBC driver.
* 
* @param fileName: A name of the file that must contain the connection string
* @return: An integer value indication a success or an error in the function.
*          Zero means that the function has succeeded
*/
int dbOpen(
    _In_ char* fileName) 
{
    int err = ERROR_INITIAL;

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
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
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
    int databaseOpen = dbOpen(fileName);

    if (!databaseOpen)
        return;

    // Allocate memory for JSON data
    int sizeofString = 0;
    json_data = (char*)malloc(sizeofString);  // Example: allocating 100 bytes
    if (json_data == NULL) {
        perror("Error allocating memory for JSON data");
        exit(EXIT_FAILURE);
    }
    json_data[0] = '\0';

    char startMarkOfJsonData[2] = "{";
    char endMarkOfJsonData[2] = "}";

    char commaOfJsonData[2] = ",";

    int success = concatToJsonData(&json_data, startMarkOfJsonData);

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

    // Process the rows and print to screen
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        //SQLINTEGER invoice_id;
        //SQL_TIMESTAMP_STRUCT ts;

        //SQLCHAR invoice_bankreference[20];

        int invoice_index = 0;

        do {
            SQLCHAR columnName[64];
            SQLSMALLINT columnNameLength;

            SQLRETURN ret = SQLDescribeCol(hstmt, 1, columnName, sizeof(columnName), &columnNameLength, NULL, NULL, NULL, NULL);

            if (strcmp((char*)columnName, "customer_id") == 0) {
                // Process the invoice data
                SQLINTEGER  customerId;
                SQLCHAR     firstName[64];
                SQLCHAR     lastName[64];
                SQLCHAR     address[64];
                SQLCHAR     zip[64];
                SQLCHAR     city[64];

                while (SQLFetch(hstmt) == SQL_SUCCESS) {
                    SQLGetData(hstmt, 1, SQL_C_SLONG,   &customerId,     0,                      NULL);
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

                    parseCustomerData(customerId,
                        firstName,
                        lastName,
                        address,
                        zip,
                        city,
                        &json_data);
                }
            }
            else if (strcmp((char*)columnName, "invoice_id") == 0) {
                // Process the invoice data
                SQLINTEGER invoiceId;
                SQLCHAR bankReference[64];
                while (SQLFetch(hstmt) == SQL_SUCCESS) {
                    SQLGetData(hstmt, 1, SQL_C_SLONG, &invoiceId, 0, NULL);
                    SQLGetData(hstmt, 4, SQL_C_CHAR, bankReference, sizeof(bankReference), NULL);
                    printf("Invoice ID: %d, Bank Reference: %s\n", invoiceId, bankReference);

                    // Allocate memory for JSON data
                    int sizeofString = 0;
                    char* temp_data = (char*)malloc(sizeofString);  // Example: allocating 100 bytes
                    if (temp_data == NULL) {
                        perror("Error allocating memory for JSON data");
                        exit(EXIT_FAILURE);
                    }
                    temp_data[0] = '\0';
                    
                    int index = -1;
                    int place = -1;
                    if (invoice_index == 0)
                    {
                        index = find_index_of_invoices_opening_bracket(json_data);
                        place = index + 1;
                        success = concatToJsonData(&temp_data, startMarkOfJsonData);
                    }
                    else
                    {
                        index = find_index_of_invoices_closing_bracket(json_data);
                        place = index;
                        success = concatToJsonData(&temp_data, commaOfJsonData);
                    }
                    invoice_index++;

                    parseInvoiceData(customer_id, invoiceId, bankReference, &temp_data);

                    success = concatToJsonData(&temp_data, endMarkOfJsonData);

                    insert_string_safely(&json_data, temp_data, place);
                    free(temp_data);
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

    success = concatToJsonData(&json_data, endMarkOfJsonData);

    *jsonString = json_data;

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
    _In_ int invoice_id, 
    _In_ char* invoiceline_product,
    _In_ int invoiceline_quantity,
    _In_ double invoiceline_price)
{
    char fileName[21] = "connectionstring.txt";
    int databaseOpen = dbOpen(fileName);

    if (!databaseOpen)
        return;


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
    dbClose();
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
    *invoice_idOut = 0;

    char fileName[21] = "connectionstring.txt";
    int databaseOpen = dbOpen(fileName);

    if (!databaseOpen)
        return;

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
    dbClose();
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
    int databaseOpen = dbOpen(fileName);

    if (!databaseOpen)
        return;

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
*
*/
void newInvoice()
{
    int customer_id = 0;
    SQL_TIMESTAMP_STRUCT invoice_date;
    char* invoice_bankreference = "";
    double invoice_subtotal;
    double invoice_tax = 0.00;
    double invoice_total = 0.00;
    int* invoice_idOut = NULL;

    addInvoice(
        _In_(int)                   customer_id,
        _In_                        invoice_date,
        _In_(char*)                 invoice_bankreference,
        _In_(double)                invoice_subtotal,
        _In_(double)                invoice_tax,
        _In_(double)                invoice_total,
        _Out_(int*)                 invoice_idOut);
}

/**
* This functions adds three tables into a database.
* The tables are: a customer, an invoice and an invoice_line tables.
*/
void createTables()
{
    char fileName[21] = "connectionstring.txt";
    int databaseOpen = dbOpen(fileName);

    if (!databaseOpen)
        return;

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
