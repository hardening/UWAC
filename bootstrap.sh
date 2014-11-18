#!/bin/sh
aclocal && \
	autoheader && \
	libtoolize && \
	automake -af && \
	autoconf
