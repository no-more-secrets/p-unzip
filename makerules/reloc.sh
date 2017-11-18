#!/bin/bash
# This bash script will simply find all  of  the  auto-dependency
# files  under  the  root and invoke a perl script on each one to
# adjust  the  relatives  paths in the file to be relative to the
# new PWD. It will also print  a  progress  bar as the process is
# happening.
this=$(cd $(dirname $0) && pwd)

. $this/progress.sh

root=$1
old_pwd=$2
new_pwd=$3

files=$(find $root -name "*.d")
# Get total number files to process.
array=( $files ); count=${#array[@]}

[[ ! "$files" ]] && exit

so_far=0 # How many files we've processed.

for f in $files; do
    $this/reloc.pl $new_pwd $old_pwd $f > $f.tmp || {
        echo "Error relocating $f!"
        # If there was an error  in  processing  then  file  then
        # leave the tmp file for debugging and  don't  erase  the
        # original.
        continue;
    }
    mv $f.tmp $f
    so_far=$(( so_far + 1 ))
    progress "   \033[35mfixing\033[00m " 50 $so_far $count
done
echo
