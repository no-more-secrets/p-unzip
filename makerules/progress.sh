#!/bin/bash
# Given a fraction this will print a progress bar:
#
#   [===========================>      ] (12/15)
#
# It  will then return the cursor to the beginning of the line so
# that  the  function  can be called repeatedly to create an ani-
# mated progress bar.
progress() {
    prefix="$1"; max=$2
    numerator=$3; denominator=$4
    # Get an  integer  in  range  [0,100]  indicating  percentage
    # progress.
    p=$(( numerator*1000 / denominator / 10 ))
    printf "$prefix["
    # Number of '=' or '>' that we need to print
    bar=$(( p * max / 100 ))
    # Number of spaces required to pad out to $max.
    spaces=$(( max - bar ))
    # First draw the bar itself
    while true; do
        # See if we've done the entire bar.
        (( bar == 0 )) && break
        # Print  an  arrow  at  the head of the progress bar, but
        # only if the progress is not yet at 100%.
        if (( bar == 1 && spaces != 0 )); then
            printf '>'
        else
            printf '='
        fi
        bar=$(( bar - 1 ))
    done
    # Now pad the rest with spaces
    while true; do
        (( spaces == 0 )) && break
        printf ' '
        spaces=$(( spaces - 1 ))
    done
    printf '] (%s/%s)      \r' $numerator $denominator
}
