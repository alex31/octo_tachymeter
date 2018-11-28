#!/usr/bin/gnuplot -c
# usage : ./doplot.gp datafile.tsv

print "datafile name        : ", ARG1

set datafile separator "\t"
plot ARG1 using 1:2 with lines, ARG1 using 1:3 with lines

pause 30
reread