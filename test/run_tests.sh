#!/bin/bash

for dir in `find . -mindepth 1 -type d` ; do
    if [ -f $dir/input ] ; then
        $jxtl -s -x t.xml -t $dir/input > $dir/test.output
        
        if [ $? -ne 0 ] ; then
            echo "jxtl exited with non-zero status when running test in $dir"
            exit 1
        fi
        
        diff $dir/output $dir/test.output > /dev/null 2>&1
        if [ $? -ne 0 ] ; then
            echo "Failed test in $dir"
            exit 1
        fi
        rm $dir/test.output
    fi
done

exit 0
