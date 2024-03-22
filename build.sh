#!/bin/bash
folder="${PWD}/build"
param=$1

echo ${PWD}
echo ${folder}

build() {
    cd ${PWD}/build
    cmake ..
    make -j8
}

buildall() {
    rm -rf ${PWD}/build
    mkdir ${PWD}/build
    cd ${PWD}/build
    cmake ..
    make -j8
}
  
if [ -d "$folder" ]; then  
    echo "文件夹已存在"  
else  
    echo "文件夹不存在，正在创建..."  
    mkdir ${folder}
fi

case ${param} in
    -a)
        buildall
        ;;
    "")
        build
        ;;
    *)
        echo "Invalid parameter: &param"
esac