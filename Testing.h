#ifndef TESTING_H
#define TESTING_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Structures.h"
#define diffFailure 0x00000f00
#define runtimeError 0x000000f0
#define timeout 0x0000000f
#define compileFail 0x0000f000
#define missingFile 0x000f0000

void runTest(Program *program, Test *test, char *exePath);

int runGcc(Program *program, Test *test);

int checkStatus(int status, int errors);

int testTestCase(char *suiteName, Program *program, Test *test, int testNum);

void testProgram(char *suiteName, Program *program);

void testPrograms(char *suiteName, Program *headProgram);

#endif
