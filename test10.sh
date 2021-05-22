#!/bin/bash -l

#SBATCH -A uppmax2021-2-7
#SBATCH -p core -n 10
#SBATCH -t 2:00:00

mpiexec -n 10 ssnew 1000
mpiexec -n 10 ssnew 2000
mpiexec -n 10 ssnew 4000
mpiexec -n 10 ssnew 8000
mpiexec -n 10 ssnew 16000
mpiexec -n 10 ssnew 20000

