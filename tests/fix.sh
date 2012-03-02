#!/bin/bash

# These are files for check in the format of "$filename:$expected_error_code"
FILES=( tabs.c spaces.c mixed.c )

BINARY="../src/prettify"
EXAMPLE_DATA_DIR="../example-data"

tmpdir=$(mktemp -d)

for filename in ${FILES[@]}
do
	$BINARY --input $EXAMPLE_DATA_DIR/$filename --analyze --silent

	if [ "$?" == "1" ]; then
		$BINARY --input $EXAMPLE_DATA_DIR/$filename --output $tmpdir/$filename --silent --force --fix-type consistency

		$BINARY --input tmpdir/$filename --analyze --silent
		if [ "$?" != "0" ]; then
			echo "Error: Consistency fix failed for $filename"
			rm -rf "$tmpdir"
			exit 1
		fi
	fi

	$BINARY --input $EXAMPLE_DATA_DIR/$filename --output $tmpdir/$filename --silent --force --fix-type tabs

	$BINARY --input $tmpdir/$filename --analyze --silent
	if [ "$?" != "0" ]; then
		echo "Error: Tab fix failed for $filename"
		rm -rf "$tmpdir"
		exit 1
	fi

	$BINARY --input $EXAMPLE_DATA_DIR/$filename --output $tmpdir/$filename --silent --force --fix-type spaces

	$BINARY --input $tmpdir/$filename --analyze --silent
	if [ "$?" == "1" ]; then
		echo "Error: Space fix failed for $filename"
		rm -rf "$tmpdir"
		exit 1
	fi

	$BINARY --input $EXAMPLE_DATA_DIR/$filename --output $tmpdir/$filename --silent --force --fix-type auto

	$BINARY --input $tmpdir/$filename --analyze --silent
	if [ "$?" != "0" ]; then
		echo "Error: Space fix failed for $filename"
		rm -rf "$tmpdir"
		exit 1
	fi
done

rm -rf "$tmpdir"

echo "Fix functionality checks done successfully"
exit 0
