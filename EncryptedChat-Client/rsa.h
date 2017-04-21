#pragma once

//char msg[100];

int prime(long int);

void calcE(long int r, long int prime1, long int prime2);

long int calcD(long int x, long int r);

char* doEncrypt(char* msg, long int prime1, long int prime2, long int key);

char* doDecrypt(char *encrytedMsg, long int prime1, long int prime2, long int key);

long int* rsaGenKeyPair(long int prime1, long int prime2);

#define  DEFAULT_BUFFER 4096