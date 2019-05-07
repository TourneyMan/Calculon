#ifndef POPULATE_H
#define POPULATE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Structures.h"

void freeArguments(Argument *argument);

void freeFiles(File *file);

void freeTests(Test *test);

void freeProgram(Program *program);

File *makeIncludeFile(FILE *fp, char *fileName);

Program *readProgram(FILE *fp);

Test *readTest(FILE *fp);

Argument *makeArgument(FILE *fp, char *argument);

#endif
