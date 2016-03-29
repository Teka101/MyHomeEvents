#!/bin/sh

curl -H "Authorization: token $GIST_TOKEN" 'https://api.github.com/gists'
