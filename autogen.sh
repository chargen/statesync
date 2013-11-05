#!/bin/sh

echo "This will do all the autotools stuff so that you can build"
echo "statesync successfully. It will also call the ./configure script"
echo "and pass on any options which you passed to this script"
echo "See ./configure --help to know which options are available"
echo
echo "When it is finished, take the usual steps to install:"
echo "make"
echo "make install"
echo

(automake --version) < /dev/null > /dev/null 2>&1 || {
	echo;
	echo "You must have automake installed to compile statesync";
	echo;
	exit;
}

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
	echo;
	echo "You must have autoconf installed to compile statesync";
	echo;
	exit;
}

echo "Generating configuration files for statesync, please wait..."
echo;

# Autoconf will call run autopoint, aclocal, autoconf, autoheader and automake
# to setup and configure the build environment
autoreconf --verbose --install --symlink --force || {
    echo;
    echo "autoreconf has encountered an error."
    echo;
    exit $?
}
# Afterwards we invoke the configure skript, "$@" will contain the arguments that
# were passed to this skript.

echo
echo "Running configure now"
echo
./configure "$@"
