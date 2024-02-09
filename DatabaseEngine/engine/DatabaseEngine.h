#ifndef DATABASE_ENGINE_H
#define DATABASE_ENGINE_H

#include "DBEngineLibraryExports.h"

#include <Windows.h>
#include <sql.h>
#include <sqlext.h>

#define ERROR_INITIAL                        0
#define ERROR_CODE                          -1
#define ERROR_CONNECTION_STRING_UNAVAILABLE -2
#define ERROR_DBOPEN_FAILED                 -3

// Function declaration
#ifdef __cplusplus
extern "C" {
#endif

	int DB_ENGINE_LIBRARY_EXPORT dbOpen(
		_In_ char*					fileName);

	void DB_ENGINE_LIBRARY_EXPORT dbClose();

	void DB_ENGINE_LIBRARY_EXPORT createTables();

	void DB_ENGINE_LIBRARY_EXPORT addCustomer(
		_In_ char*					customer_firstName,
		_In_ char*					customer_lastName,
		_In_ char*					customer_address,
		_In_ char*					customer_zip,
		_In_ char*					customer_city,
		_Out_ int*					customer_id);

	int DB_ENGINE_LIBRARY_EXPORT free_json_data();

	void DB_ENGINE_LIBRARY_EXPORT queryInvoicesByCustomer(
		_In_ int					customer_id, 
		_Out_ char**				jsonString);

	void DB_ENGINE_LIBRARY_EXPORT addInvoiceLine(
		_In_ int					invoice_id,
		_In_ char*					invoiceline_product,
		_In_ int					invoiceline_quantity,
		_In_ double					invoiceline_price);

	void DB_ENGINE_LIBRARY_EXPORT addInvoice(
		_In_ int					customer_id,
		_In_ SQL_TIMESTAMP_STRUCT	invoice_date,
		_In_ char*					invoice_bankreference,
		_In_ double					invoice_subtotal,
		_In_ double					invoice_tax,
		_In_ double					invoice_total,
		_Out_ int*					invoice_idOut);

#ifdef __cplusplus
}
#endif

#endif // DATABASE_ENGINE_H