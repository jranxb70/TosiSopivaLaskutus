#include <io.h>
#include <time.h>
#include <errno.h>
#include "Utilities.h"

char* globalWorkingDir = NULL;
char* globalConnectionString = NULL;

unsigned long roundToNextHundreds(unsigned long num) 
{

    //for ()
    // Convert the unsigned long to a string
    char str[20]; // Adjust the size as needed
    snprintf(str, sizeof(str), "%lu", num);
    size_t s = strlen(str);

    int denominator = 1000;

    if (num % denominator == 0)
    {
        return num;
    }

    // Extract the last two digits
    unsigned long lastTwoDigits = num % 100;

    // Check if the last two digits are nonzero
    if (lastTwoDigits != 0) 
    {
        if (!(lastTwoDigits < 50))
        {
            // Calculate the amount needed to reach the next full hundreds
            unsigned long increment = 100 - lastTwoDigits;

            // Round up to the next full hundreds
                num += increment;
        }
        else
        {
            num -= lastTwoDigits;
        }
    }

    return num;
}

int stringToDate(const char* inputString, SQL_DATE_STRUCT* invoiceDueDate) 
{ 
    int year, month, day;
    int result = sscanf(inputString, "%d-%d-%d", &year, &month, &day);
    invoiceDueDate->year = year;
    invoiceDueDate->month = month;
    invoiceDueDate->day = day;
    return result; 
}

// Function to convert a string to SQL_TIMESTAMP_STRUCT
int stringToTimestamp(const char* inputString, SQL_TIMESTAMP_STRUCT* timestamp) 
{
    int year, month, day, hour, minute, second, f;
    int result = sscanf(inputString, "%d-%d-%d %d:%d:%d.%d", &year, &month, &day, &hour, &minute, &second, &f);

    timestamp->year = year;
    timestamp->month = month;
    timestamp->day = day;
    timestamp->hour = hour;
    timestamp->minute = minute;
    timestamp->second = second;
    if (f > 0)
    {
        long rounded_f = roundToNextHundreds(f);
        timestamp->fraction = rounded_f; // Set fractional seconds
    }
    else
    {
        timestamp->fraction = f;
    }
    return result;
}

char* calculateInvoiceDueDate(const SQL_TIMESTAMP_STRUCT* timestamp)
{
    SQL_DATE_STRUCT due_date;
    due_date.month = timestamp->month + 1;
    due_date.year = timestamp->year;
    due_date.day = timestamp->day;

    char* formattedDueDate = (char*)malloc(20);
    if (formattedDueDate)
    {
        sprintf(formattedDueDate, "%04d-%02d-%02d", due_date.year, due_date.month, due_date.day);
    }
    return formattedDueDate;
}

char* convertTimestampToString(const SQL_TIMESTAMP_STRUCT* timestamp) 
{
   // Create a tm struct for time manipulation
   struct tm timeInfo = {
        .tm_year = timestamp->year - 1900,
        .tm_mon = timestamp->month - 1,
        .tm_mday = timestamp->day,
        .tm_hour = timestamp->hour,
        .tm_min = timestamp->minute,
        .tm_sec = timestamp->second
    };

    // Convert to time_t (seconds since epoch)
    time_t rawTime = mktime(&timeInfo);

    // Format the timestamp string
    char* formattedTimestamp = (char*)malloc(30); // Adjust size as needed
    if (formattedTimestamp)
    {
        strftime(formattedTimestamp, 30, "%Y-%m-%d %H:%M:%S", localtime(&rawTime));

        // Append the fraction part (milliseconds) if available
        if (timestamp->fraction > 0) {
            snprintf(formattedTimestamp + 19, 12, ".%06d", timestamp->fraction); // Microseconds
        }
        else
        {
            snprintf(formattedTimestamp + 19, 12, ".%06d", 0); // Microseconds
        }
    }

    return formattedTimestamp;
}

void save_to_file(const char* content, const char* filename) {
    FILE* file = fopen(filename, "w");  // Open the file for writing

    if (file != NULL) {
        // Write the content to the file
        fprintf(file, "%s", content);
        fclose(file);  // Close the file
        printf("Content saved to %s\n", filename);
    }
    else {
        printf("Error opening file %s\n", filename);
    }
}

/**
* This function returns a path to the working directory.
*
* @param pWorkingDir: A path to the working directory of the application
* @return: An integer value indication a success or an error in the function
*/
_Success_(return == 0)
int getWorkingDir(_Out_ char** pWorkingDir)
{
    int ret = 0;
    char cwd[1024] = "";
    *pWorkingDir = NULL;

    if (_getcwd(cwd, sizeof(cwd)) != 0) 
    {
        printf("Current working directory: **%s**\n", cwd);
    }
    else 
    {
        perror("_getcwd() error");
        ret = ERROR_UTILITY;
        goto error;
    }

    size_t count = strlen(cwd);

    if (NULL == (*pWorkingDir = (char*)malloc(sizeof(char) * count + 1)))
    {
        ret = ERROR_UTILITY;
        goto error;
    }

#pragma warning( push )
#pragma warning( disable : 6387 )

    strcpy_s(*pWorkingDir, (count + 1), cwd);
    globalWorkingDir = *pWorkingDir;
    return ret;

#pragma warning( pop )

error:
    free(*pWorkingDir);
    return ret;
}

int freeGlobalVariable(_In_ int selector)
{
    int freedVariable = -1;
    if (selector == 1)
    {
        if (globalWorkingDir)
        {
            free(globalWorkingDir);
            globalWorkingDir = NULL;
            freedVariable = selector;
            return freedVariable;
        }
    }
    else if (selector == 2)
    {
        if (globalConnectionString)
        {
            free(globalConnectionString);
            globalConnectionString = NULL;
            freedVariable = selector;
            return freedVariable;
        }
    }
    return freedVariable;
}

/**
* This function returns a connection string.
*
* @param workingDirectory: A path to the working directory of the application
* @param fileName: A name of the file that must contain the connection string
* @param connectionString: A connection string
* @return: An integer value indication a success or an error in the function
*/
int getConnectionString(
    _Inout_ char** workingDirectory,
    _In_ char* fileName,
    _Out_ char** connectionString)
{
    int ret = 0;

    *connectionString = NULL;

    size_t lenPath = strlen(*workingDirectory) + 2;
    size_t lenFileName = strlen(fileName);

    size_t lenFullPath = lenPath + lenFileName;

    char* temp = NULL;

    if (!!*workingDirectory)
    {
        if (NULL == (temp = (char*)realloc(*workingDirectory, lenPath)))
        {
            ret = ERROR_UTILITY;
            goto error;
        }
        *workingDirectory = temp;
        temp = NULL;

        size_t s = strlen(*workingDirectory);

        strcat_s(*workingDirectory, s + 2, "\\");

        if (NULL == (temp = (char*)realloc(*workingDirectory, lenFullPath + 1)))
        {
            ret = ERROR_UTILITY;
            goto error;
        }

        *workingDirectory = temp;
        globalWorkingDir = *workingDirectory;
        temp = NULL;

        s = strlen(*workingDirectory);

        size_t ss = strlen(fileName);

        strcat_s(*workingDirectory, s + ss + 1, fileName);
    }

    ret = readFile(workingDirectory, connectionString);
    globalConnectionString = *connectionString;

    return ret;

error:
    free(*workingDirectory);
    *workingDirectory = NULL;
    return ret;
}

/**
* This function concats a string to another string. The memory is reallocated according to a additional
* space the 
*/
int concatToJsonData(_Inout_ char** dest, _In_ const char* source)
{
    size_t max = 1024;

    size_t destLen = strnlen_s(*dest, max);
    size_t srcLen = strnlen_s(source, max);

    size_t newLength = destLen + srcLen + 1;

    char* invoicesTemp = (char*)realloc(*dest, newLength);

    if (!invoicesTemp)
    {
        printf("Memory reallocation failed\n");
        free(*dest); // Free the original memory
        *dest = NULL;
        dest = NULL;
        return -1;
    }

    *dest = invoicesTemp;

    if (destLen == 0)
    {
        // Copy the source string to the destination buffer safely
        if (strncpy_s(*dest, newLength, source, _TRUNCATE) != 0) {
            printf("String copy failed\n");
            free(*dest);
            return -2;
        }
        else
        {
            (*dest)[destLen + srcLen] = '\0';
            return 0;
        }
    }
    else
    {
        strcat_s(*dest, newLength, source);
        (*dest)[destLen + srcLen] = '\0';
    }
    return 0;
}

/**
*
*/
int parseCustomerData(
    _In_ int customer_id,
    _In_ char* first_name,
    _In_ char* last_name,
    _In_ char* address,
    _In_ char* zip,
    _In_ char* city,
    _Inout_ char** dest)
{
    char key1[16] = "\"customer_id\": ";
    key1[15] = '\0';
    char customer_id_value_as_str[20];

    int success = concatToJsonData(dest, key1);

    // Convert integer to string
    snprintf(customer_id_value_as_str, sizeof(customer_id_value_as_str), "%d", customer_id);

    success = concatToJsonData(dest, customer_id_value_as_str);

    char key2[17] = ", \"first_name\": ";
    key2[16] = '\0';
    success = concatToJsonData(dest, key2);
    success = concatToJsonData(dest, "\"");
    success = concatToJsonData(dest, first_name);
    success = concatToJsonData(dest, "\"");

    char key3[16] = ", \"last_name\": ";
    key3[15] = '\0';
    success = concatToJsonData(dest, key3);
    success = concatToJsonData(dest, "\"");
    success = concatToJsonData(dest, last_name);
    success = concatToJsonData(dest, "\"");

    char key4[14] = ", \"address\": ";
    key4[13] = '\0';
    success = concatToJsonData(dest, key4);
    success = concatToJsonData(dest, "\"");
    success = concatToJsonData(dest, address);
    success = concatToJsonData(dest, "\"");

    char key5[10] = ", \"zip\": ";
    key5[9] = '\0';
    success = concatToJsonData(dest, key5);
    success = concatToJsonData(dest, "\"");
    success = concatToJsonData(dest, zip);
    success = concatToJsonData(dest, "\"");

    char key6[11] = ", \"city\": ";
    key6[10] = '\0';
    success = concatToJsonData(dest, key6);
    success = concatToJsonData(dest, "\"");
    success = concatToJsonData(dest, city);
    success = concatToJsonData(dest, "\"");

    char key7[15] = ", \"invoices\": ";
    key7[14] = '\0';
    success = concatToJsonData(dest, key7);
    success = concatToJsonData(dest, "[]");

    return success;
}


/**
* This function ...
*
* @param customer_id: ...
*/
int parseInvoiceData(_In_ int customer_id, _In_ int invoice_id, _In_ char* bank_reference, _Inout_ char** dest)
{
    char key1[15] = "\"invoice_id\": ";
    key1[14] = '\0';
    char invoice_id_value_as_str[20];

    int success = concatToJsonData(dest, key1);

    // Convert integer to string
    snprintf(invoice_id_value_as_str, sizeof(invoice_id_value_as_str), "%d", invoice_id);

    success = concatToJsonData(dest, invoice_id_value_as_str);


    char key2[22] = ", \"bank_reference\": ";
    success = concatToJsonData(dest, key2);

    char quote[2] = "\"";
    success = concatToJsonData(dest, quote);
    success = concatToJsonData(dest, bank_reference);
    success = concatToJsonData(dest, quote);

    char key7[20] = ", \"invoice_lines\": ";
    key7[19] = '\0';
    success = concatToJsonData(dest, key7);
    success = concatToJsonData(dest, "[]");

    return success;
}

/**
* This function ...
*
* @param customer_id: ...
*/
int parseInvoiceLineData(_In_ int invoice_id, _In_ char* product_name, _Inout_ char** dest)
{
    char key1[20] = "\"invoice_line_id\": ";
    key1[19] = '\0';
    char invoice_line_id_value_as_str[20];

    int success = concatToJsonData(dest, key1);

    // Convert integer to string
    snprintf(invoice_line_id_value_as_str, sizeof(invoice_line_id_value_as_str), "%d", invoice_id);

    success = concatToJsonData(dest, invoice_line_id_value_as_str);


    char key2[19] = ", \"product_name\": ";
    key2[18] = '\0';
    success = concatToJsonData(dest, key2);

    char quote[2] = "\"";
    success = concatToJsonData(dest, quote);
    success = concatToJsonData(dest, product_name);
    success = concatToJsonData(dest, quote);

    return success;
}

void convertIntArrayToUnsignedCharArray(_In_ int* intArray, _Out_ unsigned char** charArray, _In_ int length) 
{
    *charArray = (unsigned char*)malloc(length * sizeof(unsigned char));
    if (*charArray == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    for (int i = 0; i < length; i++) {
        (*charArray)[i] = (unsigned char)(intArray[i] % 256);
    }
}

/**
* This function reads the content of a file in the file system.
*
* @param workingDirectory: A path to the working directory of the application
* @param connectionString: A connection string
* @return: An integer value indication a success or an error in the function
*/
int readFile(
    _In_ char** workingDirectory,
    _Out_ char** connectionString)
{
    int ret = 0;
    FILE* file = NULL;

    *connectionString = NULL;

    if (_access(*workingDirectory, 4) == 0) {
        printf("_access function result: \"The file exists.\"\n");
    }
    else {
        printf("_access function result: \"The file does not exist.\"\n");
    }

    const char* tbl = "r";// , ccs = UTF - 8";
    errno_t err = fopen_s(&file, *workingDirectory, tbl);
    if (err != 0) {
        printf("Cannot open file.\n");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    int i = 0;
    int* integers = (int*)malloc((length) * sizeof(int));

    if (integers) 
    {
        int b = -1;
        
        while ((b = fgetc(file)) != EOF) 
        {
            integers[i] = b;
            i++;
        }
    }

    char* encodedUTF8string = NULL;
    convertIntArrayToUnsignedCharArray(integers, &encodedUTF8string, i);

    char* decodedUTF8string = NULL;
    decodeUTF8Encoding(encodedUTF8string, &decodedUTF8string);

    *connectionString = decodedUTF8string;

    fclose(file);
    free(integers);

    return 0;

exit:
    fclose(file);
    free(integers);
    return ret;
}


void insert_string_safely(_Inout_ char** dest, _In_ const char* src, _In_ int pos) 
{
    if (dest == NULL || *dest == NULL || src == NULL) {
        return;  // Return if any of the input pointers are NULL
    }

    char* start = NULL;
    char* end = NULL;

    split_string(pos, dest, &start, &end);

    size_t lenS = strlen(start);
    size_t lensrc = strlen(src);
    size_t lenE = strlen(end);

    size_t tot = lenS + lensrc + lenE;

    char* new_dest = (char*)realloc(*dest, tot + 1);
    if (new_dest == NULL) {
        return;  // Return if memory allocation failed
    }

    // Append the rest of dest to new_dest
    strncpy_s(new_dest, tot + 1, start, strlen(start));
    strcat_s(new_dest, tot + 1, src);
    strcat_s(new_dest, tot + 1, end);

    free(start);
    free(end);

    *dest = new_dest;  // Update the dest pointer to point to the new string
}

int split_string(int index, char** src, char** start, char** end) 
{
    size_t first = strlen(*src);
    if (index < 0 || index > first)
    {
        return -1;
    }
    *start = (char*)malloc((index + 1) * sizeof(char));
    if (*start)
    {
        strncpy_s(*start, index + 1, *src, index);
        if (*start)
        {
            size_t st = strlen(*start);
            (*start)[st] = '\0'; // Null-terminate the first part
        }

        size_t nd = strlen(*src) - (index + 1) + 1;

        *end = (char*)malloc((nd + 1) * sizeof(char));
        if (*end)
        {
            strcpy_s(*end, nd + 1, *src + index);
            if (*end)
            {
                size_t second = strlen(*end);
                (*end)[second] = '\0';
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
    return 1;
}

int find_index_of_invoices_opening_bracket(_In_ const char* json) {
    const char* invoices_key = "\"invoices\": [";
    char* invoices_location = strstr(json, invoices_key);
    if (invoices_location != NULL) {
        return invoices_location - json + strlen(invoices_key) - 1;
    }
    else {
        return -1;  // Return -1 if "invoices": [ is not found in the JSON string
    }
}


int find_index_of_invoices_closing_bracketXXX(_In_ const char* json) {
    const char* invoices_key = "]";
    char* invoices_location = strstr(json, invoices_key);
    if (invoices_location != NULL) {
        return invoices_location - json + strlen(invoices_key) - 1;
    }
    else {
        return -1;  // Return -1 if "invoices": [ is not found in the JSON string
    }
}
 
int find_index_of_invoice_lines_opening_bracket(_In_ const char* json) 
{
    const char* invoice_lines_key = "\"invoice_lines\": [";
    char* invoice_lines_location = strstr(json, invoice_lines_key);
    if (invoice_lines_location != NULL) 
    {
        return invoice_lines_location - json + strlen(invoice_lines_key) - 1;
    }
    else 
    {
        return -1;  // Return -1 if "invoices": [ is not found in the JSON string
    }
}

int find_latest_index_of_invoice_lines_opening_bracket(const char* json) 
{
    const char* invoice_lines_key = "\"invoice_lines\": [";
    char* invoice_lines_location = strstr(json, invoice_lines_key);
    char* latest_location = invoice_lines_location;

    // Find the latest occurrence of invoice_lines_key in json
    while (invoice_lines_location != NULL) {
        latest_location = invoice_lines_location;
        invoice_lines_location = strstr(invoice_lines_location + 1, invoice_lines_key);
    }

    if (latest_location != NULL) {
        return latest_location - json + strlen(invoice_lines_key) - 1;
    }
    else {
        return -1;  // Return -1 if "invoice_lines": [ is not found in the JSON string
    }
}

int find_latest_index_of_invoice_line_id(const char* json, int index)
{
    //const char* invoice_lines_key = "\"invoice_lines\": ["; invoice_line_id
    const char* invoice_line_id_key = "\"invoice_line_id\": ";
    char* invoice_line_id_location = strstr(json, invoice_line_id_key);
    char* latest_location = invoice_line_id_location;

    // Find the latest occurrence of invoice_lines_key in json
    while (invoice_line_id_location != NULL) {
        latest_location = invoice_line_id_location;
        invoice_line_id_location = strstr(invoice_line_id_location + 1, invoice_line_id_key);
    }

    if (latest_location != NULL) {
        return latest_location - json + strlen(invoice_line_id_key) - 1;
    }
    else {
        return -1;  // Return -1 if "invoice_lines": [ is not found in the JSON string
    }
}

int find_index_of_invoices_closing_bracket(const char* json, int start_index) 
{
    if (json == NULL || start_index < 0 || start_index >= strlen(json)) {
        return -1;  // Return -1 if the input is invalid
    }

    const char* invoices_key = "]";
    char* invoices_location = strstr(json + start_index, invoices_key);
    if (invoices_location != NULL) {
        return invoices_location - json;
    }
    else {
        return -1;  // Return -1 if "]" is not found in the JSON string after start_index
    }
}

/*int decodeUTF8Encoding(_In_ char* encodedCharArray, _Out_ char** decodedCharArray)
{
    *decodedCharArray = NULL;

    int lengthWithEncodedChars = strlen(encodedCharArray);

    int ret = -1;
    unsigned char iso8859_1 = 0;

    int* integers = (int*)malloc(lengthWithEncodedChars * sizeof(int));

    if (!!integers)
    {
        for (int i = 0; i < lengthWithEncodedChars; i++)
        {
            integers[i] = (int)(unsigned char)encodedCharArray[i];
            printf("integers[i]: %d", integers[i]);
        }
    }
    else
    {
        goto exit;
    }

    long length = lengthWithEncodedChars;

    unsigned char* chars = (unsigned char*)malloc(1 * sizeof(char));

    if (!!chars)
    {
        chars[0] = '\0';
    }
    else
    {
        ret = ERROR_REALLOC_FAILED;
        goto exit;
    }

    long indexOfTheISO88591Array = 0;

    for (long indexOfCodepointArray = 0; indexOfCodepointArray < length; indexOfCodepointArray++)
    {
        unsigned char utf8[] = { 0, 0, 0 };
#pragma warning( push )
#pragma warning( disable : 6011 )
        utf8[0] = integers[indexOfCodepointArray];
#pragma warning( pop )

        // If the UTF-8 character is in the ASCII range, it's the same in ISO 8859-1
        if (utf8[0] < 128) 
        {
            iso8859_1 = utf8[0];
            utf8[1] = '\0';
        }
        // Otherwise, if it's a 2-byte UTF-8 character, we can find the ISO 8859-1 character by subtracting 194 from the first byte
        else if (utf8[0] < 224) 
        {
#pragma warning( push )
#pragma warning( disable : 6385)
            utf8[1] = (unsigned char)integers[indexOfCodepointArray + 1];
            utf8[2] = '\0';
#pragma warning( pop )
            iso8859_1 = (utf8[0] & 0x1F) << 6 | (utf8[1] & 0x3F);
        }

        unsigned char* tem = (unsigned char*)realloc(chars, (indexOfTheISO88591Array + 1) * sizeof(unsigned char) + 1);

        if (!tem)
        {
            free(chars);
            ret = ERROR_REALLOC_FAILED;
            goto exit;
        }

#pragma warning( push )
#pragma warning( disable : 28182 )
        long currentIndex = ((indexOfTheISO88591Array + 1) * sizeof(unsigned char));
        tem[currentIndex] = '\0';

        char* pointer = (char*) malloc(sizeof(char) * 2);

        if (!pointer)
        {
            goto exit;
        }
        pointer[0] = iso8859_1;
        pointer[1] = '\0';

#pragma warning( pop )
        if (indexOfTheISO88591Array == 0)
        {
            strcpy_s((char*)tem, (indexOfTheISO88591Array + 1) * sizeof(unsigned char) + 1, pointer);
        }
        else
        {
            strcat_s((char*)tem, (indexOfTheISO88591Array + 1) * sizeof(unsigned char) + 1, pointer);
        }
        free(pointer);

        chars = tem;
        if (utf8[0] >= 128 && utf8[0] < 224)
        {
            indexOfCodepointArray++;
        }
        indexOfTheISO88591Array++;
    }

    *decodedCharArray = chars;
    return 0;
exit:
    return -1;
}*/

int decodeUTF8Encoding(_In_ char* encodedCharArray, _Out_ char** decodedCharArray)
{
    int lengthWithEncodedChars = strlen(encodedCharArray);
    *decodedCharArray = malloc(lengthWithEncodedChars + 1); // Allocate memory for decodedCharArray

    if (!*decodedCharArray)
    {
        return -1; // Memory allocation failed
    }

    int j = 0;
    for (int i = 0; i < lengthWithEncodedChars; i++)
    {
        unsigned char c = encodedCharArray[i];
        if (c < 128) // ASCII character
        {
            (*decodedCharArray)[j++] = c;
        }
        else if (c < 224 && i + 1 < lengthWithEncodedChars) // 2-byte UTF-8 character
        {
            unsigned char c2 = encodedCharArray[++i];
            (*decodedCharArray)[j++] = ((c & 0x1F) << 6) | (c2 & 0x3F);
        }
        // Add more else if blocks here for 3-byte and 4-byte UTF-8 characters if needed
    }

    (*decodedCharArray)[j] = '\0'; // Null-terminate the output string

    return 0;
}