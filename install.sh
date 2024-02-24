# !/bin/bash

# 把头文件拷贝到/usr/include/mymuduo so库拷贝到 /usr/lib
set -e #出错就报错


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
echo "success"