--: ips-patcher :--
==============
Vincent 'MooZ' Cruz - 11/11/2014 (damn 7 years...)

mail : cruz.vincent AT gmail DOT com

See LICENSE file for the licence.

Building from sources
--------------
The makefile builds 2 binaries.
 * ips-patcher-cli a command line IPS patcher.
 * ips-patcher GTK3 IPS patcher.

They can be compile in release or debug mode. To compile in release mode
just type:

>make

For debug mode:

>make DEBUG=1

The binaries will be located in build/Release or build/Debug depending of
the chosen mode.

Command line
--------------
The usage for the command line IPS patcher is:

>ips-patcher-cli source patch destination

 * source source filename
 * patch IPS patch filename
 * destination filename

