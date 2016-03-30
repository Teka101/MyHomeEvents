#!/bin/sh

REPORT_FILE=$1

cat > report.json <<EOF
{
  "description": "Report Travis-CI $TRAVIS_JOB_NUMBER",
  "files": {
    "$REPORT_FILE" : { "content": "$(cat $REPORT_FILE | sed -r -e 's/"/\\"/g' -e 's/$/\\n/' | tr -d '\n')" }
  }
}
EOF

curl -X PATCH --data-binary @$REPORT_FILE -H "Authorization: token $GIST_TOKEN" 'https://api.github.com/gists/94c7b6a4408e1f601899'
