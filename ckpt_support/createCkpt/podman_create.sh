#!/bin/bash
local_gem5_dir=/home/cuihongwei/experiment/gem5_dir/gem5_riscv_ckpt
build_map="-v ${local_gem5_dir}/createCkpt/build_create:/home/gem5_src/gem5/build"
src_map="-v ${local_gem5_dir}/createCkpt/gem5_create:/home/gem5_src/gem5/src"
config_map="-v ${local_gem5_dir}/createCkpt/gem5_configs:/home/gem5_src/gem5/configs"
shell_map="-v ${local_gem5_dir}/createCkpt/shell:/home/gem5_src/gem5/shell"
bench_map="-v ${local_gem5_dir}/createCkpt/bench:/home/gem5_src/gem5/bench"
ckptinfo="-v ${local_gem5_dir}/restoreCkpt/ckptinfo:/home/gem5_src/gem5/ckptinfo"
local_dir_map="-v /home/cuihongwei/experiment/local_dir:/root/local_dir"

map=$build_map" "$src_map" "$config_map" "$ckptinfo" "$shell_map" "$bench_map
podman run -it --rm $map $local_dir_map  registry.cn-hangzhou.aliyuncs.com/ronglonely/gem5:3.0 /bin/bash
