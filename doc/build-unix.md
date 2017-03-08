Copyright (c) 2009-2012 Bitcoin Developers

Copyright (c) 2014-2015 Magi Developers

Distributed under the MIT/X11 software license, see the accompanying file license.txt or http://www.opensource.org/licenses/mit-license.php. This product includes software developed by the OpenSSL Project for use in the OpenSSL Toolkit (http://www.openssl.org/).  This product includes cryptographic software written by Eric Young (eay@cryptsoft.com) and UPnP
software written by Thomas Bernard.

UNIX BUILD NOTES
================

It is recommeded to use Berkeley DB 4.8 for building Magi wallet (see the following instructions) as well as OpenSSL 1.0.x. 

Build magid
================

To Build On i386, amd64
--------
	cd src/
	make -f makefile.unix                          # Headless magi

To Build On armv6l
--------
	cd src/
	make -f makefile.unix xCPUARCH=armv6l           # Headless magi

To Build On armv7l
--------
	cd src/
	make -f makefile.unix xCPUARCH=armv7l           # Headless magi

The release is built with GCC and then "strip bitcoind" to strip the debug symbols, which reduces the executable size by about 90%.

Build magi-qt
================

See readme-qt.rst for instructions on building Magi-QT. In general, the QT wallet can be built through following commands (provided dependencies are properly installed):
	qmake magi-qt.pro
	make

To compile Berkeley DB 4.8 on your own:

```bash
wget 'http://download.oracle.com/berkeley-db/db-4.8.30.NC.tar.gz'
# Verify source
echo '12edc0df75bf9abd7f82f821795bcee50f42cb2e5f76a6a281b85732798364ef  db-4.8.30.NC.tar.gz' | sha256sum -c
tar -xzvf db-4.8.30.NC.tar.gz
cd db-4.8.30.NC/build_unix
# Build and install DB4.8 under /opt/local/db-4.8.30.NC
../dist/configure --disable-shared --enable-cxx --disable-replication --with-pic --prefix=/opt/local/db-4.8.30.NC
make
sudo make install
```
Then Build Qt wallet with explicit DB paths:

	qmake magi-qt.pro BDB_INCLUDE_PATH=/opt/local/db-4.8.30.NC/include BDB_LIB_PATH=/opt/local/db-4.8.30.NC/lib BDB_LIB_SUFFIX=-4.8
	make


Dependencies for i386, amd64
------------

 Library      | Purpose           | Description
 -------------|-------------------|----------------------
 libssl       | SSL Support       | Secure communications
 libdb4.8     | Berkeley DB       | Blockchain & wallet storage
 libboost     | Boost             | C++ Library
 libminiupnpc | UPnP Support      | Optional firewall-jumping support
 libqrencode  | QRCode generation | Optional QRCode generation
 libgmp       | GMP               | Multiple precision arithmetic library

Dependencies for armv6l, armv7l
------------

 Library      | Purpose           Description
 -------------|-------------------|----------------------
 libssl       | SSL Support       | Secure communications
 libdb4.8     | Berkeley DB       | Blockchain & wallet storage
 libboost     | Boost             | C++ Library
 libminiupnpc | UPnP Support      | Optional firewall-jumping support
 libqrencode  | QRCode generation | Optional QRCode generation
 libgmp       | GMP               | Multiple precision arithmetic library

Note that libexecinfo should be installed, if you building under *BSD systems. 
This library provides backtrace facility.

miniupnpc may be used for UPnP port mapping.  It can be downloaded from
http://miniupnp.tuxfamily.org/files/.  UPnP support is compiled in and
turned off by default.  Set USE_UPNP to a different value to control this:

	USE_UPNP=-    No UPnP support - miniupnp not required
	USE_UPNP=0    (the default) UPnP support turned off by default at runtime
	USE_UPNP=1    UPnP support turned on by default at runtime

libqrencode may be used for QRCode image generation. It can be downloaded
from http://fukuchi.org/works/qrencode/index.html.en, or installed via
your package manager. Set USE_QRCODE to control this:

	USE_QRCODE=0   (the default) No QRCode support - libqrcode not required
	USE_QRCODE=1   QRCode support enabled

Dependency Build Instructions: Ubuntu & Debian (i386, amd64)
----------------------------------------------
See above db4.8 building if you cannot find it through apt installation. The version libgmp may change, for example, libgmp3-dev as in the latest Debian distribution, you'll need to make sure the right version to be installed. 

	sudo apt-get install build-essential libtool autotools-dev autoconf automake
	sudo apt-get install libssl-dev
	sudo apt-get install libdb4.8-dev
	sudo apt-get install libdb4.8++-dev
	sudo apt-get install libgmp-dev
	sudo apt-get install libminiupnpc-dev
	sudo apt-get install libboost-all-dev
	sudo apt-get install libprotobuf-dev libqrencode-dev

Dependency Build Instructions: Ubuntu & Debian (armv7l)
----------------------------------------------
See above db4.8 building if you cannot find it through apt installation. The version libgmp may change, for example, libgmp3-dev as in the latest Debian distribution, you'll need to make sure the right version to be installed. 

	sudo apt-get install build-essential
	sudo apt-get install libssl-dev
	sudo apt-get install libdb4.8-dev
	sudo apt-get install libdb4.8++-dev
	sudo apt-get install libgmp-dev
	sudo apt-get install libminiupnpc-dev
	sudo apt-get install libboost-all-dev
	sudo apt-get install libprotobuf-dev libqrencode-dev

Security
--------
To help make your bitcoin installation more secure by making certain attacks impossible to
exploit even if a vulnerability is found, you can take the following measures:

* Position Independent Executable
    Build position independent code to take advantage of Address Space Layout Randomization
    offered by some kernels. An attacker who is able to cause execution of code at an arbitrary
    memory location is thwarted if he doesn't know where anything useful is located.
    The stack and heap are randomly located by default but this allows the code section to be
    randomly located as well.

    On an Amd64 processor where a library was not compiled with -fPIC, this will cause an error
    such as: "relocation R_X86_64_32 against `......' can not be used when making a shared object;"

    To build with PIE, use:
    make -f makefile.unix ... -e PIE=1

    To test that you have built PIE executable, install scanelf, part of paxutils, and use:
    scanelf -e ./bitcoin

    The output should contain:
     TYPE
    ET_DYN

* Non-executable Stack
    If the stack is executable then trivial stack based buffer overflow exploits are possible if
    vulnerable buffers are found. By default, bitcoin should be built with a non-executable stack
    but if one of the libraries it uses asks for an executable stack or someone makes a mistake
    and uses a compiler extension which requires an executable stack, it will silently build an
    executable without the non-executable stack protection.

    To verify that the stack is non-executable after compiling use:
    scanelf -e ./bitcoin

    the output should contain:
    STK/REL/PTL
    RW- R-- RW-

    The STK RW- means that the stack is readable and writeable but not executable.
