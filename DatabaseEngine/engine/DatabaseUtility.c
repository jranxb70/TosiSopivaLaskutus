#include "DatabaseUtility.h"

int fetchInvoiceLineDataAsJson(SQLHSTMT* hstmtP, cJSON** rtt)
{
    SQLHSTMT hstmt = *hstmtP;
    cJSON* root = *rtt;

    SQLINTEGER lineId;
    SQLINTEGER invoiceId;
    SQLINTEGER productItemId;
    SQLINTEGER quantity;
    SQLDOUBLE price;

    SQLCHAR productDescription[1024];

    while (SQLFetch(hstmt) == SQL_SUCCESS) 
    {
        char* a = cJSON_Print(root);
        SQLLEN productDescriptionLen;

        SQLGetData(hstmt, 1, SQL_C_SLONG, &lineId, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_SLONG, &invoiceId, 0, NULL);
        SQLGetData(hstmt, 3, SQL_C_SLONG, &productItemId, 0, NULL);
        SQLGetData(hstmt, 4, SQL_C_SLONG, &quantity, 0, NULL);
        SQLGetData(hstmt, 5, SQL_C_DOUBLE, &price, 0, NULL);
        SQLGetData(hstmt, 6, SQL_C_CHAR, productDescription, sizeof(productDescription), &productDescriptionLen);

        ///////////////////////////////////////////

        cJSON* invoice_line = cJSON_CreateObject();
        cJSON_AddNumberToObject(invoice_line, "invoice_line_id", lineId);
        cJSON_AddNumberToObject(invoice_line, "product_item_id", productItemId);

        cJSON_AddNumberToObject(invoice_line, "quantity", quantity);
        cJSON_AddNumberToObject(invoice_line, "price", price);

        if (productDescriptionLen == SQL_NULL_DATA)
        {
            cJSON_AddStringToObject(invoice_line, "product_description", "N/A");
        }
        else
        {
            cJSON_AddStringToObject(invoice_line, "product_description", productDescription);
        }

        // If invoices is null we are (probably) dealing with queryInvoiceById
        cJSON* invoices = cJSON_GetObjectItem(root, "invoices");
        int arraySizeInvoices = -1;
        cJSON* invoice_lines = NULL;
        if (!invoices)
        {
            invoice_lines = cJSON_GetObjectItem(root, "invoice_lines");
            cJSON_AddItemReferenceToArray(invoice_lines, invoice_line);
        }
        else
        {
            arraySizeInvoices = cJSON_GetArraySize(invoices);
        }

        cJSON* invoice = NULL;
        for (int ind = 0; ind < arraySizeInvoices; ind++)
        {
            invoice = cJSON_GetArrayItem(invoices, ind);
            cJSON* it = cJSON_GetObjectItem(invoice, "invoice_id");
            double wtf = cJSON_GetNumberValue(it);
            if (invoiceId == (int)wtf)
            {
                cJSON* invoice_lines = cJSON_GetObjectItem(invoice, "invoice_lines");
                cJSON_AddItemReferenceToArray(invoice_lines, invoice_line);
                break;
            }
        }
    }
    return 0;
}
