#include <stdio.h>
#include "Utilities.h"

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

    if (_getcwd(cwd, sizeof(cwd)) != 0) {
        printf("Current working directory: **%s**\n", cwd);
    }
    else {
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
    return ret;

#pragma warning( pop )

    error:
    free(*pWorkingDir);
    return ret;
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
        temp = NULL;

        s = strlen(*workingDirectory);

        size_t ss = strlen(fileName);

        strcat_s(*workingDirectory, s + ss + 1, fileName);
    }

    readFile(workingDirectory, connectionString);

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
        return -1;
    }

    *dest = invoicesTemp;

    if (destLen == 0)
    {
        // Copy the source string to the destination buffer safely
        if (strncpy_s(*dest, newLength, source, _TRUNCATE) != 0) {
            printf("String copy failed\n");
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
        printf("%s", *dest);
    }
    return 0;
}

/**
* This function ...
*
* @param customer_id: ...
*/
int getInvoiceData(_In_ int customer_id, _In_ int invoice_id, _In_ char* bank_reference, _Inout_ char** dest)
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

    return success;
}

/**
* This function reads a content of a file in the file system.
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

    const char tbl[13] = "r";// , ccs = UTF - 8";
    errno_t err = fopen_s(&file, *workingDirectory, &tbl);
    if (err != 0) {
        printf("Cannot open file.\n");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    int* integers = (int*)malloc((length) * sizeof(int));
    if (integers) {
        int b = -1;
        int i = 0;
        while ((b = fgetc(file)) != EOF) {
            // printf("%d ", b);
            integers[i] = b;
            i++;
        }
    }

    unsigned char iso8859_1 = 0;

    unsigned char* chars = (unsigned char*)malloc(sizeof(unsigned char) + 1);

    if (!!chars)
    {
#pragma warning( push )
#pragma warning( disable : 6011 )
        chars[0] = 'a';
#pragma warning( pop )
        chars[1] = '\0';
    }
    else
    {
        ret = ERROR_REALLOC_FAILED;
        goto exit;
    }

    long indexOfTheISO88591Array = 0;

    for (long indexOfCodepointArray = 0; indexOfCodepointArray < length; indexOfCodepointArray++)
    {
        unsigned char utf8[] = { 0, 0 };
#pragma warning( push )
#pragma warning( disable : 6011 )
        utf8[0] = integers[indexOfCodepointArray];
#pragma warning( pop )

        // If the UTF-8 character is in the ASCII range, it's the same in ISO 8859-1
        if (utf8[0] < 128) {
            iso8859_1 = utf8[0];
        }
        // Otherwise, if it's a 2-byte UTF-8 character, we can find the ISO 8859-1 character by subtracting 194 from the first byte
        else if (utf8[0] < 224) {
#pragma warning( push )
#pragma warning( disable : 6385)
            utf8[1] = (unsigned char)integers[indexOfCodepointArray + 1];
#pragma warning( pop )
            iso8859_1 = (utf8[0] & 0x1F) << 6 | (utf8[1] & 0x3F);
        }

#pragma warning( push )
#pragma warning( disable : 28182 )
        chars[indexOfTheISO88591Array] = (unsigned char)iso8859_1;
#pragma warning( pop )

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
#pragma warning( pop )
        chars = tem;
        if (utf8[0] >= 128 && utf8[0] < 224)
        {
            indexOfCodepointArray++;
        }
        indexOfTheISO88591Array++;
    }

    *connectionString = chars;

    fclose(file);
    free(integers);

    return 0;

exit:
    fclose(file);
    free(integers);
    return ret;
}
