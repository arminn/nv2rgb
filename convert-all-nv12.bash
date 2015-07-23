#!/bin/bash

for file in $PWD/*.nv12
do
	./nv2rgb $file 640 480
done
