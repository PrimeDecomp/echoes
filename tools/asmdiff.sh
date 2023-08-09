#!/bin/bash -e
VERSION="${VERSION:=0}"
OBJDUMP="$DEVKITPPC/bin/powerpc-eabi-objdump -Dz -bbinary -EB -mpowerpc -M gekko"
if [ ! -z "$1" ]; then
  OPTIONS="--start-address=$(($1)) --stop-address=$(($2))"
fi
$OBJDUMP $OPTIONS orig/G2ME01/sys/main.dol > /tmp/main.dump
$OBJDUMP $OPTIONS build/G2ME01/main.dol > /tmp/new.dump
diff -u --color=always /tmp/main.dump /tmp/new.dump
