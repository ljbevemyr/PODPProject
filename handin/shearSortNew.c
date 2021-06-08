/**********************************************************************
 * Quick sort
 * Usage: ./a.out sequence length
 *
 **********************************************************************/
#define PI 3.14159265358979323846
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <string.h>

typedef struct Return_data {
  double *data;
  int len;
} return_data_t;

int partition(double *data, int left, int right, int pivotIndex);
return_data_t *quicksort_rec(double *data, int len_data, MPI_Comm comm);
void quicksort(double *data, int left, int right);
void quicksort_rev(double *data, int left, int right);
double *merge(double *v1, int n1, double *v2, int n2);
double *matrix_maker(int matrix_size);
void print_rows(double *data, int matrix_dim, int num_rows, int show_rank);
void print_col_as_rows(double *data, int matrix_dim, int num_rows, int show_rank);

int main(int argc, char *argv[]) {
  int i, j, k, size, rank, dim, round;
  int print = 0;
  double start_time, end_time;
  double *data;
  char *input_filename;
  char *output_filename;


  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size); /* Get the number of PEs*/
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); /* Get my number        */

  if (argc != 2 && argc != 3 && argc != 5){
    if (rank == 0) {
      printf("Usage: mpiexec -n <number of processors> ssnew <matrix dimensions> <print> <input file> <output file>\n");
      printf("       print: 1 if input and output should be printed\n");
      printf("       input file: The name of the input file (optional)\n");
      printf("       output file: The name of the output file (optional)\n");
    }
    MPI_Finalize();
    return 0;
  }

  dim = atoi(argv[1]);
  if (argc >= 3 && dim < 100) print = atoi(argv[2]);

  if (dim%size != 0){
    printf("The matrix-dimensions has to be eqally dividable with the number of processes\n");
    return 0;
  }

  if (rank == 0 && argc == 5) {
    input_filename = argv[3];
    output_filename = argv[4];
    FILE *file = fopen(input_filename, "r");
    data = (double*)malloc((dim*dim)*sizeof(double));
    for (i = 0; i < dim*dim; i++){
      fscanf(file, "%lf", &data[i]);
      printf("data[%d]: %f\n", i, data[i]);
    }
    fclose(file);
  } else {
    if (rank == 0) data = matrix_maker(dim);
  }

  if (print && rank == 0) print_rows(data, dim, dim, 0);

  // Start timer
  start_time = MPI_Wtime();
  // Divide up the data between processes
  int proc_num_row = dim/size;
  int proc_size = proc_num_row * dim;

  MPI_Datatype column_t;
  MPI_Type_vector(proc_num_row, proc_num_row, dim, MPI_DOUBLE, &column_t);
  MPI_Type_commit(&column_t);

  double *proc_data_row = (double *)malloc(proc_size*sizeof(double));
  double *proc_data_col = (double *)malloc(proc_size*sizeof(double));
  double *tmp = (double *)malloc(proc_size*sizeof(double));
  MPI_Scatter(&(data[0]), proc_size, MPI_DOUBLE,
    &(proc_data_row[0]), proc_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  // Shear sort
  int d = ceil(log2(dim))+1;
  for (round = 0; round < d; round++) {
    for (j = 0; j < proc_num_row; j++) {
      int index = rank*proc_num_row + j;
      if (index % 2 == 0) {
        quicksort(&(proc_data_row[j*dim]), 0, dim-1);
      } else {
        quicksort_rev(&(proc_data_row[j*dim]), 0, dim-1);
      }
    }

    // Send values and get column
    for (i = 0; i < size; i++) {
      if (i != rank) {
        MPI_Sendrecv(&(proc_data_row[i*proc_num_row]), 1, column_t,
                     i, 3,
                     &(tmp[i*proc_num_row]), 1, column_t,
                     i, 3,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
    }

    for (i =0; i < size; i++) {
      for (j = 0; j < proc_num_row; j++) {
        for (k = 0; k < proc_num_row; k++) {
          if (i == rank) {
            proc_data_col[j*dim+proc_num_row*i+k] = proc_data_row[k*dim+proc_num_row*i+j];
          } else {
            proc_data_col[j*dim+proc_num_row*i+k] = tmp[k*dim+proc_num_row*i+j];
          }
        }
      }
    }

    // Check if not last round
    if (round+1 < d) {
      //Sort columns
      for (i = 0; i < proc_num_row; i++) {
        quicksort(&(proc_data_col[i*dim]), 0, dim-1);
      }

    for (i = 0; i < size; i++) {
      if (i != rank) {
        MPI_Sendrecv(&(proc_data_col[i*proc_num_row]), 1, column_t,
                     i, 3,
                     &(tmp[i*proc_num_row]), 1, column_t,
                     i, 3,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
    }

    for (i =0; i < size; i++) {
      for (j = 0; j < proc_num_row; j++) { //ev. scounts[i]
        for (k = 0; k < proc_num_row; k++) {
          if (i == rank) {
            proc_data_row[j*dim+proc_num_row*i+k] = proc_data_col[k*dim+proc_num_row*i+j];
          } else {
            proc_data_row[j*dim+proc_num_row*i+k] = tmp[k*dim+proc_num_row*i+j];
          }
        }
      }
    }

    } else {
      break;
      // Send to root values
    }
  }

  //Gather all sorted colums
  MPI_Gather(proc_data_col, dim*proc_num_row, MPI_DOUBLE,
              data, dim*proc_num_row, MPI_DOUBLE,
              0, MPI_COMM_WORLD);

  // Stop timer
  end_time = MPI_Wtime();
  if (rank == 0) printf("Dim: %d, Size: %d, Time: %f seconds\n", dim, size, end_time-start_time);
  if (print && rank == 0) print_col_as_rows(data, dim, dim, 0);

  if (rank == 0 && argc == 5){
    FILE *output_file = fopen(output_filename, "w");

    for (i = 0; i < dim; i++){
      for (j = 0; j < dim; j++) {
        fprintf(output_file, "%lf ", data[j*dim+i]);
      }
      fprintf(output_file, "\n");
    }
    fclose(output_file);
  }

  free(proc_data_col);
  free(proc_data_row);
  if (rank == 0) free(data);
  MPI_Finalize();
  return 0;
}

/****************** HELPER FUNCTIONS ******************/


int partition(double *data, int left, int right, int pivotIndex){
  double pivotValue,temp;
  int storeIndex,i;
  pivotValue = data[pivotIndex];
  temp=data[pivotIndex]; data[pivotIndex]=data[right]; data[right]=temp;
  storeIndex = left;
  for (i=left;i<right;i++)
    if (data[i] <= pivotValue){
      temp=data[i];data[i]=data[storeIndex];data[storeIndex]=temp;
      storeIndex = storeIndex + 1;
    }
  temp=data[storeIndex];data[storeIndex]=data[right]; data[right]=temp;
  return storeIndex;
}

void quicksort(double *data, int left, int right){
  int pivotIndex, pivotNewIndex;

  if (right > left){
    pivotIndex = left+(right-left)/2;
    pivotNewIndex = partition(data, left, right, pivotIndex);
    quicksort(data,left, pivotNewIndex - 1);
    quicksort(data,pivotNewIndex + 1, right);
  }
}

int partition_rev(double *data, int left, int right, int pivotIndex){
  double pivotValue,temp;
  int storeIndex,i;
  pivotValue = data[pivotIndex];
  temp = data[right]; data[right] = data[pivotIndex]; data[pivotIndex] = temp;
  storeIndex = left;
  for (i=left;i<right;i++)
    if (data[i] >= data[right]){
      temp=data[i];
      data[i]=data[storeIndex];
      data[storeIndex]=temp;
      storeIndex = storeIndex + 1;
    }
  temp = data[right]; data[right] = data[storeIndex]; data[storeIndex] = temp;
  return storeIndex;
}

void quicksort_rev(double *data, int left, int right){
  int pivotIndex, pivotNewIndex;
  if (right > left){
    pivotIndex = left+(right-left)/2;
    pivotNewIndex = partition_rev(data, left, right, pivotIndex);
    quicksort_rev(data,left, pivotNewIndex - 1);
    quicksort_rev(data,pivotNewIndex + 1, right);
  }
}

double *matrix_maker(int matrix_size){
  double *matrix = (double*)malloc((matrix_size*matrix_size)*sizeof(double));
  int i;
  double value;

  for (i = 0; i < matrix_size*matrix_size; i++){
    value = (double) (matrix_size*matrix_size-i);
    matrix[i] = value;
  }
  return matrix;
}

void print_rows(double *data, int matrix_dim, int num_rows, int show_rank){
  int i, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (show_rank){
    printf("My rank : %d\n My values:\n", rank);
  }

  for (i = 0; i < matrix_dim*num_rows; i++){
    if ((i % matrix_dim) == 0){
      printf("\n");
    }
    printf("%lf ", data[i]);
  }
  printf("\n");
}

void print_col_as_rows(double *data, int dim, int num_col, int show_rank) {
  int i, j, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (show_rank){
    printf("My rank : %d\n My values:\n", rank);
  }

  for (i = 0; i < dim; i++){
    for (j = 0; j < num_col; j++) {
      printf("%lf ", data[j*dim+i]);
    }
    printf("\n");
  }
  printf("\n");
}
