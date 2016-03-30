#!/bin/sh

REPORT_FILE=$1

[ -z "$GIST_TOKEN" ] && echo "Missing variable 'GIST_TOKEN'" && exit 1
[ -z "$GIST_ID" ] && echo "Missing variable 'GIST_ID'" && exit 1
[ -z "$REPORT_FILE" ] && echo "Missing argument: report filename" && exit 1

cat > report.json <<EOF
{
  "description": "Report Travis-CI $TRAVIS_JOB_NUMBER",
  "files": {
    "$REPORT_FILE" : { "content": "$(cat $REPORT_FILE | sed -r -e 's/"/\\"/g' -e 's/$/\\n/' | tr -d '\n')" }
  }
}
EOF

curl -X PATCH --data-binary @report.json -H "Authorization: token $GIST_TOKEN" "https://api.github.com/gists/$GIST_ID"
