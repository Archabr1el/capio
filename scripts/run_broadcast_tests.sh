#!/bin/bash

mpiexec -n 2 ../build/capio_process/capio 1048576 10000 ../one_node_2_2.yaml > broadcast_2_2_capio.txt &
sleep .1
mpiexec -n 2 ../build/tests/collective/broadcast/broadcast_read 1048576 ../one_node_2_2.yaml > broadcast_2_2_cons.txt &
sleep .1
mpiexec -n 2 ../build/tests/collective/broadcast/broadcast_write 1048576 2 ../one_node_2_2.yaml > broadcast_2_2_prods.txt
if [ $? -ne 0 ];
then
    echo "broadcast test case 1 failed"
fi
sleep .1
rm /dev/shm/*
mpiexec -n 2 ../build/capio_process/capio 1048576 10000 ../one_node_2_4.yaml > broadcast_2_4_capio.txt &
sleep .1
mpiexec -n 4 ../build/tests/collective/broadcast/broadcast_read 1048576 ../one_node_2_4.yaml > broadcast_2_4_cons.txt &
sleep .1
mpiexec -n 2 ../build/tests/collective/broadcast/broadcast_write 1048576 4 ../one_node_2_4.yaml > broadcast_2_4_prods.txt
if [ $? -ne 0 ];
then
    echo "broadcast test case 2 failed"
fi
sleep .1
rm /dev/shm/*
mpiexec -n 2 ../build/capio_process/capio 1048576 10000 ../one_node_4_2.yaml > broadcast_4_2_capio.txt &
sleep .1
mpiexec -n 2 ../build/tests/collective/broadcast/broadcast_read 1048576 ../one_node_4_2.yaml > broadcast_4_2_cons.txt &
sleep .1
mpiexec -n 4 ../build/tests/collective/broadcast/broadcast_write 1048576 2 ../one_node_4_2.yaml > broadcast_4_2_prods.txt
if [ $? -ne 0 ];
then
    echo "broadcast test case 3 failed"
fi
rm /dev/shm/*