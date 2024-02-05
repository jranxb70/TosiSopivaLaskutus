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

	int DB_ENGINE_LIBRARY_EXPORT dbOpen(_In_ char* fileName);
	void DB_ENGINE_LIBRARY_EXPORT dbClose();

	void DB_ENGINE_LIBRARY_EXPORT createTables();

	void DB_ENGINE_LIBRARY_EXPORT addCustomer(
		char* customer_firstName, 
		char* customer_lastName, 
		char* customer_address, 
		char* customer_zip, 
		char* customer_city, 
		int* customer_id);

	void DB_ENGINE_LIBRARY_EXPORT queryInvoicesByCustomer(
		int customer_id);

	void DB_ENGINE_LIBRARY_EXPORT addInvoiceLine(
		int invoice_id,
		char* invoiceline_product,
		int invoiceline_quantity,
		double invoiceline_price);

	void DB_ENGINE_LIBRARY_EXPORT addInvoice(
		int customer_id,
		SQL_TIMESTAMP_STRUCT invoice_date,
		char* invoice_bankreference,
		double               invoice_subtotal,
		double               invoice_tax,
		double               invoice_total,
		int*                 invoice_idOut);

#ifdef __cplusplus
}
#endif

	_Success_(return == 0)
	int getWorkingDir(
		_Out_ char** pWorkingDir);

	int getConnectionString(
		_Inout_ char** workingDirectory, 
		_In_ char* fileName, 
		_Out_ char** connectionString);

	int readFile(
		_In_ char** workingDirectory, 
		_Out_ char** connectionString);

#endif // DATABASE_ENGINE_H