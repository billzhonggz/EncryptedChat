/*
	The test project for encryption algorithm.
*/

#include<stdio.h>
#include<string.h>
#include<math.h>
#include<cstdlib>
#include<ctime>

int main(void)
{
	printf("Enter a plaintext for encryption.\n");
	char* plaintext;
	scanf("%s\n", plaintext);
	printf("Your input is: %s.\n", plaintext);

	// Begin RSA encryption. 
	
}

// Generate a prime number within 0-999.
int genPrimeNum(void)
{
	// Set system time as random seed. 
	// That ensure the random number is different.
	srand((unsigned)time(0));
	int randomNum = rand() * 1000;
	// Calaculate the most close prime number. 

}

long rsa(int baseNum, int key, long message)
{
	if (baseNum < 1 || key < 1)
		return 0L;
	// Data after encryption / decryption.
	long rsaMessage = 0L;
	// Core encryption.
	double rsaPow = pow(message, key);
	rsaMessage = lround(rsaPow) % baseNum;
	return rsaMessage;
}