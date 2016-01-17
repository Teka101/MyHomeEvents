#!/bin/sh

TRANSMISSION_IP='192.168.0.1'
TRANSMISSION_PORT='9091';
TRANSMISSION_URL='/transmission';
DEBUG=0

REQUEST_CSRF=$(cat <<_EOF
POST ${TRANSMISSION_URL}/rpc HTTP/1.1
Host: ${TRANSMISSION_IP}:${TRANSMISSION_PORT}
User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:39.0) Gecko/20100101 Firefox/39.0
Accept: application/json, text/javascript, */*; q=0.01
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate
DNT: 1
Content-Type: json; charset=UTF-8
X-Requested-With: XMLHttpRequest
Referer: http://${TRANSMISSION_IP}:${TRANSMISSION_PORT}${TRANSMISSION_URL}/web/
Content-Length: 24
Connection: keep-alive
Pragma: no-cache
Cache-Control: no-cache

{"method":"session-get"}
_EOF
)

[ $DEBUG = '2' ] && echo "$REQUEST_CSRF"
CSRF_TOKEN=$(echo "$REQUEST_CSRF" | nc $TRANSMISSION_IP $TRANSMISSION_PORT | grep -e '^X-Transmission-Session-Id:' | cut -d ':' -f2  | tr -d '[[:space:]]')
[ $DEBUG = '1' ] && echo "Token CSRF: $CSRF_TOKEN"

if [ "x$CSRF_TOKEN" = "x" ] ; then
	echo "Application transmission pas exécutée."
	exit 1
fi

REQUEST_DOWN=$(cat << EOF
POST ${TRANSMISSION_URL}/rpc HTTP/1.1
Host: ${TRANSMISSION_IP}:${TRANSMISSION_PORT}
User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:39.0) Gecko/20100101 Firefox/39.0
Accept: application/json, text/javascript, */*; q=0.01
Accept-Language: en-US,en;q=0.5
DNT: 1
Content-Type: json; charset=UTF-8
X-Transmission-Session-Id: ${CSRF_TOKEN}
X-Requested-With: XMLHttpRequest
Referer: http://${TRANSMISSION_IP}:${TRANSMISSION_PORT}${TRANSMISSION_URL}/web/
Content-Length: 221
Connection: close
Pragma: no-cache
Cache-Control: no-cache

{"method":"torrent-get","arguments":{"fields":["id","addedDate","name","totalSize","error","errorString"
,"eta","isFinished","isStalled","leftUntilDone","metadataPercentComplete","percentDone","sizeWhenDone","status"
]}}
EOF
)
[ $DEBUG = '2' ] && echo "$REQUEST_DOWN"

RESPONSE_DOWN=$(echo "$REQUEST_DOWN" | nc $TRANSMISSION_IP $TRANSMISSION_PORT)
[ $DEBUG = '2' ] && echo "$RESPONSE_DOWN"

TRANSMISSION_JSON=$(echo "$RESPONSE_DOWN" | tail --lines=+8 | json_pp -f json -t json -json_opt indent | perl -e 'while (<STDIN>){s/("percentDone":)([^,]*)(,)/"$1".($2*100)."$3"/e; print};')
[ $DEBUG = '1' ] && echo "$TRANSMISSION_JSON"

FILES=$(echo "$TRANSMISSION_JSON" | awk 'BEGIN { RS = "{" ; FS = "\n" ; FPAT=".*:(.*)," } { if ($7!=""){print $2,$5,$7} }' | sed -r -e '/"errorString":"(.+)"/ s/^.*"percentDone":(.*),.*"name":"(.*)".*"errorString":"(.*)".*$/\2 erreur \3./ ; s/^.*"percentDone":(.*),.*"name":"(.*)".*"errorString":"(.*)".*$/\2 à \1%/')

echo "$FILES" | sed -r -e 's/\.(mp4|avi|divx)//' | sed -r -e 's/\[(720p|1080p|DTS|AC3|DTS-HD)\]//g' | sed -r -e 's/(fullhd|FHD)//g' | sed -r -s 's/(\s\s+)/ /g';

exit 0
