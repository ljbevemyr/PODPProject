#!/bin/bash -l

#SBATCH -A uppmax2021-2-7
#SBATCH -p core -n 16
#SBATCH -t 2:00:00

mpiexec -n 16 ssnew 1000
mpiexec -n 16 ssnew 2000
mpiexec -n 16 ssnew 4000
mpiexec -n 16 ssnew 8000
mpiexec -n 16 ssnew 16000
mpiexec -n 16 ssnew 20000
