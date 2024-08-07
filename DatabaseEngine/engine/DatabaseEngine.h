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

#define ASCENDING                       1
#define DESCENDING                      -1

typedef long long int largeint;

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

	void DB_ENGINE_LIBRARY_EXPORT dbOpenCloseTest(
		_In_ char*   fileName,
		_Out_ char** dbErrMsg);

	void DB_ENGINE_LIBRARY_EXPORT queryProductItemByEAN(
		_In_  char*  ean, 
		_Out_ char** json_output);

	void DB_ENGINE_LIBRARY_EXPORT fetchCustomerData(
		_In_ char*   inputParam,
		_In_ int     isAPhoneNumber,
		_Out_ char** json_output);

	int DB_ENGINE_LIBRARY_EXPORT addDBUser(
		_In_ char*					login,
		_In_ char*					password,
		_In_ char*					email);

	int DB_ENGINE_LIBRARY_EXPORT getDBUser(
		_In_ char*					login,
		_In_ char*					user_password);

	void DB_ENGINE_LIBRARY_EXPORT addCompany(
		_In_  const char* company_name,
		_In_  const char* company_address,
		_In_  const char* company_zip,
		_In_  const char* company_city,
		_In_  const char* company_phone,
		_In_  const char* company_business_id,
		_Out_ SQLINTEGER* company_id);

	void DB_ENGINE_LIBRARY_EXPORT addCompanyFromJson(
		_In_  const char* companyJson,
		_Out_ SQLINTEGER* company_id);

	void DB_ENGINE_LIBRARY_EXPORT getCompany(
		_In_ int company_id, 
		_Out_ char** jsonStringCompany);

	SQLRETURN DB_ENGINE_LIBRARY_EXPORT updateCompany(
		_In_ int id, 
		_In_ char* company_name, 
		_In_ char* company_address,
		_In_ char* company_zip, 
		_In_ char* company_city, 
		_In_ char* company_phone,
		_In_ char* company_email,
		_In_ char* company_business_id);

	int  DB_ENGINE_LIBRARY_EXPORT addNewInvoiceData(
		_In_ char*					invoicing_data_json, 
		_In_ int					length);

	int UpdateBankReference(long invoice_id);

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


	void DB_ENGINE_LIBRARY_EXPORT addCustomerFromJson(
		_In_ const char* customerJson, 
		_Out_ SQLINTEGER* customer_id);

	int  DB_ENGINE_LIBRARY_EXPORT deleteCustomer(
		_In_ long					customer_id);

	int DB_ENGINE_LIBRARY_EXPORT updateInvoice(
		_In_ int					invoice_id,
		_In_ int					customer_id,
		_In_ char*					invoice_date,
		_In_ char*					invoice_bankreference,
		_In_ largeint			    invoice_subtotal,
		_In_ largeint				invoice_tax,
		_In_ largeint				invoice_total,
		_In_ char*					invoice_due_date,
		_In_ largeint				invoice_outstanding_balance);

	int DB_ENGINE_LIBRARY_EXPORT updateInvoiceFromJson(_In_ char* invoice_json_data, _Out_ char** error_msg);

	int  DB_ENGINE_LIBRARY_EXPORT deleteInvoice(
		_In_ long					invoice_id);

	int  DB_ENGINE_LIBRARY_EXPORT getCustomer(
		_In_ int					customer_id,
		_Out_ cJSON**				customer_data);

	int  DB_ENGINE_LIBRARY_EXPORT getCustomerCharOut(
		_In_ int					customer_id,
		_Out_ char**				customer_data);

	void DB_ENGINE_LIBRARY_EXPORT queryCustomersAsJson(
		_Out_ char**				jsonString, 
		_Out_ node_t**				errorList);

	void DB_ENGINE_LIBRARY_EXPORT queryInvoiceById(
		_In_ int					invoice_id, 
		_Out_ char**				jsonString, 
		_Out_ node_t**				errorList);

	int  DB_ENGINE_LIBRARY_EXPORT queryInvoices(
		_In_ long					procudere_switch, 
		_In_ char*					start_date, 
		_In_ char*					end_date, 
		_In_ long					sorting, 
		_Out_ char**				jsonString);

	void DB_ENGINE_LIBRARY_EXPORT updateCustomer(
		_In_ int					customer_id, 
		_In_ char*					customer_firstName, 
		_In_ char*					customer_lastName,
		_In_ char*					customer_address,
		_In_ char*					customer_zip,
		_In_ char*					customer_city,
		_In_ char*					customer_phone,
		_In_ char*					customer_email);

	int  DB_ENGINE_LIBRARY_EXPORT free_json_data();

	int DB_ENGINE_LIBRARY_EXPORT free_error_message();

	void DB_ENGINE_LIBRARY_EXPORT free_sql_error_details();

	void DB_ENGINE_LIBRARY_EXPORT queryInvoicesByCustomer(
		_In_ int					customer_id, 
		_Out_ char**				jsonString,
		_Out_ node_t**				errorList);

	void DB_ENGINE_LIBRARY_EXPORT addInvoiceLine(
		_In_ bool                   open_database,
		_In_ int                    invoice_id,
		_In_ int                    product_item_id,
		_In_ int                    invoiceline_quantity,
		_In_ largeint               invoiceline_price,
		_In_ char*					product_description);

	int DB_ENGINE_LIBRARY_EXPORT addInvoice(
		_In_ bool                   open_database,
		_In_ int                    customer_id,
		_In_ SQL_TIMESTAMP_STRUCT   invoice_date,
		_In_ char*					invoice_bankreference,
		_In_ largeint               invoice_subtotal,
		_In_ largeint               invoice_tax,
		_In_ largeint               invoice_total,
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