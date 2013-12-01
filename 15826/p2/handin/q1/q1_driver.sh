#! /usr/bin/env bash

rm -f hw2.q1.output.txt
echo "Calculating the DTW between the following files:"

for file1 in $(ls data);
do
  for file2 in $(ls data);
  do
    if test ! $file1 = $file2; then
      ./timewarp data/$file1 data/$file2
	fi
  done
done

echo "Results have been saved in hw2.q1.output.txt"