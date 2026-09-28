#!/usr/bin/env bash

filename=$(realpath "lib.so")
[ ! -f "${filename}" ] && echo "${filename} not found" && exit 1

pid=$(pidof hoi4)
[ ! -z "${pid}" ] && echo "HOI4 is already running, aborting" && exit 1

echo "Waiting for HOI4..."
while [ -z "${pid}" ]; do
	sleep 0.5
	pid=$(pidof hoi4)
done

gdb -n -q -batch-silent \
  -ex "attach ${pid}" \
  -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
  -ex "set \$dlerror = (char*(*)(void)) dlerror" \
  -ex "set \$lib = \$dlopen(\"${filename}\", 1)" \
  -ex "call \$dlerror()" \
  -ex "detach" \
  -ex "quit"
echo "Done."
