#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
/*
typedef struct {
  double *pvector;
  size_t n;
} vector_data_t;
*/


typedef struct {
  double **p_mat;
  int up;
  int left;
  double *p_vec_up;
  size_t n;
  size_t m;
  //vector_data_t result;
  double *result;
} block_data_t;

//vector_data_t* block_mult(block_data_t *data) {
double* block_mult(block_data_t *data) {
  /*
  vector_data_t *result = malloc(sizeof(vector_data_t));
  result->n = data->n;
  result->pvector = malloc(result->n * sizeof(double));
  */
  double* result = malloc(data->n * sizeof(double));

  for (int i = 0; i < data->n; ++i) {
    //result->pvector[i] = 0;
    result[i] = 0;
    int ind = data->up + i;
    for (int j = 0; j < data->m; ++j) {
      int jnd = data->left + j;
      //result->pvector[i] += data->p_left_up[i][j] * data->p_vec_up[j];
      result[i] += data->p_mat[ind][jnd] * data->p_vec_up[j];
    }
  }
//   free(result);
  return result;
}

void *block_mult_routine(void *data) {
  
  block_data_t *data_ = (block_data_t *)data;
  
  data_->result = block_mult(data_);
  
  return NULL;
  
}

void run_block_mult() {
  int n;
  scanf("%d", &n);

  double *vec = malloc(n * sizeof(double));
  double **mat = malloc(n * sizeof(double*));
  if (!vec || !mat) {
    exit(1);
  }

  for (int i = 0; i < n; ++i) {
    scanf("%lf", &vec[i]);
  }

  for (int i = 0; i < n; ++i) {
    mat[i] = malloc(n * sizeof(double));
    for (int j = 0; j < n; ++j) {
      scanf("%lf", &(mat[i][j]));
    }
  }
  // return;
  int thrcount = 4; 

  pthread_t *handles = malloc(thrcount * thrcount * sizeof(pthread_t));
  block_data_t *data = malloc(thrcount * thrcount * sizeof(block_data_t));
  if (!handles || !data) {
    exit(1);

  }


  int up = 0;
  int left = 0;
  int q = n / thrcount;
  int r = n % thrcount;
  int row_cnt = 0;
  int col_cnt = 0;


  for (int i = 0; i < thrcount; i++) {
    up += row_cnt;
    row_cnt = q + (i < r);
    left = 0;
    col_cnt = 0;
    for (int j = 0; j < thrcount; j++) {
        left += col_cnt;
        col_cnt = q + (j < r);
        int ind = i * thrcount + j;
        
        data[ind].p_mat = malloc(n * sizeof(double*));
       
        data[ind].p_vec_up = malloc(col_cnt * sizeof(double));

        data[ind].result = malloc(row_cnt * sizeof(double));
              
    }
    
  }
  // return;
  up = 0;
  left = 0;
  q = n / thrcount;
  r = n % thrcount;
  row_cnt = 0;
  col_cnt = 0;
//  return;
  for (int i = 0; i < thrcount; i++) {
    up += row_cnt;
    row_cnt = q + (i < r);
    left = 0;
    col_cnt = 0;
    for (int j = 0; j < thrcount; j++) {
      
      
      
        left += col_cnt;
        col_cnt = q + (j < r);
        int ind = i * thrcount + j;
        
        
        data[ind].n = row_cnt;
        data[ind].m = col_cnt;
        // data[ind].p_mat = malloc(n * sizeof(double*));
        data[ind].p_mat = mat;
        data[ind].up = up;
        data[ind].left = left;
        // data[ind].p_vec_up = malloc(col_cnt * sizeof(double));
        data[ind].p_vec_up = &(vec[left]);
        // data[ind].result = malloc(row_cnt * sizeof(double));
        
        
        pthread_create(&handles[ind], NULL, (void *(*)(void *)) & block_mult_routine, &data[ind]);
    }
    
  }
//  return;
  for (int i = 0; i < n; i++) {
    pthread_join(handles[i], NULL);
  }

  printf("\nresult: ");
  double *answer = malloc(n * sizeof(double));

  up = 0;
  left = 0;
  row_cnt = 0;
  col_cnt = 0;

  for (int i = 0; i < thrcount; i++) {
    up += row_cnt;
    row_cnt = q + (i < r);
    left = 0;
    col_cnt = 0;
    for (int j = 0; j < thrcount; j++) {
      left += col_cnt;
      col_cnt = q + (j < r);
      int ind = i * thrcount + j;
      for (int k = 0; k < row_cnt; k++) {
        answer[k + up] += data[ind].result[up];
      }

    }
  }
  for (int i = 0; i < n; ++i) {
    printf("%.2lf ", answer[i]);
  }
  printf("\n");
  
  // free(answer);
  
  
  // for (int i = 0; i < thrcount; i++) {
  //   for (int j = 0; j < thrcount; j++) {
  //       int ind = i * thrcount + j;
  //       free(data[ind].p_mat);
  //       free(data[ind].p_vec_up);
  //       free(data[ind].result);
  //   }
    
  // }
  
  // free(data);
  // free(handles);
  // for (int i = 0; i < n; ++i) {
  //   free(mat[i]);
  // }
  // free(mat);
  // free(vec);
}

int main() {
    run_block_mult();
    return 0;
}