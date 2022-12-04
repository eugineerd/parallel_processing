#include "row_mult.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("pass in file path\n");
    exit(0);
  }
  FILE *in_file = fopen(argv[1], "r");
  FILE *out_file = fopen("/dev/null", "w");

  double time_ms = 0;
  for (int i = 0; i < 100000; ++i) {
    time_ms += (double)run_row_mult_test(in_file, out_file);
    fseek(in_file, 0, SEEK_SET);
  }
  printf("avg. time: %.3lfms\n", time_ms / 100000);
}