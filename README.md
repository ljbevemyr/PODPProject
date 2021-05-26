# PODPProject

A parallel implementation of the shear sort algorithm using MPI communication.
**_NOTE:_**  The number matrix dimensions has to be evenly dividable with the number of processors.

## Usage

Make the program using the Makefile.

```bash
make
```
Run the code to test performace (without printed matrices):

```bash
mpiexec -n <number of processors> ssnew <matrix dimensions>
```
Run the code to test correctnes (with printed matrices):

```bash
mpiexec -n <number of processors> ssnew <matrix dimensions> 1
```

## Author
Lisa Bevemyr
lisa.bevemyr.9492@student.uu.se
