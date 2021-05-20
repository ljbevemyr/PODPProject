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
int PIVOT_MODE = 0;

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

  if (argc != 2 && argc != 3){
    printf("Usage : parqs <distribution> <length> <pivot_mode>\n");
    return 0;
  }
  int i, j, k, round, len, size, rank, dim;
  int print = 0;

  dim = atoi(argv[1]);
  if (argc == 3) print = atoi(argv[2]);

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size); /* Get the number of PEs*/
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); /* Get my number        */

  double *data;
  if (rank == 0) data = matrix_maker(dim);

  // Test pdf-matrix
  double temp[16] = {3.0, 11.0, 6.0, 16.0, 8.0, 1.0, 5.0, 10.0, 14.0, 7.0, 12.0, 2.0, 4.0, 13.0, 9.0, 15.0};
  //data = temp;

  if (print && rank == 0) print_rows(data, dim, dim, 0);

  // Start timer
  double start_time = MPI_Wtime();
  // Divide up the data between processes
  int proc_num_row = dim/size;
  int proc_size = proc_num_row * dim;
  int *displs = (int *)malloc(size*sizeof(int));
  int *scounts = (int *)malloc(size*sizeof(int));
  int c = 0;
  for (i=0; i<size; ++i) {
    displs[i] = c;
    c = c + proc_size;
    scounts[i] = proc_size;
  }
  double *proc_data_row = (double *)malloc(scounts[rank]*sizeof(double));
  double *proc_data_col = (double *)malloc(scounts[rank]*sizeof(double));
  MPI_Scatterv(&(data[0]), scounts, displs, MPI_DOUBLE,
               &(proc_data_row[0]), scounts[rank], MPI_DOUBLE,
               0, MPI_COMM_WORLD);
  if (rank == 0) {
    //print_rows(proc_data_row, dim, proc_num_row, 1);
  }

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
    for (i = 0; i < scounts[rank]; i++) {
      proc_data_col[i] = 9,0;
    }
    c = 0;
    for (i=0; i<size; ++i) {
      displs[i] = c;
      c = c + proc_num_row;
      scounts[i] = 1;
    }
    for (k = 0; k < size; k++) { //proc
      for (i = 0; i < proc_num_row; i++) { //row
        for (j = 0; j < proc_num_row; j++) { //col
          MPI_Scatterv(&(proc_data_row[i*dim+j]), scounts, displs, MPI_DOUBLE,
                       &(proc_data_col[j*dim+i + k*proc_num_row]),
                       scounts[rank], MPI_DOUBLE,
                       k, MPI_COMM_WORLD);
        }
      }
    }

    // Check if not last round
    if (round+1 < d) {
      //Sort columns
      for (i = 0; i < proc_num_row; i++) {
        quicksort(&(proc_data_col[i*dim]), 0, dim-1);
      }
      //print_rows(proc_data_col, dim, proc_num_row, 1);

      // Send values get rows
      c = 0;
      for (i=0; i<size; ++i) {
        displs[i] = c;
        c = c + proc_num_row;
        scounts[i] = 1;
      }
      for (k = 0; k < size; k++) { //proc
        for (i = 0; i < proc_num_row; i++) { //row
          for (j = 0; j < proc_num_row; j++) { //col
            MPI_Scatterv(&(proc_data_col[i*dim+j]), scounts, displs, MPI_DOUBLE,
                         &(proc_data_row[j*dim+i + k*proc_num_row]),
                         scounts[rank], MPI_DOUBLE,
                         k, MPI_COMM_WORLD);
          }
        }
      }

    } else {
      //print_rows(proc_data_col, dim, proc_num_row, 1);
      break;
      // Send to root values
    }
  }

  MPI_Gather(proc_data_col, dim*proc_num_row, MPI_DOUBLE,
              data, dim*proc_num_row, MPI_DOUBLE,
              0, MPI_COMM_WORLD);

  // Stop timer
  double end_time = MPI_Wtime();

  if (rank == 0) printf("Time: %f\n", end_time-start_time);

  if (print && rank == 0) print_col_as_rows(data, dim, dim, 0);

  //  }

  /*MPI_Scatterv(&(data[0]), scounts, displs, MPI_DOUBLE,
    &(proc_data[0]), scounts[rank], MPI_DOUBLE,
    0, MPI_COMM_WORLD);*/


  /*
  if (rank == 0) {
    int seq;

    seq=atoi(argv[1]);
    data=(double *)malloc(len*sizeof(double));

    if (seq==0) {

      // Uniform random numbers
      for (i=0;i<len;i++)
        data[i]=drand48();

    }

    else if (seq==1) {

      // Exponential distribution
      double lambda=10;
      for (i=0;i<len;i++)
        data[i]=-lambda*log(1-drand48());
    }

    else if (seq==2) {

      // Normal distribution
      double x,y;
      for (i=0;i<len;i++){
        x=drand48(); y=drand48();
        data[i]=sqrt(-2*log(x))*cos(2*PI*y);
      }
    } else if (seq==3){
      double *tmpData=(double *)malloc(len*sizeof(double));
      for (i=0;i<len;i++)
        tmpData[i]=drand48();
      quicksort(tmpData, 0, len-1);
      for (i=0;i<len;i++)
	data[i]=tmpData[len-1-i];
      
    }
    if (print){
      int j;
      for (j=0; j < len; j++) {
	printf("%f ", data[j]);
      }
      printf("\n");
      printf("Intial Data ...\n");
    }
    //Divide and distribute data into p equal parts
  }
  double start_timer = MPI_Wtime();
  int proc_size = floor((double)len/(double)size);
  int rest = len%size;
  int *displs = (int *)malloc(size*sizeof(int));
  int *scounts = (int *)malloc(size*sizeof(int));
  int c = 0;
  for (i=0; i<size; ++i) {
    displs[i] = c;
    if (rest>i) {
      scounts[i] = proc_size + 1;
      c = c + proc_size + 1;
    } else {
      c = c + proc_size;
      scounts[i] = proc_size;
    }
  }
  double *proc_data = (double *)malloc(scounts[rank]*sizeof(double));
  MPI_Scatterv(&(data[0]), scounts, displs, MPI_DOUBLE,
               &(proc_data[0]), scounts[rank], MPI_DOUBLE,
              0, MPI_COMM_WORLD);
  return_data_t *tmp = quicksort_rec(proc_data, scounts[rank], MPI_COMM_WORLD);
  int j;  
  MPI_Gather(&(tmp->len), 1, MPI_INT,
	     scounts, 1, MPI_INT,
	     0, MPI_COMM_WORLD);
  if (rank == 0){
    displs[0] = 0;
    for (j = 1; j < size; j++){
      displs[j] = scounts[j-1] + displs[j-1];
    }
  }
  MPI_Gatherv((tmp->data), tmp->len, MPI_DOUBLE,
	      data, scounts, displs, MPI_DOUBLE,
	      0, MPI_COMM_WORLD);
  
  double end_timer = MPI_Wtime();
 if(rank == 0){
   printf("Data size: %d, Mode used: %d, Proc used: %d, Time passed: %f\n", len, PIVOT_MODE, size, end_timer - start_timer);
 }

  if (print) {
    if (rank == 0){
      for (j = 0; j < len; j++){
	printf("%f ", data[j]);
      }
      printf("\n");
      printf("Data sorted correctly!\n");
    }
  }
  */
  free(proc_data_col);
  free(proc_data_row);
  free(displs);
  free(scounts);
  if (rank == 0) free(data);
  
  MPI_Finalize();
  return 0;
}


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
  //temp=data[pivotIndex]; data[pivotIndex]=data[right]; data[right]=temp;
  storeIndex = left;
  for (i=left;i<right;i++)
    if (data[i] >= data[right]){
      temp=data[i];
      data[i]=data[storeIndex];
      data[storeIndex]=temp;
      storeIndex = storeIndex + 1;
    }
  temp = data[right]; data[right] = data[storeIndex]; data[storeIndex] = temp;
  //temp=data[storeIndex];data[storeIndex]=data[right]; data[right]=temp;
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

  for (i = 0; i < matrix_size*matrix_size; i++){
    //double decimal = (double) (i % 10000);
    //decimal = decimal / 10000;
    double value = (double) (i % 4);
    //double value = (double) i;
    //value = value + decimal;
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
