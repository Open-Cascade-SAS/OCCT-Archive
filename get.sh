#!/bin/bash
list="master integration"
for name in $list ; do
    echo $name
    export shorthash_$name=$(git rev-parse $name)
    echo shorthash_$name=$(eval echo "\$shorthash_${name}")
done

#echo shorthash_master=$shorthash_master
