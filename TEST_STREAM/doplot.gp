#!/usr/bin/gnuplot

set datafile separator "\t"
plot 'octoTacho.tsv' using 1:2 with lines, 'octoTacho.tsv' using 1:3 with lines

pause 30
reread
