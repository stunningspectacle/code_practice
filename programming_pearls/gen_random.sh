#!/bin/bash

buf=
count=0

if ! [ $# -eq 2 ]; then
    echo "usage: $0 max num"
    exit
fi

MAX=$1
NUM=$2
file=random_"$MAX"_"$NUM".txt

if [ -f $file ]; then
    rm $file
fi

while true;do 
    tmp=$RANDOM
    if [ $tmp -gt $MAX ]; then
        continue
    fi
    buf=$buf$tmp"\n"
    count=$((count + 1))
    if [ $((count % 500)) -eq 0 ]; then
        echo -e -n "$buf" >> $file
        buf=""
    fi
    if ! [ $count -lt $NUM ]; then
        if [ "$buf" != "" ]; then
            echo -n -e $buf >> $file
        fi
        exit
    fi
done
