#!/bin/bash

./consistency.sh	||	exit 1
./fix.sh		||	exit 1

echo "All checks done successfully"
exit 0
