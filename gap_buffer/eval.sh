#!/bin/bash

make
for i in {-100..-600..-100}
do
./gb_test_persistent $i
./gb_test $i
done
