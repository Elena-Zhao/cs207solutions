#!/bin/bash
cc -o bloom -O2 bloom.c -DN_HASHES=3
# Hash test
./bloom 0
# Smoke test
./bloom 1
# Experiments
cc -o bloom -O2 bloom.c -DN_HASHES=1 && ./bloom 2
cc -o bloom -O2 bloom.c -DN_HASHES=2 && ./bloom 2
cc -o bloom -O2 bloom.c -DN_HASHES=3 && ./bloom 2
cc -o bloom -O2 bloom.c -DN_HASHES=4 && ./bloom 2
cc -o bloom -O2 bloom.c -DN_HASHES=5 && ./bloom 2
