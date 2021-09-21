#!/bin/sh

echo     "std::string $2_vs = " > $1/$2.h
cat $1/$2.vert | sed -s 's/^/"/g' | sed -z "s/\\n/\\\\n\"\\n/g" >> $1/$2.h
echo ";" >> $1/$2.h

echo "\n\nstd::string $2_fs = " >> $1/$2.h
cat $1/$2.frag | sed -s 's/^/"/g' | sed -z "s/\\n/\\\\n\"\\n/g" >> $1/$2.h 
echo ";" >> $1/$2.h

