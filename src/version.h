// Copyright (c) 2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_VERSION_H
#define BITCOIN_VERSION_H

#include "clientversion.h"
#include <string>
#include <vector>

/**
 * database format versioning
 */
static const int DATABASE_VERSION = 72001;

/**
 * network protocol versioning
 */

// v1.4.5
static const int PROTOCOL_VERSION = 71064;
static const int MIN_PROTO_VERSION = 71062;

/* prior to v1.4.3
static const int PROTOCOL_VERSION = 71061;
static const int MIN_PROTO_VERSION = 71040;
*/

// nTime field added to CAddress, starting with this version;
// if possible, avoid requesting addresses nodes older than this
static const int CADDR_TIME_VERSION = 31402;

// only request blocks from nodes outside this range of versions
static const int NOBLKS_VERSION_START = 60002;
static const int NOBLKS_VERSION_END = 70900;

// BIP 0031, pong message, is enabled for all versions AFTER this one
static const int BIP0031_VERSION = 60000;

// "mempool" command, enhanced "getdata" behavior starts with this version:
static const int MEMPOOL_GD_VERSION = 60002;

extern const std::string CLIENT_NAME;
extern const std::string CLIENT_BUILD;
extern const std::string CLIENT_DATE;

std::string FormatFullVersion();
std::string FormatSubVersion(const std::string& name, int nClientVersion, const std::vector<std::string>& comments);

//#define DISPLAY_VERSION_MAJOR       1
//#define DISPLAY_VERSION_MINOR       3
//#define DISPLAY_VERSION_REVISION    0
//#define DISPLAY_VERSION_BUILD       0

#endif
