#!/bin/bash

> time_and_interval.txt
> offset_data.txt
gcc -pthread -o Thread Time_Interval_Thread.c
# timeout 10 ./Thread "1"

# get time with X period for (interval) seconds, can be changed for better surveying
interval=1

#run 5 times
for i in {6..2} 
do
	X_fake=$((10 ** $i))
    echo "$X_fake" > freq.txt
    echo "X frequency: $X_fake" >> time_and_interval.txt

    # run program for (interval) seconds
    timeout $interval ./Thread 
    # sleep 10
done
