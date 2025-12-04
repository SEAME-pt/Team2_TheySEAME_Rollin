#!/bin/bash

echo "CHANGED: $1"

write_tag() {
	echo $1 >> $GITHUB_STEP_SUMMARY
	echo "" >> $GITHUB_STEP_SUMMARY
}

echo "# Formatter Details" >> $GITHUB_STEP_SUMMARY
for file in $1; do
	echo "Running Formatter in $file"
	docker run \
		-v $(pwd):$(pwd) \
		-w $(pwd) \
		ghcr.io/seame-pt/team2_theyseame_rollin/clang-format_docker:latest \
		"--dry-run" \
		"--style=file" \
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
