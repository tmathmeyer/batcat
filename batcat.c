
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


void timed_pipe(unsigned long long timestamp_ms, FILE *src, FILE *dest, bool debug) {
  char c[16] = {0}; // the batter power is usually about 7 digits, so be safe.
  
  fgets(c, 16, src);
  fprintf(dest, "%llu, %s", timestamp_ms, c);
  
  if (debug) {
    printf("%llu, %s", timestamp_ms, c);
  }
}

int main(int argc, char* argv[]) {
  char c = 0;
  char *battery = NULL;
  char *output = NULL;
  bool debug = false;
  opterr = 0;

  while((c = getopt(argc, argv, "b:f:d")) != EOF) {
    switch(c) {
      case 'b':
        battery = strdup(optarg);
        break;
      case 'f':
        output = strdup(optarg);
        break;
      case 'd':
        debug = true;
        break;
      case '?':
        if (optopt == 'f' || optopt == 'b')
          printf("Option -%c requires an argument\n", optopt);
        else
          printf("Option -%c unrecognized\n", optopt);
        return 1;
    }
  }

  if (!battery) battery = strdup("BAT0");
  if (!battery) { perror("No battery"); return 1; }

  if (!output) output = strdup("batcat_output");
  if (!output) { perror("No output file"); return 1; }

  char *power_device = malloc(strlen(battery) + 36);
  sprintf(power_device, "/sys/class/power_supply/%s/power_now", battery);

  FILE *destination = fopen(output, "w");
  if (!destination) { perror("No output file"); return 1; }

  struct timespec time;
  clock_gettime(CLOCK_REALTIME, &time);
  unsigned long long base_timestamp_ns = time.tv_nsec + time.tv_sec * 1000000000;
  unsigned long long base_timestamp_ms = base_timestamp_ns / 1000000;

  while(1) {
    clock_gettime(CLOCK_REALTIME, &time);
    unsigned long long timestamp_ns = time.tv_nsec + time.tv_sec * 1000000000;
    unsigned long long timestamp_ms = (timestamp_ns / 1000000) - base_timestamp_ms;

    FILE *battery_src = fopen(power_device, "r");
    if (!battery_src) { perror("No battery"); return 1; }
    timed_pipe(timestamp_ms, battery_src, destination, debug);
    fclose(battery_src);

    clock_gettime(CLOCK_REALTIME, &time);
    unsigned long long post_write_ts = time.tv_nsec + time.tv_sec * 1000000000;
    unsigned long long elapsed_us = (post_write_ts - timestamp_ns) / 1000;

    // Expected to sleep for 100ms (1 tenth of a second), or 100000us
    unsigned long long delay = (99800 - elapsed_us);
    usleep(delay);
  }
}