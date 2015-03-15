#!/bin/sh
### BEGIN INIT INFO
# Provides:          freelan
# Required-Start:    $network $remote_fs $syslog
# Required-Stop:     $network $remote_fs $syslog
# Should-Start:      network-manager
# Should-Stop:       network-manager
# X-Start-Before:    $x-display-manager gdm kdm xdm wdm ldm sdm nodm
# X-Interactive:     true
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Freelan VPN service
# Description: This script will start Freelan tunnels as specified in /etc/default/freelan and /etc/freelan/*.conf
### END INIT INFO

# Author: Julien Kauffmann <julien.kauffmann@freelan.org>

PATH=/sbin:/usr/sbin:/bin:/usr/bin
DESC="peer-to-peer virtual private network daemon"
NAME="freelan"
DAEMON=/usr/sbin/freelan
DAEMON_ARGS=""
PIDFILE=/var/run/$NAME.pid
SCRIPTNAME=/etc/init.d/$NAME
CONFIG_DIR=/etc/$NAME

# Exit if the package is not installed
[ -x $DAEMON ] || exit 0

# Defaults
CONFIGURATIONS=""

# Read configuration variable file if it is present
[ -r /etc/default/$NAME ] && . /etc/default/$NAME

# Load the VERBOSE setting and other rcS variables
. /lib/init/vars.sh

# We force the VERBOSE setting
VERBOSE=1

# Define LSB log_* functions.
# Depend on lsb-base (>= 3.0-6) to ensure that this file is present.
. /lib/lsb/init-functions

do_start_instance()
{
	# Return
	#   0 if daemon has been started
	#   1 if daemon was already running
	#   2 if daemon could not be started
	start-stop-daemon --start --quiet --pidfile $PIDFILE.$CONFIG --exec $DAEMON --test > /dev/null \
		|| return 1
	start-stop-daemon --start --quiet --pidfile $PIDFILE.$CONFIG --exec $DAEMON -- \
		$DAEMON_ARGS -p $PIDFILE.$CONFIG -c $CONFIG_FILE \
		|| return 2

	# Add code here, if necessary, that waits for the process to be ready
	# to handle requests from services started subsequently which depend
	# on this one.  As a last resort, sleep for some time.
}

do_stop_instance()
{
	# Return
	#   0 if daemon has been stopped
	#   1 if daemon was already stopped
	#   2 if daemon could not be stopped
	#   other if a failure occurred
	start-stop-daemon --stop --quiet --retry=TERM/30/KILL/5 --pidfile $PIDFILE.$CONFIG --name $NAME
	RETVAL="$?"
	[ "$RETVAL" = 2 ] && return 2
	# Wait for children to finish too if this is a daemon that forks
	# and if the daemon is only ever run from this initscript.
	# If the above conditions are not satisfied then add some other code
	# that waits for the process to drop all resources that could be
	# needed by services started subsequently.  A last resort is to
	# sleep for some time.
	#start-stop-daemon --stop --quiet --oknodo --retry=0/30/KILL/5 --exec $DAEMON
	#[ "$?" = 2 ] && return 2
	# Many daemons don't delete their pidfiles when they exit.
	rm -f $PIDFILE.$CONFIG
	return "$RETVAL"
}

#
# Function that starts the daemon/service
#
do_start()
{
	if [ "$CONFIGURATIONS" = "" ]; then
		[ "$VERBOSE" != no ] && log_warning_msg "$NAME: No configuration specified. Did you edit /etc/default/$NAME ?"
		return 0
	else
		for CONFIG in $CONFIGURATIONS; do
			[ "$VERBOSE" != no ] && log_daemon_msg "Starting $NAME instance - $CONFIG"

			CONFIG_FILE="$CONFIG_DIR/$CONFIG.conf"

			if test -e "$CONFIG_FILE"; then
				do_start_instance

				RETVAL="$?"
				[ "$RETVAL" = 2 ] && [ "$VERBOSE" != no ] && log_progress_msg "failed"
				[ "$RETVAL" = 2 ] && [ "$VERBOSE" != no ] && log_end_msg 1
				[ "$RETVAL" = 2 ] && return 2

				[ "$RETVAL" = 1 ] && [ "$VERBOSE" != no ] && log_progress_msg "already running - doing nothing"
			else
				[ "$VERBOSE" != no ] && log_progress_msg "$CONFIG_FILE not found"
				[ "$VERBOSE" != no ] && log_end_msg 1
				return 2
			fi

			[ "$VERBOSE" != no ] && log_end_msg 0
		done
	fi
}

#
# Function that stops the daemon/service
#
do_stop()
{
	if [ "$CONFIGURATIONS" = "" ]; then
		[ "$VERBOSE" != no ] && log_warning_msg "$NAME: No configuration specified. Did you edit /etc/default/$NAME ?"
		return 0
	else
		for CONFIG in $CONFIGURATIONS; do
			[ "$VERBOSE" != no ] && log_daemon_msg "Stopping $NAME instance - $CONFIG"

			CONFIG_FILE="$CONFIG_DIR/$CONFIG.conf"

			if test -e "$CONFIG_FILE"; then
				do_stop_instance

				RETVAL="$?"
				[ "$RETVAL" = 2 ] && [ "$VERBOSE" != no ] && log_progress_msg "failed"
				[ "$RETVAL" = 2 ] && [ "$VERBOSE" != no ] && log_end_msg 1
				[ "$RETVAL" = 2 ] && return 2

				[ "$RETVAL" = 1 ] && [ "$VERBOSE" != no ] && log_progress_msg "not running - doing nothing"
			else
				[ "$VERBOSE" != no ] && log_progress_msg "$CONFIG_FILE not found"
				[ "$VERBOSE" != no ] && log_end_msg 1
				return 2
			fi

			[ "$VERBOSE" != no ] && log_end_msg 0
		done
	fi
}

case "$1" in
	start)
		[ "$VERBOSE" != no ] && log_daemon_msg "Starting $NAME - $DESC"
		[ "$VERBOSE" != no ] && log_end_msg 0
		do_start
		exit $?
		;;
	stop)
		[ "$VERBOSE" != no ] && log_daemon_msg "Stopping $NAME - $DESC"
		[ "$VERBOSE" != no ] && log_end_msg 0
		do_stop
		exit $?
		;;
	status)
		status_of_proc "$DAEMON" "$NAME" && exit 0 || exit $?
		;;
	restart|force-reload)
		#
		# If the "reload" option is implemented then remove the
		# 'force-reload' alias
		#
		do_stop
		case "$?" in
			0|1)
				do_start
				;;
		esac
		;;
	*)
		echo "Usage: $SCRIPTNAME {start|stop|status|restart|force-reload}" >&2
		exit 3
		;;
esac

:
