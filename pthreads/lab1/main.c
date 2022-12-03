#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 5
sem_t sem[NUM_THREADS];

void tokenize(int *pi) {
  int i = *pi;
  char str[1000];
  char out[1000] = {0};
  char out_buf[1000] = {0};
  char *buf;

  sem_wait(&sem[i]);
  fgets(str, 1000, stdin);
  sem_post(&sem[(i + 1) % NUM_THREADS]);

  char *pout = &out[0];
  sprintf(out_buf, "thread %d: ", i);
  int buf_size = strlen(out_buf);
  snprintf(pout, buf_size, "%s", out_buf);
  pout += buf_size;

  char *tok = strtok_r(str, " ", &buf);
  while (tok) {
    // sprintf(out, "%s ", tok);
    sprintf(out_buf, "%s  ", tok);
    int buf_size = strlen(out_buf);
    snprintf(pout - 1, buf_size, "%s", out_buf);
    pout += buf_size - 1;
    tok = strtok_r(NULL, " ", &buf);
  }
  printf("%s\n", out);
}

int main() {
  pthread_t handles[NUM_THREADS];
  int args[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++) {
    args[i] = i;
    if (i == 0) {
      sem_init(&sem[i], 0, 1);
    } else {
      sem_init(&sem[i], 0, 0);
    }
  }
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_create(&handles[i], NULL, (void *)tokenize, &args[i]);
  }
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(handles[i], NULL);
  }
}