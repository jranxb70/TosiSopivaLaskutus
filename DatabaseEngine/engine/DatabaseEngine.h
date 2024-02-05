#ifndef DATABASE_ENGINE_H
#define DATABASE_ENGINE_H

#include "DBEngineLibraryExports.h"

#include <Windows.h>
#include <sql.h>
#include <sqlext.h>

#define ERROR_CODE -1

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

	void DB_ENGINE_LIBRARY_EXPORT queryInvoicesByCustomer(
		_In_ int					customer_id);

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

	_Success_(return == 0)
	int getWorkingDir(
		_Out_ char**				pWorkingDir);

	int getConnectionString(
		_Inout_ char**				workingDirectory, 
		_In_ char*					fileName, 
		_Out_ char**				connectionString);

	int readFile(
		_In_ char**					workingDirectory, 
		_Out_ char**				connectionString);

#endif // DATABASE_ENGINE_H