#!/bin/bash

files_regex='\.(c|cpp|h|hpp)(?!.)'
ret=0

if [[ $EVENT == "push" ]] then
	CHANGED_FILES=$(git diff --name-only $BEFORE $AFTER | grep -P $files_regex)
else
	CHANGED_FILES=$(git diff --name-only HEAD~1 HEAD | grep -P $files_regex)
fi

echo "CHANGED: $CHANGED_FILES"

write_tag() {
	echo $1 >> $GITHUB_STEP_SUMMARY
	echo "" >> $GITHUB_STEP_SUMMARY
}

echo "# Formatter Details" >> $GITHUB_STEP_SUMMARY
for file in $CHANGED_FILES; do
	echo "Running Formatter in $file"
	docker run \
		-v $(pwd):$(pwd) \
		-w $(pwd) \
		format \
		"--dry-run" \
		$file 2> changes.txt
	changes_nbr=$(wc -l changes.txt)
	echo "Changes Nbr: $changes_nbr"
	if [[ "$changes_nbr" > 0 ]]; then
		write_tag "<details>"
		write_tag "<summary># $file</summary>"
		write_tag "\`\`\`c"
		cat changes.txt >> $GITHUB_STEP_SUMMARY 
		write_tag "\`\`\`"
		write_tag "</details>"
		write_tag "---" >> $GITHUB_STEP_SUMMARY
		ret=1
	fi
done

exit $ret
