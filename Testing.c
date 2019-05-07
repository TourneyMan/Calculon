#include "Testing.h"
void makeInnerDir(Program *program, Test *test, char *tempDir) {
   char **cpArgs;
   char **curArg;
   File *includeFile = program->includeFiles;

   cpArgs = calloc(sizeof(char*), MAX_SIZE);
   curArg = cpArgs;

   *curArg++ = "cp";
   *curArg++ = test->inName;
   *curArg++ = test->outName;
   *curArg++ = program->executableName;
   *curArg++ = program->programName;
   while (includeFile != NULL) {
      *curArg++ = includeFile->fileName;
      includeFile = includeFile->next;
   }
   *curArg++ = tempDir;
   *curArg++ = NULL;

   mkdir(tempDir, ACCESSPERMS);
   if (fork() == 0) {
      execvp("cp", cpArgs);
   }
   wait(NULL);

   chdir(tempDir);
   return;
}

int runGcc(Program *program, Test *test) {
   char **compileArgs;
   char **curArg;
   int status;
   File *fileToAdd = program->includeFiles;

   compileArgs = calloc(sizeof(char*), MAX_SIZE);
   curArg = compileArgs;

   *curArg++ = "gcc";
   *curArg++ = program->programName;
   while (fileToAdd != NULL) {
      if (access(fileToAdd->fileName, F_OK) == 0) {
         *curArg++ = fileToAdd->fileName;
         fileToAdd = fileToAdd->next;
      }
      else {
         free(compileArgs);
         return missingFile;
      }
   }
   *curArg++ = "-o";
   *curArg++ = program->executableName;
   *curArg = NULL;

   if (fork() == 0) {
      execvp("gcc", compileArgs);
   }
   wait(&status);

   if (WEXITSTATUS(status) > 0) {
      printf("Failed:");
      curArg = compileArgs;
      while (*curArg != NULL) {
         printf(" %s", *curArg);
         curArg++;
      }
      free(compileArgs);
      return compileFail;
   }

   return 0;
}

void runTest(Program *program, Test *test, char *exePath) {
   char **testArgs, **curArg;
   char *cpuTime = calloc(sizeof(char), MAX_SIZE);
   char *cpuTimeArg = calloc(sizeof(char), MAX_SIZE);
   char *wallTime = calloc(sizeof(char), MAX_SIZE);
   char *wallTimeArg = calloc(sizeof(char), MAX_SIZE);
   int wallTimeInt = (test->time) * 10, inFile, outFile;

   sprintf(cpuTime, "%d", test->time);
   strcat(cpuTimeArg, "-t");
   strcat(cpuTimeArg, cpuTime);

   sprintf(wallTime, "%d", wallTimeInt);
   strcat(wallTimeArg, "-T");
   strcat(wallTimeArg, wallTime);

   testArgs = calloc(sizeof(char *), MAX_SIZE);
   curArg = testArgs;
   *curArg++ = "SafeRun";
   *curArg++ = "-p30";
   *curArg++ = cpuTimeArg;
   *curArg++ = wallTimeArg;
   *curArg++ = exePath;
   Argument *argToAdd = test->arguments;
   while (argToAdd != NULL) {
      *curArg++ = argToAdd->argument;
      argToAdd = argToAdd->next;
   }
   *curArg++ = NULL;

   inFile = open(test->inName, O_RDONLY);
   outFile = open("test.output.temp", O_WRONLY | O_CREAT | O_TRUNC, 0600);
   dup2(inFile, 0);
   close(inFile);
   dup2(outFile, 1);
   dup2(outFile, 2);
   close(outFile);

   execvp("SafeRun", testArgs);
   perror("SafeRun exec failed: ");
}

int checkErrors(int status, int errors) {
   if (status >= 192) {
      if ((status & 0x7) != 0) {
         errors = errors | timeout;
      }
      if ((status & 0x18) != 0) {
         errors = errors | runtimeError;
      }
   }
   else if (status >= 180) {
      errors = errors | runtimeError;
   }

   return errors;
}

int testTestCase(char *suiteName, Program *program, Test *test, int testNum) {
   char *dirPath = calloc(sizeof(char), MAX_SIZE);
   char *exePath = calloc(sizeof(char), MAX_SIZE); 
   int status, errors = 0;

   strcat(dirPath, "./");
   strcat(dirPath, test->outName);
   strcat(dirPath, ".dir");

   strcat(exePath, "./");
   strcat(exePath, program->executableName);

   if (access(test->inName, F_OK) != 0) {
      printf("Could not find required test file '%s'\n", test->inName);
      return missingFile;
   }
   if (access(test->outName, F_OK) != 0) {
      printf("Could not find required test file '%s'\n", test->outName);
      return missingFile;
   }

   if (access("./Makefile", F_OK) == 0) {
      if (fork() == 0) {
         int devNull = open("/dev/null", O_WRONLY);
         dup2(devNull, 1);
         execl("/usr/bin/make", "make", "-s", program->executableName, NULL);
      }
      wait(&status);
      if (WEXITSTATUS(status) > 0) {
         printf("Failed: make %s", program->executableName);
         return compileFail;
      }
   }

   //gcc
   else {
      status = runGcc(program, test);
      if (status) {
         return status;
      }
   }

   makeInnerDir(program, test, dirPath);

   //Run the test
   if (fork() == 0) {
      runTest(program, test, exePath);
   }

   wait(&status);
   errors = checkErrors(WEXITSTATUS(status), errors);

   if (fork() == 0) {
      int devNull = open("/dev/null", O_WRONLY);
      dup2(devNull, 1);
      dup2(devNull, 2);
      close(devNull);
      execlp("diff", "diff", "test.output.temp", test->outName, NULL);
   }

   wait(&status);
   if (WEXITSTATUS(status) != 0) {
      errors = errors | diffFailure;
   }

   chdir("..");
   free(dirPath);
   free(exePath);
   return errors;
}

void testProgram(char *suiteName, Program *program) {
   int totalTests = 0;
   int passedAllTests = 1;
   int thisTestPassed = 1;
   int errors;
   Test *curTest = program->tests;

   while (curTest != NULL) {
      totalTests += 1;
      errors = testTestCase(suiteName, program, curTest, totalTests);
      thisTestPassed = 1;
      
      if (errors == missingFile) {
         exit(1);
      }

      if (errors == compileFail) {
         passedAllTests = 0;
         thisTestPassed = 0;
      }

      if ((errors & diffFailure) > 0) { 
         printf("%s test %d failed: diff failure",
          program->executableName, totalTests);
         passedAllTests = 0;
         thisTestPassed = 0;
      }

      if ((errors & runtimeError) > 0) {
         if (thisTestPassed) {
            printf("%s test %d failed: runtime error",
             program->executableName, totalTests);
         }
         else {
            printf(", runtime error");
         }
         passedAllTests = 0;
         thisTestPassed = 0;
      }
      
      if ((errors & timeout) > 0) {
         if (thisTestPassed) {
            printf("%s test %d failed: timeout",
             program->executableName, totalTests);
         }
         else { 
            printf(", timeout");
         }
         passedAllTests = 0;
         thisTestPassed = 0;
      }

      if (thisTestPassed == 0) {
         printf("\n");
      }
      curTest = curTest->next;
   }

   if (passedAllTests == 1) {
      printf("%s %d of %d tests passed.\n",
       program->executableName, totalTests, totalTests);
   }
}

void testPrograms(char *suiteName, Program *headProgram) {
   testProgram(suiteName, headProgram);
   while (headProgram->next != NULL) {
      headProgram = headProgram->next;
      testProgram(suiteName, headProgram);
   }   
}
