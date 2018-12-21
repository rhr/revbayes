#!/bin/sh

SRC=../../src

cd $SRC

echo "Defining variable 'src_inc' in 'src/meson.build'"

echo "src_inc = include_directories([" > meson.build
find libs revlanguage core cmd -name '*.h' |
    sed "s|/[^/]*$||g" |
    sort |
    uniq |
    sed "s|^|'|" |
    sed "s|$|',|" >> meson.build
echo "])" >> meson.build
echo >> meson.build
