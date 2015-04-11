#-------------------------------------------------
#
# Project created by QtCreator 2014-04-26T16:17:15
#
#-------------------------------------------------

QT       += core gui network webkit webkitwidgets printsupport webkit-private ftp


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CyberDragon
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dialog.cpp \
    browser.cpp \
    mynetworkaccessmanager.cpp \
    general.cpp \
    trackerblocker.cpp \
    cookie.cpp \
    encryption.cpp \
    cache.cpp \
    downloads.cpp \
    notification.cpp \
    mywebpage.cpp \
    mytreewidget.cpp \
    proxy.cpp \
    mycookiejar.cpp \
    ftpreply.cpp \
    mywebview.cpp \
    worker.cpp \
    sandboxreply.cpp \
    log.cpp \
    plugins.cpp \
    serverinfo.cpp \
    cookieviewdelegate.cpp

HEADERS  += mainwindow.h \
    dialog.h \
    browser.h \
    mynetworkaccessmanager.h \
    general.h \
    trackerblocker.h \
    cookie.h \
    encryption.h \
    cache.h \
    downloads.h \
    notification.h \
    mywebpage.h \
    mytreewidget.h \
    proxy.h \
    mycookiejar.h \
    cookieviewdelegate.h \
    ping.h \
    ftpreply.h \
    mywebview.h \
    colors.h \
    worker.h \
    version.h \
    sandboxreply.h \
    log.h \
    plugins.h \
    serverinfo.h

FORMS    += mainwindow.ui \
    dialog.ui \
    browser.ui \
    general.ui \
    trackerblocker.ui \
    cookie.ui \
    encryption.ui \
    cache.ui \
    downloads.ui \
    notification.ui \
    proxy.ui \
    log.ui \
    plugins.ui \
    serverinfo.ui

RESOURCES += \
    default.qrc

CONFIG(debug, debug|release):DEFINES += QT_USE_QSTRINGBUILDER
CONFIG(release, debug|release):DEFINES += QT_USE_QSTRINGBUILDER QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT QT_NO_DEBUG

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

# For Windows platform
win32 {


# For release builds.
# Generic optimizations for all x86/x86_64 platforms
CONFIG(release, debug|release): QMAKE_CXXFLAGS += -ftree-vectorize -funsafe-loop-optimizations -D_WIN32_WINNT=0x0501  -fdata-sections -ffunction-sections -fvisibility-inlines-hidden -O2 -ftree-loop-linear -floop-parallelize-all -flto
CONFIG(release, debug|release): QMAKE_LFLAGS += -Wl,-O1 -Wl,--as-needed -Wl,-S -Wl,--sort-common -Wl,--gc-sections -Wl,--enable-auto-image-base -fuse-linker-plugin -flto

# Additional, arch specific optimizations. Use only one. Add more if needed.

# for classic pentium (i586)
# win32:CONFIG(release, debug|release): QMAKE_CXXFLAGS += -march=pentium -malign-double

# for pentium-mmx
# win32:CONFIG(release, debug|release): QMAKE_CXXFLAGS += -march=pentium-mmx -mmmx -malign-double

# for pentiumpro(i686)
# win32:CONFIG(release, debug|release): QMAKE_CXXFLAGS += -march=pentiumpro -malign-double

# for pentium2
# win32:CONFIG(release, debug|release): QMAKE_CXXFLAGS += -march=pentium2	-mmmx -malign-double

# for pentium3
# win32:CONFIG(release, debug|release): QMAKE_CXXFLAGS += -march=pentium3	-mmmx -msse

# for pentium4
CONFIG(release, debug|release): QMAKE_CXXFLAGS += -march=pentium4 -mfpmath=sse -msse2

# For debug builds
CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -D_WIN32_WINNT=0x0501 -g

LIBS += -L$$PWD/lib/ -lIphlpapi -lws2_32 -lz -lGeoIP
RC_FILE = CyberDragon.rc
DESTDIR = $$OUT_PWD/CyberDragon

copy_dll.target = copy_dll
copy_dll.commands = windeployqt $$OUT_PWD/CyberDragon
QMAKE_EXTRA_TARGETS += copy_dll

lib.depends += copy_dll
lib.path    =  $${DESTDIR}
lib.files   += data/*

INSTALLS       += lib

}

# Suggested file locations for Linux RPM SPEC file:

# CyberDragon ---> /usr/bin/CyberDragon
# CyberDragonManualDraft.pdf ---> /usr/share/doc/CyberDragon-version_number/
# README.txt --> /usr/share/doc/CyberDragon-version_number/
# TODO.txt --> /usr/share/doc/CyberDragon-version_number/
# config.ini --> /etc/CyberDragon/
# anonymous_proxies.txt --> /etc/CyberDragon/
# filters.txt --> /etc/CyberDragon/
# filters_optional.txt --> /etc/CyberDragon/
# more_proxies.txt --> /etc/CyberDragon/
# offline_proxies.txt --> /etc/CyberDragon/
# tor.txt --> /etc/CyberDragon/
# useragents.txt --> /etc/CyberDragon/
# GeoIP.dat --> /usr/libexec/CyberDragon

# For Unix & Linux platform
unix {

DESTDIR = $$OUT_PWD/CyberDragon

# For release builds.
# Generic optimizations for all x86/x86_64 platforms
CONFIG(release, debug|release): QMAKE_CXXFLAGS += -ftree-vectorize -funsafe-loop-optimizations -fdata-sections -ffunction-sections -fvisibility-inlines-hidden -O2 -ftree-loop-linear -floop-parallelize-all -flto
CONFIG(release, debug|release): QMAKE_LFLAGS += -Wl,-O1 -Wl,--as-needed -Wl,--hash-style=gnu -Wl,-S -Wl,--sort-common -Wl,--gc-sections -fuse-linker-plugin -flto

# Additional, arch specific optimizations. Use only one. Add more if needed.

# for classic pentium (i586)
# win32:CONFIG(release, debug|release): QMAKE_CXXFLAGS += -march=pentium -malign-double

# for pentium-mmx
# win32:CONFIG(release, debug|release): QMAKE_CXXFLAGS += -march=pentium-mmx -mmmx -malign-double

# for pentiumpro(i686)
# win32:CONFIG(release, debug|release): QMAKE_CXXFLAGS += -march=pentiumpro -malign-double

# for pentium2
# win32:CONFIG(release, debug|release): QMAKE_CXXFLAGS += -march=pentium2	-mmmx -malign-double

# for pentium3
# win32:CONFIG(release, debug|release): QMAKE_CXXFLAGS += -march=pentium3	-mmmx -msse

# for pentium4
CONFIG(release, debug|release): QMAKE_CXXFLAGS += -march=pentium4 -mfpmath=sse -msse2

LIBS += -lz -lGeoIP

lib.path    =  $${DESTDIR}
lib.files   += data/*.txt data/*.pdf data/*.ini data/*.dat

INSTALLS       += lib

}

# Resource hack. Without this Windows resources won't show up in Qt 5.4
CONFIG += ltcg
