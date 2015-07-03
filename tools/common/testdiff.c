#include "iniparser.h"
#include <stdio.h>
#include <stdlib.h>
#include "commonlib.h"

int main(int argc,  char **argv)
{
  int i;
  i=ini_diff(argv[1],argv[2]);	
  printf("ini diff result is %d\n",i);
  return 0;	
}
