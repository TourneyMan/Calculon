#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Structures.h"
#include "Populate.h"
#include "Testing.h"
   
int main(int argc, char **argv) {
   Program *headProgram = NULL;
   char buffer[MAX_SIZE];
   FILE *fp;
   char *suiteName = argv[1];

   fp = fopen(suiteName, "r");
   fscanf(fp, "%s", buffer);

   headProgram = readProgram(fp);
   testPrograms(suiteName, headProgram);
   freeProgram(headProgram);

   return 0;
}

