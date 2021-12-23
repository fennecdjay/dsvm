#!/bin/sh

do_test() {
  ret=$(./ds_test $1)
  if [ "$ret" = "result $2" ]
  then echo "OK"
  else echo "NOT OK"
  fi
}

do_test 10 55
do_test 20 6765
do_test 30 832040
do_test 40 102334155
