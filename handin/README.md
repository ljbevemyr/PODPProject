# PODPProject

A parallel implementation of the shear sort algorithm using MPI communication.

**_NOTE:_**  The number matrix dimensions has to be evenly dividable with the number of processors.

## Usage

Make the program using the Makefile.

```bash
make
```
### Test performance
Run the code to test performace (without printed matrices):

```bash
mpiexec -n <number of processors> ssnew <matrix dimensions>
```
### Test correctnes
Run the code to test correctnes (with printed matrices):

```bash
mpiexec -n <number of processors> ssnew <matrix dimensions> 1
```
### Test input and output files
If you want to run the program with a custom inputfile and produce a outputfile with the result. The format of the file should only contain the values as doubles listed after each other. As the example below:
Matrix-format:
a b c
d e f
g h i

File-format for input file:
a b c d e f g h i

```bash
mpiexec -n <number of processors> ssnew <matrix dimensions> x <name of inputfile> <name of outputfile>
```

## Author
Lisa Bevemyr
lisa.bevemyr.9492@student.uu.se
