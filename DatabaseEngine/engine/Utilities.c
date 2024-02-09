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

int concatToJsonData(char** dest, const char* source)
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