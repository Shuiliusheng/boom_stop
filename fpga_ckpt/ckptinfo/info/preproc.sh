#!/bin/bash
if [[ $# < 1 ]]; then
    echo "parameter is not enought!"
    exit
fi

if [[ ! -e ../createckpt/process.py ]]; then
    echo "../createckpt/process.py is not exist!"
fi

srcfile=$1
dir=""

if [[ ! -e $srcfile ]]; then
    echo $srcfile " is not exist"
    exit
fi

if [[ $# == 2 ]]; then
    echo "output is copy to $2"
    dir=$2
    mkdir -p $dir
fi

textinfo=`grep -r textRange $srcfile | awk -F '{' '{print "{"$2}' `
echo $textinfo
echo $textinfo >temp.log
cat $srcfile | awk -F '{' '{print "{"$2}' >> temp.log
logname=$(basename $srcfile )
mv temp.log $logname
mv $logname $dir

../createckpt/process.py $dir/$logname

