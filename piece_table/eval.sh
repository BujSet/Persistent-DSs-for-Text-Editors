#!/bin/bash

make
for i in {-100..-600..-100}
do
./pt_test_persistent $i
./pt_test $i
done
