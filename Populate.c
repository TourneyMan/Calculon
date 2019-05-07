#include "Populate.h"

void freeArguments(Argument *argument) {
   Argument *nextArgument;
   while (argument) {
      nextArgument = argument->next;

      free(argument->argument);
      free(argument);

      argument = nextArgument;
   }
}

void freeFiles(File *file) {
   File *nextFile;
   while (file) {
      nextFile = file->next;

      free(file->fileName);
      free(file);

      file = nextFile;
   }
}

void freeTests(Test *test) {
   Test *nextTest;
   while (test) {
      nextTest = test->next;

      free(test->inName);
      free(test->outName);
      freeArguments(test->arguments);
      free(test);

      test = nextTest;
   }
}

void freeProgram(Program *program) {
   Program *nextProgram;
   while (program) {
      nextProgram = program->next;

      free(program->executableName);
      free(program->programName);
      freeTests(program->tests);
      freeFiles(program->includeFiles);
      free(program);

      program = nextProgram;
   }
}

File *makeIncludeFile(FILE *fp, char *fileName) {
   File *newFile = malloc(sizeof(File));
   newFile->fileName = calloc(sizeof(char), MAX_SIZE);

   char *curChar = newFile->fileName;
   while (*fileName) {
      *curChar++ = *fileName++;
   }
   *curChar = *fileName;

   newFile->next = NULL;
   return newFile;
}

Argument *makeArgument(FILE *fp, char *argument) {
   Argument *newArgument = malloc(sizeof(Argument));
   newArgument->argument = calloc(sizeof(char), MAX_SIZE);

   char *curChar = newArgument->argument;
   while (*argument) {
      *curChar++ = *argument++;
   }
   *curChar = *argument;

   newArgument->next = NULL;
   return newArgument;
}

Test *readTest(FILE *fp) {
   Test *newTest = malloc(sizeof(Test));

   newTest->inName = calloc(sizeof(char), MAX_SIZE);
   fscanf(fp, "%s", newTest->inName);

   newTest->outName = calloc(sizeof(char), MAX_SIZE);
   fscanf(fp, "%s", newTest->outName);

   fscanf(fp, "%d", &(newTest->time));

   char buffer[MAX_SIZE];
   newTest->arguments = NULL;
   Argument *curArgument;
   while (fscanf(fp, "%s", buffer) != EOF) {
      if (strcmp(buffer, "T") != 0 && strcmp(buffer, "P") != 0) {
         if (newTest->arguments == NULL) {
            newTest->arguments = makeArgument(fp, buffer);
            curArgument = newTest->arguments;
         }
         else {
            curArgument->next = makeArgument(fp, buffer);
            curArgument = curArgument->next;
         }
      }
      else {
         break;
      }
   }

   if (strcmp(buffer, "T") == 0) {
      newTest->next = readTest(fp);
   }
   
   return newTest;
}

Program *readProgram(FILE *fp) {
   Program *newProgram = malloc(sizeof(Program));
   
   newProgram->executableName = calloc(sizeof(char), MAX_SIZE);
   fscanf(fp, "%s", newProgram->executableName);
   
   newProgram->programName = calloc(sizeof(char), MAX_SIZE);
   fscanf(fp, "%s", newProgram->programName);

   char buffer[MAX_SIZE];

   newProgram->includeFiles = NULL;
   File *curFile;
   while (fscanf(fp, "%s", buffer) != EOF) {
      if (strcmp(buffer, "T") != 0) {
         if (newProgram->includeFiles == NULL) {
            newProgram->includeFiles = makeIncludeFile(fp, buffer);
            curFile = newProgram->includeFiles;
         }
         else {
            curFile->next = makeIncludeFile(fp, buffer);
            curFile = curFile->next;
         }
      }
      else {
         break;
      }
   }

   newProgram->tests = readTest(fp);

   if (!feof(fp)) {
      newProgram->next = readProgram(fp);
   }

   else {
      newProgram->next = NULL;
   }

   return newProgram;
}
