#!/bin/bash

cc=g++
flags="-static -O2"
dflags=""
target="showinfo"

filelist="ckpt_load.cpp elf_load.cpp readckpt.cpp"
echo ${cc} $filelist $flags $dflags -o ${target}
${cc} $filelist $flags $dflags -w -o ${target}

