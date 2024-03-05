// A simple program that computes the square root of a number
#include <stdio.h>
#include <stdlib.h>

#include "engine/cJSON.h"
#include "engine/DatabaseEngine.h"
#include "engine/Utilities.h"
#include "BankReferenceCalculator.h"


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stdout, "%s\n", "Insufficient input - enter a value.");
        return 1;
    }

    printf("%s\n", argv[0]);
    printf("%s\n", argv[1]);

    char firstName[50] = "Testi";
    char lastName[50] = "Kommunisti";
    char address[50] = "Huhtalantie 50";
    char zip[50] = "60200";
    char city[50] = "Seinäjoki";

    int customer_id = 1;
    int invoice_id = -1;

    const SQL_TIMESTAMP_STRUCT invoice_date = {2017, 1, 1, 12, 12, 0, 0};

    char* invoice_date_str = convertTimestampToString(&invoice_date);

    int bankreference = 1000;
    double invoice_subtotal = 0.0;
    double invoice_tax = 0.00;
    double invoice_total = 0.00;

    char* product = "kalja";
    int quantity = 10;
    double price = 0.84;

    char* product2 = "siideri";
    int quantity2 = 8;
    double price2 = 1.64;

    char* product3 = "lonkero";
    int quantity3 = 6;
    double price3 = 2.04;

    char* customer_data_1st = NULL;
    int e = getCustomerCharOut(1, &customer_data_1st);

    free(customer_data_1st);

    cJSON* customer_data = NULL;
    int err = getCustomer(1, &customer_data);

    //addCustomer(firstName, lastName, address, zip, city, &customer_id);

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
                                                 \"invoice_total\": %f, \"invoice_tax\": %f, \"bank_reference\" : \"%s\", \
                                                 \"invoice_lines\" : [{\"product_name\": \"%s\", \"quantity\": %d, \"price\": %f}, \
                                                                      {\"product_name\": \"%s\", \"quantity\": %d, \"price\": %f}, \
                                                                      {\"product_name\": \"%s\", \"quantity\": %d, \"price\": %f}]}", 
                                                    customer_id, invoice_date_str, invoice_subtotal, invoice_total, invoice_tax, invoice_bankreference, 
                                                    product, quantity, price, product2, quantity2, price2, product3, quantity3, price3);

    int len = strlen(sample_json);


    addNewInvoiceData(sample_json, len);

    printf("%s", cJSON_Print(customer_data));

    /* free all objects under root and root itself */
    cJSON_Delete(customer_data);
    free(invoice_date_str);

    return 0;
}
