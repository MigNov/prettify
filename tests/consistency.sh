#!/bin/bash

# These are files for check in the format of "$filename:$expected_error_code"
FILES=( tabs.c:0 spaces.c:0 mixed.c:2 )

BINARY="../src/prettify"
EXAMPLE_DATA_DIR="../example-data"

function split_item()
{
	local str="$1"

	SAVE_IFS="$IFS"
	IFS=":"
	num=0
	filename=""
	public=""
	for i in $str
	do
		if [ "$num" -eq 0 ]; then
			filename="$i"
		else
			public="$i"
		fi
		let num=$num+1
	done
	IFS="$SAVE_IFS"

	out=( $filename $public )
	echo ${out[@]}
}

for file in ${FILES[@]}
do
	tmp=( $(split_item $file) )
	filename=${tmp[0]}
	expcode=${tmp[1]}

	$BINARY --input $EXAMPLE_DATA_DIR/$filename --analyze --silent
	if [ "$?" != "$expcode" ]; then
		echo "Check of file $filename failed!"
		exit 1
	fi
done

echo "Consistency checks done successfully"
exit 0
