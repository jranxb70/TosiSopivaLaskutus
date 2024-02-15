#ifndef BANKREFERENCECALCULATOR_H
#define BANKREFERENCECALCULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



// Function declaration
#ifdef __cplusplus
extern "C" {
#endif
	void calcNewReference(
		_In_ const int					nNumber, 
		_Out_ int*						referenceOut);


#ifdef __cplusplus
}
#endif

int calcTheCheck(
		int								resultArray[]);

void divideIntoArray(
		int								integerValue, 
		int								resultArray[]);

int countDigits(
		int								theNumber);

int formTheReference(
		const int						theReferenceBase, 
		const int						intValue);

int nextFullTen(
		const int						number);

#endif // BANKREFERENCECALCULATOR_H

