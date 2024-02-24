# !/bin/bash

set -e #出错就报错

echo $(pwd)

#如果没有build目录，就创建
if [ ! -d "$(pwd)/build" ]; then
    mkdir "$(pwd)/build"
fi

#删除之前编译的文件
rm -rf $(pwd)/build/*

cd $(pwd)/build/ && 
    cmake .. &&
    make

