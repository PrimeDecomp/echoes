#!/bin/bash -e
VERSION="${VERSION:=0}"
OBJDUMP="$DEVKITPPC/bin/powerpc-eabi-objdump -Dz -bbinary -EB -mpowerpc -M gekko"
if [ ! -z "$1" ]; then
  OPTIONS="--start-address=$(($1)) --stop-address=$(($2))"
fi
$OBJDUMP $OPTIONS main.dol > main.dump
$OBJDUMP $OPTIONS new.dol > new.dump
diff -u --color=always main.dump new.dump
