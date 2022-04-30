#!/bin/bash

if [[ $# < 1 ]]; then
    echo "parameters are not enough!"
    exit
fi

src_dir=`pwd`
dst_dir=$1

if [[ ! -e $dst_dir/src/main/scala ]]; then
    echo "$dst_dir/src/main/scala is not exist!"
    exit
fi

if [[ ! -e $src_dir/boom_scala ]]; then
    echo "$src_dir/boom_scala is not exist!"
    exit
fi

echo rm $dst_dir/src/main/scala -rf
rm $dst_dir/src/main/scala -rf

echo cp $src_dir/boom_scala $dst_dir/src/main/scala -r
cp $src_dir/boom_scala $dst_dir/src/main/scala -r
