/* Copyright 2010 A.N.M. Imroz Choudhury
 *
 * array-walk.c - A short C program that allocates an array, adds up
 * its (uninitialized) values, prints the address of the array, and
 * the computed total, then exits.  Meant as a sanity check for
 * reference tracing software.
 */

#include <stdio.h>
#include <stdlib.h>

int main(){
  int *array = (int *)malloc(32*sizeof(int));
  int i;
  int total = 0;

  for(i=0; i<32; i++){
    array[i] = i;
  }

  for(i=0; i<32; i++){
    total += array[i];
  }

  free(array);

  printf("%p\n", array);
  printf("%d\n", total);

  return 0;
}
