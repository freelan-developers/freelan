#!/bin/sh

/bin/launchctl unload /Library/LaunchDaemons/org.freelan.freelan.plist
rm -f /Library/LaunchDaemons/org.freelan.freelan.plist
rm -f /usr/local/sbin/freelan
rm -f /usr/local/etc/freelan/freelan.cfg
