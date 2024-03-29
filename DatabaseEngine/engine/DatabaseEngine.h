#ifndef DATABASE_ENGINE_H
#define DATABASE_ENGINE_H

#include <Windows.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>
#include "DBEngineLibraryExports.h"
#include "SqlErrorUtil.h"
#include "cJSON.h"

#define ERROR_INITIAL                        0
#define ERROR_CODE                          -1
#define ERROR_CONNECTION_STRING_UNAVAILABLE -8
#define ERROR_WORKING_DIRECTORY_ERROR       -9
#define ERROR_DBOPEN_FAILED                 -3

#define true 1
#define false 0
#define bool int

node_t* internalErrorList = NULL;

enum FailedFunction { ErrFuncNone = 0, ErrFuncSQLAllocHandleA, ErrFuncSQLSetEnvAttrA, ErrFuncSQLAllocHandleB, ErrFuncSQLDriverConnectA, ErrFunc_getConnectionStringA, ErrFunc_getWorkingDirA };

typedef struct DBError {
	enum FailedFunction failedFunction;
	SQLRETURN			errorCode;
	int                 errorInt;

} DBERROR;

void DisplayError(SQLCHAR* sqlState, SQLINTEGER nativeError, SQLCHAR* message, SQLSMALLINT msgLen);

// Function declaration
#ifdef __cplusplus
extern "C" {
#endif

	int  DB_ENGINE_LIBRARY_EXPORT addNewInvoiceData(
		_In_ char*					invoicing_data_json, 
		_In_ int					length);

	void DB_ENGINE_LIBRARY_EXPORT createTables();

	void DB_ENGINE_LIBRARY_EXPORT addCustomer(
		_In_ char*					customer_firstName,
		_In_ char*					customer_lastName,
		_In_ char*					customer_address,
		_In_ char*					customer_zip,
		_In_ char*					customer_city,
		_In_ char*					customer_phone,
		_In_ char*					customer_email,

		_Out_ int*					customer_id);

	//void DB_ENGINE_LIBRARY_EXPORT updateCustomer(_In_ const char* customer_email);

	int  DB_ENGINE_LIBRARY_EXPORT getCustomer(
		_In_ int					customer_id,
		_Out_ cJSON**				customer_data);

	int  DB_ENGINE_LIBRARY_EXPORT getCustomerCharOut(
		_In_ int					customer_id,
		_Out_ char**				customer_data);

	void DB_ENGINE_LIBRARY_EXPORT queryCustomers(
		_Out_ char**				jsonString, 
		_Out_ node_t**				errorList);

	void DB_ENGINE_LIBRARY_EXPORT queryInvoiceById(
		_In_ int					invoice_id, 
		_Out_ char**				jsonString, 
		_Out_ node_t**				errorList);

	void DB_ENGINE_LIBRARY_EXPORT updateCustomer(
		_In_ int					customer_id, 
		_In_ const char*			customer_firstName, 
		_In_ const char*			customer_lastName,
		_In_ const char*			customer_address,
		_In_ const char*			customer_zip,
		_In_ const char*			customer_city,
		_In_ const char*			customer_phone,
		_In_ const char*			customer_email);

	int  DB_ENGINE_LIBRARY_EXPORT free_json_data(int selector);

	void DB_ENGINE_LIBRARY_EXPORT free_sql_error_details();

	void DB_ENGINE_LIBRARY_EXPORT queryInvoicesByCustomer(
		_In_ int					customer_id, 
		_Out_ char**				jsonString,
		_Out_ node_t**				errorList);

	void DB_ENGINE_LIBRARY_EXPORT addInvoiceLine(
		_In_ bool                   open_database,
		_In_ int                    invoice_id,
		//_In_ char* invoiceline_product,
		_In_ int                    product_item_id,
		_In_ int                    invoiceline_quantity,
		_In_ double                 invoiceline_price,
		_In_ char*					product_description);

	int DB_ENGINE_LIBRARY_EXPORT addInvoice(
		_In_ bool                   open_database,
		_In_ int                    customer_id,
		_In_ SQL_TIMESTAMP_STRUCT   invoice_date,
		_In_ char*					invoice_bankreference,
		_In_ double                 invoice_subtotal,
		_In_ double                 invoice_tax,
		_In_ double                 invoice_total,
		_In_ SQL_DATE_STRUCT        invoice_due_date,
		_Out_ int*					invoice_idOut,
		_Out_ node_t**				errorList);

#ifdef __cplusplus
}
#endif

	void					      dbOpen(
		_In_ char*				    fileName,
		_Out_ DBERROR**			    dbErr);

	void						  dbClose();

#endif // DATABASE_ENGINE_H