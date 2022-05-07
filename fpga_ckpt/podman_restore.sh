#!/bin/bash
local_gem5_dir=`pwd`
build_map="-v ${local_gem5_dir}/build_restore:/home/gem5_src/gem5/build"
src_map="-v ${local_gem5_dir}/gem5_restore:/home/gem5_src/gem5/src"
config_map="-v ${local_gem5_dir}/gem5_configs:/home/gem5_src/gem5/configs"
ckptinfo="-v ${local_gem5_dir}/ckptinfo:/home/gem5_src/gem5/ckptinfo"

map=$build_map" "$src_map" "$config_map" "$ckptinfo
podman run -it --rm $map registry.cn-hangzhou.aliyuncs.com/ronglonely/gem5:3.0 /bin/bash
