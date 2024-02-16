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
#回到项目根目录
cd ..

# 把头文件拷贝到/usr/include/mymuduo so库拷贝到 /usr/lib

if [ ! -d /usr/include/mymuduo ]; then
    mkdir /usr/include/mymuduo
fi

# copy 头.h文件
for header in ./include/*.h
do
    cp $header /usr/include/mymuduo
done

#库文件copy

cp $(pwd)/lib/libmymuduo_net.so /usr/lib
cp $(pwd)/lib/libmymuduo_utils.so /usr/lib

# 更新库中文件告述ld

ldconfig
