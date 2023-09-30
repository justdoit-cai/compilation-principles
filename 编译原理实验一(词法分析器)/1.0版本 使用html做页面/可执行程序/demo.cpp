#include <stdio.h>

#ifdef __cplusplus
extern "C"{
#endif
int* __stdcall __declspec(dllexport) myFunction()
{
   int* myArray = new int[3];
   myArray[0] = 1;
   myArray[1] = 2;
   myArray[2] = 3;
   return myArray;
}

#ifdef __cplusplus
}
#endif