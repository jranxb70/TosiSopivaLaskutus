#ifndef BANKREFERENCECALCULATOR_H
#define BANKREFERENCECALCULATOR_H

// Content of Ansi.h


// Function declaration
#ifdef __cplusplus
extern "C" {
#endif

	void calcNewReference(const int nNumber);


#ifdef __cplusplus
}
#endif

int calcTheCheck(int resultArray[]);
void divideIntoArray(int integerValue, int resultArray[]);
int countDigits(int theNumber);
int formTheReference(const int theReferenceBase, const int intValue);
int nextFullTen(const int number);

#endif // BANKREFERENCECALCULATOR_H

