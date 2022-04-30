#!/bin/bash
local_dir=/home/cuihongwei/unicore-boom-fpga/riscv-pk
build_map="-v ${local_dir}:/home/pktest"

map=$build_map
docker_run="/home/gem5_src/gem5/spec2006_script/single_run.sh $1 1 x86; exit;"

if [[ $# == 1 ]]; then
    podman run -it --rm $map registry.cn-hangzhou.aliyuncs.com/ronglonely/gem5:3.0 /bin/bash -c "$docker_run"
else
    #podman run -it --rm $map $local_dir_map  registry.cn-hangzhou.aliyuncs.com/ronglonely/gem5:3.0 /bin/bash
    podman run -it --rm $map $local_dir_map  localhost/chipyard-slim  /bin/bash
fi
