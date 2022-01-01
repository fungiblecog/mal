#!/bin/bash

for f in ./tests/*.mal; do

    echo "Processing $f file..";
    ../../runtest.py $f -- ./stepA_mal
done
