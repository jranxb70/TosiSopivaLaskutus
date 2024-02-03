#include "BankReferenceCalculator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_DIGITS 10

#define true 1
#define false 0
#define bool int

void calcNewReference(const int nNumber, int* referenceOut)
{
    int resultArray[NUM_DIGITS]; // Assuming a maximum of 10 digits
    *referenceOut = 0;

    // Initialize the result array with zeros
    for (int i = 0; i < NUM_DIGITS; i++) {
        resultArray[i] = 0;
    }

    divideIntoArray(nNumber, resultArray);

    int theCheck = calcTheCheck(resultArray);

    int theBankReference = formTheReference(nNumber, theCheck);

    printf("%s", "\n");
    bool digitUnequalToZeroFound = false;
    // Print the result array
    for (int i = 0; i < NUM_DIGITS; i++) {
        if (resultArray[i] != 0 || digitUnequalToZeroFound) {
            printf("%d ", resultArray[i]);
            digitUnequalToZeroFound = true;
        }
    }
    *referenceOut = theBankReference;
    printf("\nThe bank reference is: %d\n", theBankReference);
}

int formTheReference(const int theReferenceBase, const int intValue)
{
    char buffer[2];  // Adjust the size based on your needs
    char buffer2[NUM_DIGITS];  // Adjust the size based on your needs
    sprintf_s(buffer, sizeof(buffer), "%d", intValue);
    sprintf_s(buffer2, sizeof(buffer2), "%d", theReferenceBase);

    if (strcat_s(buffer2, sizeof(buffer2), buffer) == 0) {
        // Concatenation was successful
        printf("%s\n", buffer2);
    }
    else {
        // Handle the error, e.g., buffer was too small
        printf("Concatenation failed. Buffer too small.\n");
    }

    // Using atoi to convert string to int
    int bankReference = atoi(buffer2);
    return bankReference;
}

void divideIntoArray(int integerValue, int resultArray[]) {
    // Ensure the integer value is non-negative
    if (integerValue < 0) {
        // Handle negative numbers as needed
        return;
    }

    int numDigits = countDigits(integerValue);

    // Iterate through each digit and store in the array in correct order
    for (int i = 0; i < numDigits; i++) {
        resultArray[i] = integerValue % 10;
        integerValue /= 10;
    }
}


int calcTheCheck(int resultArray[])
{
    int temp = 0;
    const int seven = 7;
    const int three = 3;
    const int one = 1;
    int current = -1;

    int sum = 0;


    // Iterate through each digit and store in the array in correct order
    for (int i = 0; i < NUM_DIGITS; i++) {
        temp = resultArray[i];

        if (current == -1 || current == one)
        {
            current = seven;
        }
        else if (current == seven)
        {
            current = three;
        }
        else if (current == three)
        {
            current = one;
        }
        temp = current * temp;
        sum += temp;
    }
    int theCheckNumber = 0;
    printf("\n%d\n", sum);
    if (sum % 10 != 0)
    {
        int full = nextFullTen(sum);
        theCheckNumber = full - sum;
    }

    return theCheckNumber;
}

int nextFullTen(const int number) {
    int remainder = number % 10;
    int nextTen = number - remainder + 10;

    return nextTen;
}

int countDigits(int theNumber)
{
    // Determine the number of digits in the integer
    int numDigits = 1;
    int tempValue = theNumber;
    while (tempValue /= 10) {
        numDigits++;
    }
    return numDigits;
}
