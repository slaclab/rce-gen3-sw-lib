#!/bin/sh

echo "LONG URL, SHORT URL"

COOKIEFILE="/tmp/hwdb_cookie.cook"

for url in `cat $1`
do
    if [ -a $COOKIEFILE ]; then 
	short=`curl --silent --cookie $COOKIEFILE --cookie-jar $COOKIEFILE "$url" | grep shortlink | awk 'BEGIN{FS="href="}{print $2}' | sed 's/\"//g ; s/>//g'`
    else
	short=`curl --silent -u $USER --cookie $COOKIEFILE --cookie-jar $COOKIEFILE "$url" | grep shortlink | awk 'BEGIN{FS="href="}{print $2}' | sed 's/\"//g ; s/>//g'`
    fi
    echo "$url, $short"
done

rm $COOKIEFILE