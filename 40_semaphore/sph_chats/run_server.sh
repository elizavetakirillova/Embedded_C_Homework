#!/bin/sh
gcc task2.c -o cs
O_KEY=$1
if [ -z "$O_KEY" ]; then
  O_KEY="1"
fi
echo "O_KEY = ${O_KEY}"
>"/tmp/shared-memory-key-${O_KEY}"
>"/tmp/sem-mutex-key-${O_KEY}"
./cs -s
