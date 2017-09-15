#ifndef CLIENTVERSION_H
#define CLIENTVERSION_H

/**
 * client versioning and copyright year
 */

// These need to be macros, as version.cpp's and magi-qt.rc's voodoo requires it
#define CLIENT_VERSION_MAJOR       1
#define CLIENT_VERSION_MINOR       4
#define CLIENT_VERSION_REVISION    4
#define CLIENT_VERSION_BUILD       1

// Set to true for release, false for prerelease or test build
#define CLIENT_VERSION_IS_RELEASE  true
#define CLIENT_VERSION_IS_TEST     false

// Version before stable release: ALPHA1 ~ ALPHA3, BETA1 ~ BETA3, RC1 ~ RC3
// Set to "STABLE" for stable release
#define CLIENT_VERSION_RELEASE_CANDIDATE "RC1"

/**
 * Copyright year (2009-this)
 * Todo: update this when changing our copyright comments in the source
 */
#define COPYRIGHT_YEAR 2017

// Converts the parameter X to a string after macro replacement on X has been performed.
// Don't merge these into one macro!
#define STRINGIZE(X) DO_STRINGIZE(X)
#define DO_STRINGIZE(X) #X

//! Copyright string used in Windows .rc files
#define COPYRIGHT_STR "2014-" STRINGIZE(COPYRIGHT_YEAR) " The Magi Core Developers"

static const int CLIENT_VERSION =
                           1000000 * CLIENT_VERSION_MAJOR
                         +   10000 * CLIENT_VERSION_MINOR
                         +     100 * CLIENT_VERSION_REVISION
                         +       1 * CLIENT_VERSION_BUILD;

#endif // CLIENTVERSION_H
