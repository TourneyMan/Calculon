#ifndef STRUCTURES_H
#define STRUCTURES_H
#define MAX_SIZE 255

typedef struct Program {
   char *executableName;
   char *programName;
   struct Test *tests;
   struct File *includeFiles;
   struct Program *next;
} Program;

typedef struct File {
   char *fileName;
   struct File *next;
} File;

typedef struct Test {
   char *inName;
   char *outName;
   int time;
   struct Argument *arguments;
   struct Test *next;
} Test;

typedef struct Argument {
   char *argument;
   struct Argument *next;
} Argument;

#endif
