// A simple program that computes the square root of a number
#include <stdio.h>
#include <stdlib.h>

#include "engine/cJSON.h"
#include "engine/DatabaseEngine.h"
#include "engine/Utilities.h"
#include "BankReferenceCalculator.h"

void queryInvoiceByInvoiceId();

void queryInvoiceByInvoiceId()
{
    char* jsonString = NULL;
    int invoice_id = 1;
    node_t* errs = NULL;
    queryInvoiceById(invoice_id, &jsonString, &errs);
    int succeeded = free_json_data();
    free_sql_error_details();

}

void testEAN()
{
    char lastName[14] = "6438460761591";
    char* data = NULL;
    queryProductItemByEAN(lastName, &data);
    
    int nakki = 0;
    free_json_data();
    nakki++;
}

void testEAN2()
{
    char* fileName = "connectionstring.txt";
    char* dbErrMsg = NULL;
    dbOpenCloseTest(
        _In_(char*) fileName,
        _Out_(char**) & dbErrMsg);
    free(dbErrMsg);

    char lastName[14] = "6438460761591";
    char* data = NULL;
    queryProductItemByEAN(lastName, &data);

    int nakki = 0;
    free_json_data();
    nakki++;
}

void testUpdateForCustomer()
{
    int id = 1;
    char firstName[50] = "Testi";
    char lastName[50] = "Kommunisti";
    char address[50] = "Huhtalantie 50";
    char zip[50] = "60200";

    char phone[20] = "964-122089";
    char email[100] = "testi.kommunisti@kommarit.fi";

    char city[50] = "Ilmajoki";

    updateCustomer(
        _In_ (int) id,
        _In_ (char*) firstName,
        _In_ (char*) lastName,
        _In_ (char*) address,
        _In_ (char*) zip,
        _In_ (char*) city,
        _In_ (char*) phone,
        _In_ (char*) email);
}


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stdout, "%s\n", "Insufficient input - enter a value.");
        return 1;
    }

    testEAN();
    return 0;

    int del = updateInvoice(311, 1, "2024-03-01 0:00:00.0000000", "12783", 0.00, 0.00, 0.00, "2024-03-15", 0.00);
    
    int d = deleteInvoice(52);

    char* jsonStringCompany = NULL;
    int company_id = 1;
    getCompany(_In_ (int) company_id, _Out_ (char**) &jsonStringCompany);

    free_json_data();

    char* invoicesS = NULL;
    queryInvoices(3, "2024-03-01 0:00:00.0000000", "2024-03-31 23:59:59.9999999", DESCENDING, &invoicesS);

    char* tuppu = "tuppu";
    char* user_password = "tumpeloitse";
    int granted = getDBUser(_In_ (char*) tuppu, _In_ (char*) user_password);

    int resultOf = free_json_data();

    char* user = "artistimaksaa";
    char* passwd = "kakkosnelonen";
    char* emailU = "masa@aaltoyliopisto.fi";
    
    int result = addDBUser(user, passwd, emailU);


    char* array = "\xc3\x85gren";

    char* array2 = malloc(sizeof(char) * 7); // Allocate memory for array2

    // Convert hexadecimal representation to normal string
    sprintf(array2, "%s", array);

    deleteCustomer(
        _In_ (long)					10);
    return 0;
    
    char* decodedCharArray = NULL;
    decodeUTF8Encoding(array, &decodedCharArray);
    printf("decodedCharArray: %s", decodedCharArray);

    free(decodedCharArray);
    testUpdateForCustomer();

    char* jsonX = NULL;
    node_t* errsX = NULL;
    queryCustomers(&jsonX, &errsX);
    int succeededX = free_json_data();
    free_sql_error_details();

    free(jsonX);


    queryInvoiceByInvoiceId();

    printf("%s\n", argv[0]);
    printf("%s\n", argv[1]);

    char firstName[50] = "Testi";
    char lastName[50]  = "Kommunisti";
    char address[50]   = "Huhtalantie 50";
    char zip[50]       = "60200";
    char city[50]      = "Seinäjoki";

    char phone[20]     = "964-120089";
    char email[100]    = "testi.kommunisti@kommarit.fi";

    int customer_id = 1;
    int invoice_id = -1;

    const SQL_TIMESTAMP_STRUCT invoice_date = {2017, 1, 1, 12, 12, 0, 0};

    char* invoice_date_str = convertTimestampToString(&invoice_date);
    char* invoice_due_date_str = calculateInvoiceDueDate(&invoice_date);

    int bankreference = 1000;
    double invoice_subtotal = 0.0;
    double invoice_tax = 0.00;
    double invoice_total = 0.00;

    char* product_description = "Vaalea Lager 0,5l 5,3%";
    int product_item_id = 1;
    int quantity = 10;
    double price = 0.84;

    char* product_description2 = "Vaalea Lager 0,33l 4,6%";
    int product_item_id2 = 2;
    int quantity2 = 8;
    double price2 = 1.64;

    char* product_description3 = "Makea siideri 0,33 4,6%";
    int product_item_id3 = 3;
    int quantity3 = 6;
    double price3 = 2.04;

    char* customer_data_1st = NULL;
    int e = getCustomerCharOut(1, &customer_data_1st);

    int success = free_json_data();

    //free(customer_data_1st);

    cJSON* customer_data = NULL;
    int err = getCustomer(1, &customer_data);

    addCustomer(firstName, lastName, address, zip, city, phone, email, &customer_id);

    bankreference = bankreference + customer_id;
    int bankreferenceOut = 0;
    int* ref = NULL;
    ref = &bankreferenceOut;
    calcNewReference(bankreference, ref);

    char invoice_bankreference[20]; // Buffer big enough for a 32-bit number

    snprintf(invoice_bankreference, sizeof(invoice_bankreference), "%d", bankreferenceOut);

    invoice_subtotal = quantity * price + quantity2 * price2 + quantity3 * price3;
    invoice_total = invoice_subtotal * 1.25;
    invoice_tax = invoice_total - invoice_subtotal;

    //addInvoice(customer_id, invoice_date, invoice_bankreference, invoice_subtotal, invoice_tax, invoice_total, &invoice_id);

    //addInvoiceLine(invoice_id, product, quantity, price);
    //addInvoiceLine(invoice_id, product2, quantity2, price2);
    //addInvoiceLine(invoice_id, product3, quantity3, price3);

    char* json = NULL;
    node_t* errs = NULL;
    queryInvoicesByCustomer(customer_id, &json, &errs);
    int succeeded = free_json_data();
    free_sql_error_details();

    free(json);

    char sample_json[2048];

    snprintf(sample_json, sizeof(sample_json), "{\"customer_id\": %d, \"invoice_date\": \"%s\", \"invoice_subtotal\": %f, \
                                                 \"invoice_total\": %f, \"invoice_tax\": %f, \"bank_reference\" : \"%s\", \"invoice_due_date\": \"%s\",\
                                                 \"invoice_lines\" : [{\"product_item_id\": %d, \"quantity\": %d, \"price\": %f, \"product_description\": \"%s\"}, \
                                                                      {\"product_item_id\": %d, \"quantity\": %d, \"price\": %f, \"product_description\": \"%s\"}, \
                                                                      {\"product_item_id\": %d, \"quantity\": %d, \"price\": %f, \"product_description\": \"%s\"}]}", 
                                                    customer_id, invoice_date_str, invoice_subtotal, invoice_total, invoice_tax, invoice_bankreference, invoice_due_date_str,
                                                    product_item_id, quantity, price, product_description, product_item_id2, quantity2, price2, product_description2, product_item_id3, quantity3, price3, product_description3);

    int len = strlen(sample_json);


    addNewInvoiceData(sample_json, len);

    printf("%s", cJSON_Print(customer_data));

    /* free all objects under root and root itself */
    cJSON_Delete(customer_data);
    free(invoice_date_str);

    return 0;
}
