#!/usr/bin/gnuplot -persist

max=1e-2

set title "Location data"
set xlabel "location"
set yrange [0:1e-2]
set ylabel "count"
set grid
plot "offset_data.txt" u (column(0)):2:xtic(1) w l title "", 1000000
