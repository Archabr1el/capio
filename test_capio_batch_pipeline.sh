#!/bin/bash


if [ $# -lt 3 ] || [ $# -gt 4 ]
then
	echo "input error: ./test_capio_batch_pipeline.sh parallelism num_elements num_IO_ops [MPI params])"
exit
fi

PAR=$1
NUM_ELEMS=$2
N_IOOPS=$3
MPI_PARS=$4
mpiexec $MPI_PARS -n 1 src/capio_server > server.log &
PID=$!
echo $PAR " " $NUM_ELEMS " " $N_IOOPS >> time_capio_writes.txt
{ time mpiexec $MPI_PARS -n $PAR unit_tests/simple_write $NUM_ELEMS $N_IOOPS > output_capio_writes.txt ; } 2> >(grep real >> time_capio_writes.txt) 
echo $PAR " " $NUM_ELEMS " " $N_IOOPS >> time_capio_reads.txt
{ time mpiexec $MPI_PARS -n $PAR unit_tests/simple_read $NUM_ELEMS $N_IOOPS > output_capio_reads.txt ; } 2> >(grep real >> time_capio_reads.txt) 
kill $PID
sleep 1
./clean.sh