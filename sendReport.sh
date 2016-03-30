#!/bin/sh

cat > report.json <<EOF
{
  "description": "Report Travis-CI $TRAVIS_JOB_NUMBER",
  "files": {
    "report.html" : { "content": "$(cat report.html | sed -r -e 's/"/\\"/g' | tr '\n' ' ')" }
  }
}
EOF

curl -X PATCH --data-binary @report.json -H "Authorization: token $GIST_TOKEN" 'https://api.github.com/gists/94c7b6a4408e1f601899'
