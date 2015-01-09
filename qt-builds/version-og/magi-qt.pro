TEMPLATE = app
TARGET = magi-qt
VERSION = 1.2.1.1
SRCDIR = ../../src
INCLUDEPATH += $$SRCDIR $$SRCDIR/json $$SRCDIR/qt-og
DEFINES += QT_GUI BOOST_THREAD_USE_LIB BOOST_SPIRIT_THREADSAFE
CONFIG += no_include_pwd
CONFIG += thread
CONFIG += static
QT += core gui network
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

# for boost 1.37, add -mt to the boost libraries
# use: qmake BOOST_LIB_SUFFIX=-mt
# for boost thread win32 with _win32 sufix
# use: BOOST_THREAD_LIB_SUFFIX=_win32-...
# or when linking against a specific BerkelyDB version: BDB_LIB_SUFFIX=-4.8

# Dependency library locations can be customized with:
#    BOOST_INCLUDE_PATH, BOOST_LIB_PATH, BDB_INCLUDE_PATH,
#    BDB_LIB_PATH, OPENSSL_INCLUDE_PATH and OPENSSL_LIB_PATH respectively

# winbuild dependencies
win32 {
BOOST_LIB_SUFFIX=-mgw49-mt-s-1_57
BOOST_INCLUDE_PATH=C:/deps/boost_1_57_0
BOOST_LIB_PATH=C:/deps/boost_1_57_0/stage/lib
BDB_INCLUDE_PATH=C:/deps/db-4.8.30.NC/build_unix
BDB_LIB_PATH=C:/deps/db-4.8.30.NC/build_unix
OPENSSL_INCLUDE_PATH=C:/deps/openssl-1.0.1j/include
OPENSSL_LIB_PATH=C:/deps/openssl-1.0.1j
MINIUPNPC_INCLUDE_PATH=C:/deps/
MINIUPNPC_LIB_PATH=C:/deps/miniupnpc
QRENCODE_INCLUDE_PATH=C:/deps/qrencode-3.4.4
QRENCODE_LIB_PATH=C:/deps/qrencode-3.4.4/.libs
GMP_INCLUDE_PATH=C:/deps/gmp-6.0.0
GMP_LIB_PATH=C:/deps/gmp-6.0.0/.libs
}

OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build

# use: qmake "RELEASE=1"
contains(RELEASE, 1) {
    # Mac: compile for maximum compatibility (10.5, 64-bit)
    macx:QMAKE_CXXFLAGS += -mmacosx-version-min=10.5 -arch x86_64 -isysroot $(xcode-select --print-path)/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.5.sdk

    !windows:!macx {
        # Linux: static link
        LIBS += -Wl,-Bstatic
    }
}

!win32 {
# for extra security against potential buffer overflows: enable GCCs Stack Smashing Protection
QMAKE_CXXFLAGS *= -fstack-protector-all --param ssp-buffer-size=1
QMAKE_LFLAGS *= -fstack-protector-all --param ssp-buffer-size=1
# We need to exclude this for Windows cross compile with MinGW 4.2.x, as it will result in a non-working executable!
# This can be enabled for Windows, when we switch to MinGW >= 4.4.x.
}
# for extra security on Windows: enable ASLR and DEP via GCC linker flags
#win32:QMAKE_LFLAGS *= -Wl,--dynamicbase -Wl,--nxcompat -Wl,--large-address-aware -static
win32:QMAKE_LFLAGS *= -Wl,--dynamicbase -Wl,--nxcompat -static
win32:QMAKE_LFLAGS += -static-libgcc -static-libstdc++

# use: qmake "USE_QRCODE=1"
# libqrencode (http://fukuchi.org/works/qrencode/index.en.html) must be installed for support
contains(USE_QRCODE, 1) {
    message(Building with QRCode support)
    DEFINES += USE_QRCODE
    LIBS += -lqrencode
}

# use: qmake "USE_UPNP=1" ( enabled by default; default)
#  or: qmake "USE_UPNP=0" (disabled by default)
#  or: qmake "USE_UPNP=-" (not supported)
# miniupnpc (http://miniupnp.free.fr/files/) must be installed for support
contains(USE_UPNP, -) {
    message(Building without UPNP support)
} else {
    message(Building with UPNP support)
    count(USE_UPNP, 0) {
        USE_UPNP=1
    }
    DEFINES += USE_UPNP=$$USE_UPNP STATICLIB
    INCLUDEPATH += $$MINIUPNPC_INCLUDE_PATH
    LIBS += $$join(MINIUPNPC_LIB_PATH,,-L,) -lminiupnpc
    win32:LIBS += -liphlpapi
}


# use: qmake "USE_DBUS=1"
contains(USE_DBUS, 1) {
    message(Building with DBUS (Freedesktop notifications) support)
    DEFINES += USE_DBUS
    QT += dbus
}

# use: qmake "USE_IPV6=1" ( enabled by default; default)
#  or: qmake "USE_IPV6=0" (disabled by default)
#  or: qmake "USE_IPV6=-" (not supported)
contains(USE_IPV6, -) {
    message(Building without IPv6 support)
} else {
    message(Building with IPv6 support)
    count(USE_IPV6, 0) {
        USE_IPV6=1
    }
    DEFINES += USE_IPV6=$$USE_IPV6
}

contains(BITCOIN_NEED_QT_PLUGINS, 1) {
    DEFINES += BITCOIN_NEED_QT_PLUGINS
    QTPLUGIN += qcncodecs qjpcodecs qtwcodecs qkrcodecs qtaccessiblewidgets
}


# regenerate $$SRCDIR/build.h
!windows|contains(USE_BUILD_INFO, 1) {
    genbuild.depends = FORCE
    genbuild.commands = cd $$PWD; /bin/sh share/genbuild.sh $$OUT_PWD/build/build.h
    genbuild.target = $$OUT_PWD/build/build.h
    PRE_TARGETDEPS += $$OUT_PWD/build/build.h
    QMAKE_EXTRA_TARGETS += genbuild
    DEFINES += HAVE_BUILD_INFO
}

QMAKE_CXXFLAGS += -msse2
QMAKE_CFLAGS += -msse2
QMAKE_CXXFLAGS_WARN_ON = -fdiagnostics-show-option -Wall -Wextra -Wformat -Wformat-security -Wno-unused-parameter -Wstack-protector

# Input
DEPENDPATH += $$SRCDIR $$SRCDIR/json $$SRCDIR/qt-og
HEADERS += $$SRCDIR/qt-og/bitcoingui.h \
    $$SRCDIR/qt-og/transactiontablemodel.h \
    $$SRCDIR/qt-og/addresstablemodel.h \
    $$SRCDIR/qt-og/optionsdialog.h \
    $$SRCDIR/qt-og/coincontroldialog.h \
    $$SRCDIR/qt-og/coincontroltreewidget.h \
    $$SRCDIR/qt-og/sendcoinsdialog.h \
    $$SRCDIR/qt-og/addressbookpage.h \
    $$SRCDIR/qt-og/signverifymessagedialog.h \
    $$SRCDIR/qt-og/aboutdialog.h \
    $$SRCDIR/qt-og/editaddressdialog.h \
    $$SRCDIR/qt-og/bitcoinaddressvalidator.h \
    $$SRCDIR/alert.h \
    $$SRCDIR/addrman.h \
    $$SRCDIR/base58.h \
    $$SRCDIR/bignum.h \
    $$SRCDIR/checkpoints.h \
    $$SRCDIR/compat.h \
    $$SRCDIR/coincontrol.h \
    $$SRCDIR/sync.h \
    $$SRCDIR/util.h \
    $$SRCDIR/hash.h \
    $$SRCDIR/uint256.h \
    $$SRCDIR/kernel.h \
    $$SRCDIR/scrypt_mine.h \
    $$SRCDIR/pbkdf2.h \
    $$SRCDIR/serialize.h \
    $$SRCDIR/strlcpy.h \
    $$SRCDIR/main.h \
    $$SRCDIR/net.h \
    $$SRCDIR/key.h \
    $$SRCDIR/db.h \
    $$SRCDIR/walletdb.h \
    $$SRCDIR/script.h \
    $$SRCDIR/init.h \
    $$SRCDIR/irc.h \
    $$SRCDIR/mruset.h \
    $$SRCDIR/magimath.h \
    $$SRCDIR/json/json_spirit_writer_template.h \
    $$SRCDIR/json/json_spirit_writer.h \
    $$SRCDIR/json/json_spirit_value.h \
    $$SRCDIR/json/json_spirit_utils.h \
    $$SRCDIR/json/json_spirit_stream_reader.h \
    $$SRCDIR/json/json_spirit_reader_template.h \
    $$SRCDIR/json/json_spirit_reader.h \
    $$SRCDIR/json/json_spirit_error_position.h \
    $$SRCDIR/json/json_spirit.h \
    $$SRCDIR/qt-og/clientmodel.h \
    $$SRCDIR/qt-og/guiutil.h \
    $$SRCDIR/qt-og/transactionrecord.h \
    $$SRCDIR/qt-og/guiconstants.h \
    $$SRCDIR/qt-og/optionsmodel.h \
    $$SRCDIR/qt-og/monitoreddatamapper.h \
    $$SRCDIR/qt-og/transactiondesc.h \
    $$SRCDIR/qt-og/transactiondescdialog.h \
    $$SRCDIR/qt-og/bitcoinamountfield.h \
    $$SRCDIR/wallet.h \
    $$SRCDIR/keystore.h \
    $$SRCDIR/qt-og/transactionfilterproxy.h \
    $$SRCDIR/qt-og/transactionview.h \
    $$SRCDIR/qt-og/walletmodel.h \
    $$SRCDIR/bitcoinrpc.h \
    $$SRCDIR/qt-og/overviewpage.h \
    $$SRCDIR/qt-og/csvmodelwriter.h \
    $$SRCDIR/crypter.h \
    $$SRCDIR/qt-og/sendcoinsentry.h \
    $$SRCDIR/qt-og/qvalidatedlineedit.h \
    $$SRCDIR/qt-og/bitcoinunits.h \
    $$SRCDIR/qt-og/qvaluecombobox.h \
    $$SRCDIR/qt-og/askpassphrasedialog.h \
    $$SRCDIR/protocol.h \
    $$SRCDIR/qt-og/notificator.h \
    $$SRCDIR/qt-og/qtipcserver.h \
    $$SRCDIR/allocators.h \
    $$SRCDIR/ui_interface.h \
    $$SRCDIR/qt-og/rpcconsole.h \
    $$SRCDIR/version.h \
    $$SRCDIR/netbase.h \
    $$SRCDIR/clientversion.h \
    $$SRCDIR/hash_magi.h \
    $$SRCDIR/hash/sph_types.h \
    $$SRCDIR/hash/sph_keccak.h \
    $$SRCDIR/hash/sph_haval.h \
    $$SRCDIR/hash/sph_ripemd.h \
    $$SRCDIR/hash/sph_sha2.h \
    $$SRCDIR/hash/sph_tiger.h \
    $$SRCDIR/hash/sph_whirlpool.h

SOURCES += $$SRCDIR/qt-og/bitcoin.cpp $$SRCDIR/qt-og/bitcoingui.cpp \
    $$SRCDIR/qt-og/transactiontablemodel.cpp \
    $$SRCDIR/qt-og/addresstablemodel.cpp \
    $$SRCDIR/qt-og/optionsdialog.cpp \
    $$SRCDIR/qt-og/sendcoinsdialog.cpp \
    $$SRCDIR/qt-og/coincontroldialog.cpp \
    $$SRCDIR/qt-og/coincontroltreewidget.cpp \
    $$SRCDIR/qt-og/addressbookpage.cpp \
    $$SRCDIR/qt-og/signverifymessagedialog.cpp \
    $$SRCDIR/qt-og/aboutdialog.cpp \
    $$SRCDIR/qt-og/editaddressdialog.cpp \
    $$SRCDIR/qt-og/bitcoinaddressvalidator.cpp \
    $$SRCDIR/alert.cpp \
    $$SRCDIR/version.cpp \
    $$SRCDIR/sync.cpp \
    $$SRCDIR/util.cpp \
    $$SRCDIR/hash.cpp \
    $$SRCDIR/netbase.cpp \
    $$SRCDIR/key.cpp \
    $$SRCDIR/script.cpp \
    $$SRCDIR/main.cpp \
    $$SRCDIR/init.cpp \
    $$SRCDIR/net.cpp \
    $$SRCDIR/irc.cpp \
    $$SRCDIR/checkpoints.cpp \
    $$SRCDIR/addrman.cpp \
    $$SRCDIR/db.cpp \
    $$SRCDIR/walletdb.cpp \
    $$SRCDIR/magimath.cpp \
    $$SRCDIR/qt-og/clientmodel.cpp \
    $$SRCDIR/qt-og/guiutil.cpp \
    $$SRCDIR/qt-og/transactionrecord.cpp \
    $$SRCDIR/qt-og/optionsmodel.cpp \
    $$SRCDIR/qt-og/monitoreddatamapper.cpp \
    $$SRCDIR/qt-og/transactiondesc.cpp \
    $$SRCDIR/qt-og/transactiondescdialog.cpp \
    $$SRCDIR/qt-og/bitcoinstrings.cpp \
    $$SRCDIR/qt-og/bitcoinamountfield.cpp \
    $$SRCDIR/wallet.cpp \
    $$SRCDIR/keystore.cpp \
    $$SRCDIR/qt-og/transactionfilterproxy.cpp \
    $$SRCDIR/qt-og/transactionview.cpp \
    $$SRCDIR/qt-og/walletmodel.cpp \
    $$SRCDIR/bitcoinrpc.cpp \
    $$SRCDIR/rpcdump.cpp \
    $$SRCDIR/rpcnet.cpp \
    $$SRCDIR/rpcmining.cpp \
    $$SRCDIR/rpcwallet.cpp \
    $$SRCDIR/rpcblockchain.cpp \
    $$SRCDIR/rpcrawtransaction.cpp \
    $$SRCDIR/qt-og/overviewpage.cpp \
    $$SRCDIR/qt-og/csvmodelwriter.cpp \
    $$SRCDIR/crypter.cpp \
    $$SRCDIR/qt-og/sendcoinsentry.cpp \
    $$SRCDIR/qt-og/qvalidatedlineedit.cpp \
    $$SRCDIR/qt-og/bitcoinunits.cpp \
    $$SRCDIR/qt-og/qvaluecombobox.cpp \
    $$SRCDIR/qt-og/askpassphrasedialog.cpp \
    $$SRCDIR/protocol.cpp \
    $$SRCDIR/qt-og/notificator.cpp \
    $$SRCDIR/qt-og/qtipcserver.cpp \
    $$SRCDIR/qt-og/rpcconsole.cpp \
    $$SRCDIR/noui.cpp \
    $$SRCDIR/kernel.cpp \
    $$SRCDIR/pbkdf2.cpp \
    $$SRCDIR/hash/keccak.cpp \
    $$SRCDIR/hash/haval.cpp \
    $$SRCDIR/hash/ripemd.cpp \
    $$SRCDIR/hash/sha2.cpp \
    $$SRCDIR/hash/sha2big.cpp \
    $$SRCDIR/hash/tiger.cpp \
    $$SRCDIR/hash/whirlpool.cpp

RESOURCES += \
    $$SRCDIR/qt-og/bitcoin.qrc

FORMS += \
    $$SRCDIR/qt-og/forms/coincontroldialog.ui \
    $$SRCDIR/qt-og/forms/sendcoinsdialog.ui \
    $$SRCDIR/qt-og/forms/addressbookpage.ui \
    $$SRCDIR/qt-og/forms/signverifymessagedialog.ui \
    $$SRCDIR/qt-og/forms/aboutdialog.ui \
    $$SRCDIR/qt-og/forms/editaddressdialog.ui \
    $$SRCDIR/qt-og/forms/transactiondescdialog.ui \
    $$SRCDIR/qt-og/forms/overviewpage.ui \
    $$SRCDIR/qt-og/forms/sendcoinsentry.ui \
    $$SRCDIR/qt-og/forms/askpassphrasedialog.ui \
    $$SRCDIR/qt-og/forms/rpcconsole.ui \
    $$SRCDIR/qt-og/forms/optionsdialog.ui

contains(USE_QRCODE, 1) {
HEADERS += $$SRCDIR/qt-og/qrcodedialog.h
SOURCES += $$SRCDIR/qt-og/qrcodedialog.cpp
FORMS += $$SRCDIR/qt-og/forms/qrcodedialog.ui
}

contains(BITCOIN_QT_TEST, 1) {
SOURCES += $$SRCDIR/qt-og/test/test_main.cpp \
    $$SRCDIR/qt-og/test/uritests.cpp
HEADERS += $$SRCDIR/qt-og/test/uritests.h
DEPENDPATH += $$SRCDIR/qt-og/test
QT += testlib
TARGET = magi-qt_test
DEFINES += BITCOIN_QT_TEST
}

CODECFORTR = UTF-8

# for lrelease/lupdate
# also add new translations to $$SRCDIR/qt-og/bitcoin.qrc under translations/
TRANSLATIONS = $$files($$SRCDIR/qt-og/locale/bitcoin_*.ts)

isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
isEmpty(QM_DIR):QM_DIR = $$PWD/$$SRCDIR/qt-og/locale
# automatically build translations, so they can be included in resource file
TSQM.name = lrelease ${QMAKE_FILE_IN}
TSQM.input = TRANSLATIONS
TSQM.output = $$QM_DIR/${QMAKE_FILE_BASE}.qm
TSQM.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
TSQM.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += TSQM

# "Other files" to show in Qt Creator
OTHER_FILES += \
    doc/*.rst doc/*.txt doc/README README.md res/bitcoin-qt.rc $$SRCDIR/test/*.cpp $$SRCDIR/test/*.h $$SRCDIR/qt-og/test/*.cpp $$SRCDIR/qt-og/test/*.h

# platform specific defaults, if not overridden on command line
isEmpty(BOOST_LIB_SUFFIX) {
    macx:BOOST_LIB_SUFFIX = -mt
    windows:BOOST_LIB_SUFFIX = -mgw48-mt-s-1_55
}

isEmpty(BOOST_THREAD_LIB_SUFFIX) {
    BOOST_THREAD_LIB_SUFFIX = $$BOOST_LIB_SUFFIX
}

isEmpty(BDB_LIB_PATH) {
    macx:BDB_LIB_PATH = /opt/local/lib/db48
}

isEmpty(BDB_LIB_SUFFIX) {
    macx:BDB_LIB_SUFFIX = -4.8
}

isEmpty(BDB_INCLUDE_PATH) {
    macx:BDB_INCLUDE_PATH = /opt/local/include/db48
}

isEmpty(BOOST_LIB_PATH) {
    macx:BOOST_LIB_PATH = /opt/local/lib
}

isEmpty(BOOST_INCLUDE_PATH) {
    macx:BOOST_INCLUDE_PATH = /opt/local/include
}

windows:DEFINES += WIN32
windows:RC_FILE = $$SRCDIR/qt-og/res/bitcoin-qt.rc

windows:!contains(MINGW_THREAD_BUGFIX, 0) {
    # At least qmake's win32-g++-cross profile is missing the -lmingwthrd
    # thread-safety flag. GCC has -mthreads to enable this, but it doesn't
    # work with static linking. -lmingwthrd must come BEFORE -lmingw, so
    # it is prepended to QMAKE_LIBS_QT_ENTRY.
    # It can be turned off with MINGW_THREAD_BUGFIX=0, just in case it causes
    # any problems on some untested qmake profile now or in the future.
    DEFINES += _MT
    QMAKE_LIBS_QT_ENTRY = -lmingwthrd $$QMAKE_LIBS_QT_ENTRY
}

!windows:!macx {
    DEFINES += LINUX
    LIBS += -lrt
}

macx:HEADERS += $$SRCDIR/qt/macdockiconhandler.h $$SRCDIR/qt/macnotificationhandler.h
macx:OBJECTIVE_SOURCES += $$SRCDIR/qt/macdockiconhandler.mm $$SRCDIR/qt/macnotificationhandler.mm
macx:LIBS += -framework Foundation -framework ApplicationServices -framework AppKit -framework CoreServices
macx:DEFINES += MAC_OSX MSG_NOSIGNAL=0
macx:ICON = $$SRCDIR/qt-og/res/icons/magi.icns
macx:TARGET = "Magi-Qt"
macx:QMAKE_CFLAGS_THREAD += -pthread
macx:QMAKE_LFLAGS_THREAD += -pthread
macx:QMAKE_CXXFLAGS_THREAD += -pthread

# Set libraries and includes at end, to use platform-defined defaults if not overridden
INCLUDEPATH += $$BOOST_INCLUDE_PATH $$BDB_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH $$QRENCODE_INCLUDE_PATH $$GMP_INCLUDE_PATH
LIBS += $$join(BOOST_LIB_PATH,,-L,) $$join(BDB_LIB_PATH,,-L,) $$join(OPENSSL_LIB_PATH,,-L,) $$join(QRENCODE_LIB_PATH,,-L,) $$join(GMP_LIB_PATH,,-L,)
LIBS += -lssl -lgmp -lcrypto -ldb_cxx$$BDB_LIB_SUFFIX
# -lgdi32 has to happen after -lcrypto (see  #681)
windows:LIBS += -lws2_32 -lshlwapi -lmswsock -lole32 -loleaut32 -luuid -lgdi32
LIBS += -lboost_system$$BOOST_LIB_SUFFIX -lboost_filesystem$$BOOST_LIB_SUFFIX -lboost_program_options$$BOOST_LIB_SUFFIX -lboost_thread$$BOOST_THREAD_LIB_SUFFIX
windows:LIBS += -lboost_chrono$$BOOST_LIB_SUFFIX

contains(RELEASE, 1) {
    !windows:!macx {
        # Linux: turn dynamic linking back on for c/c++ runtime libraries
        LIBS += -Wl,-Bdynamic
    }
}

system($$QMAKE_LRELEASE -silent $$_PRO_FILE_)
