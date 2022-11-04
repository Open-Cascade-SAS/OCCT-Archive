#!/bin/bash

problem_file=upgrade.bat

list="master integration"
echo "Getting commit hash"
for name in $list ; do
    echo $name
    export shorthash_$name=$(git rev-parse --short $name)
    echo shorthash_$name=$(eval echo "\$shorthash_${name}")
done

echo "Getting list of commits between master and integration"
list=$(git rev-list --ancestry-path ${shorthash_master}..${shorthash_integration})

echo "Getting commit whose hash matches the hash of commit with problematic file"
for hash in $list ; do    
    list_changed_files=$(git diff-tree --no-commit-id --name-only -r $hash | sed 's/\// /g' | awk '{ print $NF }')
    for changed_file in $list_changed_files; do
        if [ "$problem_file" == "$changed_file" ] ; then
            echo Reverting: 
            echo $(git log -1 --format=oneline $hash)
            # git revert $hash
            git rebase --onto $hash^ $hash
        fi
    done
done