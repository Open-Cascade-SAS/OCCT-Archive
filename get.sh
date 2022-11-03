#!/bin/bash

problem_file=Adaptor2d_Curve2d.cxx

list="master integration"
echo "Getting branches hash"
for name in $list ; do
    echo $name
    export shorthash_$name=$(git rev-parse $name)
    echo shorthash_$name=$(eval echo "\$shorthash_${name}")
done

list=$(git rev-list --ancestry-path ${shorthash_master}..${shorthash_integration})

for hash in $list ; do    
    list_changed_files=$(git diff-tree --no-commit-id --name-only -r $hash | sed 's/\// /g' | awk '{ print $NF }')
    for changed_file in $list_changed_files; do
        echo problem_file=$problem_file
        echo changed_file=$changed_file
        if [ "$problem_file" == "$changed_file" ] ; then
            echo Reverting: 
            echo $(git log -1 --format=oneline $hash)
            # git revert $hash
            git rebase --onto $hash^ $hash
        fi
    done
done