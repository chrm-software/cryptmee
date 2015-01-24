#############################################################################
# Makefile for building: CryptMee
# Generated by qmake (2.01a) (Qt 4.7.4) on: Fr 23. Jan 18:58:01 2015
# Project:  CryptMee.pro
# Template: app
# Command: d:/sdks/nokia-qt/madde/targets/harmattan_10.2011.34-1_rt1.2/bin/qmake.exe -spec ../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/default -unix -after  OBJECTS_DIR=obj MOC_DIR=moc UI_DIR=ui RCC_DIR=rcc -o Makefile CryptMee.pro
#############################################################################

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = -DHARMATTAN_BOOSTER -DQT_NO_DEBUG -DQT_DECLARATIVE_LIB -DQT_SQL_LIB -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED
CFLAGS        = -pipe -O2 -g -Wno-psabi -fPIC -fvisibility=hidden -fvisibility-inlines-hidden -I/usr/include/applauncherd -Wall -W -D_REENTRANT $(DEFINES)
CXXFLAGS      = -pipe -O2 -g -Wno-psabi -fPIC -fvisibility=hidden -fvisibility-inlines-hidden -I/usr/include/applauncherd -Wall -W -D_REENTRANT $(DEFINES)
INCPATH       = -I../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/default -I. -I../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/include/qt4/QtCore -I../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/include/qt4/QtGui -I../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/include/qt4/QtSql -I../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/include/qt4/QtDeclarative -I../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/include/qt4 -Iqmlapplicationviewer -Imoc
LINK          = g++
LFLAGS        = -Wl,-O1
LIBS          = $(SUBLIBS)  -LD:/SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/lib -pie -rdynamic -lmdeclarativecache -lQtDeclarative -lQtSql -lQtGui -lQtCore -lpthread 
AR            = ar cqs
RANLIB        = 
QMAKE         = d:/sdks/nokia-qt/madde/targets/harmattan_10.2011.34-1_rt1.2/bin/qmake.exe
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = $(COPY)
COPY_DIR      = $(COPY) -r
STRIP         = strip
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

####### Output directory

OBJECTS_DIR   = obj/

####### Files

SOURCES       = main.cpp \
		mailreaderthread.cpp \
		mailreader.cpp \
		mailobject.cpp \
		keyreader.cpp \
		keyobject.cpp \
		gnupgconnector.cpp \
		qmlapplicationviewer/qmlapplicationviewer.cpp moc/moc_qmlapplicationviewer.cpp \
		moc/moc_mailreaderthread.cpp \
		moc/moc_mailreader.cpp \
		moc/moc_gnupgconnector.cpp \
		rcc/qrc_cryptmeeressource.cpp
OBJECTS       = obj/main.o \
		obj/mailreaderthread.o \
		obj/mailreader.o \
		obj/mailobject.o \
		obj/keyreader.o \
		obj/keyobject.o \
		obj/gnupgconnector.o \
		obj/qmlapplicationviewer.o \
		obj/moc_qmlapplicationviewer.o \
		obj/moc_mailreaderthread.o \
		obj/moc_mailreader.o \
		obj/moc_gnupgconnector.o \
		obj/qrc_cryptmeeressource.o
DIST          = ../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/common/g++.conf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/common/unix.conf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/common/linux.conf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/qconfig.pri \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/qt_functions.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/qt_config.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/exclusive_builds.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/default_pre.prf \
		qmlapplicationviewer/qmlapplicationviewer.pri \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/release.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/default_post.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/qdeclarative-boostable.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/link_pkgconfig.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/warn_on.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/qt.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/unix/thread.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/moc.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/resources.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/uic.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/yacc.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/lex.prf \
		CryptMee.pro
QMAKE_TARGET  = CryptMee
DESTDIR       = 
TARGET        = CryptMee

first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile $(TARGET)

$(TARGET):  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

Makefile: CryptMee.pro  ../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/default/qmake.conf ../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/common/g++.conf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/common/unix.conf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/common/linux.conf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/qconfig.pri \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/qt_functions.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/qt_config.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/exclusive_builds.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/default_pre.prf \
		qmlapplicationviewer/qmlapplicationviewer.pri \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/release.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/default_post.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/qdeclarative-boostable.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/link_pkgconfig.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/warn_on.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/qt.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/unix/thread.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/moc.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/resources.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/uic.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/yacc.prf \
		../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/lex.prf \
		d:/SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/lib/libQtDeclarative.prl \
		d:/SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/lib/libQtSql.prl \
		d:/SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/lib/libQtGui.prl \
		d:/SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/lib/libQtCore.prl
	$(QMAKE) -spec ../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/default -unix -after  OBJECTS_DIR=obj MOC_DIR=moc UI_DIR=ui RCC_DIR=rcc -o Makefile CryptMee.pro
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/common/g++.conf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/common/unix.conf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/common/linux.conf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/qconfig.pri:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/qt_functions.prf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/qt_config.prf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/exclusive_builds.prf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/default_pre.prf:
qmlapplicationviewer/qmlapplicationviewer.pri:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/release.prf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/default_post.prf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/qdeclarative-boostable.prf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/link_pkgconfig.prf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/warn_on.prf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/qt.prf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/unix/thread.prf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/moc.prf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/resources.prf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/uic.prf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/yacc.prf:
../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/features/lex.prf:
d:/SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/lib/libQtDeclarative.prl:
d:/SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/lib/libQtSql.prl:
d:/SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/lib/libQtGui.prl:
d:/SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/lib/libQtCore.prl:
qmake:  FORCE
	@$(QMAKE) -spec ../../../../SDKs/Nokia-Qt/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim/usr/share/qt4/mkspecs/default -unix -after  OBJECTS_DIR=obj MOC_DIR=moc UI_DIR=ui RCC_DIR=rcc -o Makefile CryptMee.pro

dist: 
	@$(CHK_DIR_EXISTS) obj/CryptMee1.0.0 || $(MKDIR) obj/CryptMee1.0.0 
	$(COPY_FILE) --parents $(SOURCES) $(DIST) obj/CryptMee1.0.0/ && $(COPY_FILE) --parents qmlapplicationviewer/qmlapplicationviewer.h mailreaderthread.h mailreader.h mailobject.h keyreader.h keyobject.h gnupgconnector.h configuration.h obj/CryptMee1.0.0/ && $(COPY_FILE) --parents cryptmeeressource.qrc obj/CryptMee1.0.0/ && $(COPY_FILE) --parents main.cpp mailreaderthread.cpp mailreader.cpp mailobject.cpp keyreader.cpp keyobject.cpp gnupgconnector.cpp qmlapplicationviewer/qmlapplicationviewer.cpp obj/CryptMee1.0.0/ && (cd `dirname obj/CryptMee1.0.0` && $(TAR) CryptMee1.0.0.tar CryptMee1.0.0 && $(COMPRESS) CryptMee1.0.0.tar) && $(MOVE) `dirname obj/CryptMee1.0.0`/CryptMee1.0.0.tar.gz . && $(DEL_FILE) -r obj/CryptMee1.0.0


clean:compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) Makefile


check: first

mocclean: compiler_moc_header_clean compiler_moc_source_clean

mocables: compiler_moc_header_make_all compiler_moc_source_make_all

compiler_moc_header_make_all: moc/moc_qmlapplicationviewer.cpp moc/moc_mailreaderthread.cpp moc/moc_mailreader.cpp moc/moc_gnupgconnector.cpp
compiler_moc_header_clean:
	-$(DEL_FILE) moc/moc_qmlapplicationviewer.cpp moc/moc_mailreaderthread.cpp moc/moc_mailreader.cpp moc/moc_gnupgconnector.cpp
moc/moc_qmlapplicationviewer.cpp: qmlapplicationviewer/qmlapplicationviewer.h
	D:/SDKs/Nokia-Qt/Madde/targets/harmattan_10.2011.34-1_rt1.2/bin/moc.exe $(DEFINES) $(INCPATH) qmlapplicationviewer/qmlapplicationviewer.h -o moc/moc_qmlapplicationviewer.cpp

moc/moc_mailreaderthread.cpp: mailreaderthread.h
	D:/SDKs/Nokia-Qt/Madde/targets/harmattan_10.2011.34-1_rt1.2/bin/moc.exe $(DEFINES) $(INCPATH) mailreaderthread.h -o moc/moc_mailreaderthread.cpp

moc/moc_mailreader.cpp: mailobject.h \
		mailreader.h \
		mailreaderthread.h \
		mailreader.h
	D:/SDKs/Nokia-Qt/Madde/targets/harmattan_10.2011.34-1_rt1.2/bin/moc.exe $(DEFINES) $(INCPATH) mailreader.h -o moc/moc_mailreader.cpp

moc/moc_gnupgconnector.cpp: keyreader.h \
		keyobject.h \
		configuration.h \
		gnupgconnector.h
	D:/SDKs/Nokia-Qt/Madde/targets/harmattan_10.2011.34-1_rt1.2/bin/moc.exe $(DEFINES) $(INCPATH) gnupgconnector.h -o moc/moc_gnupgconnector.cpp

compiler_rcc_make_all: rcc/qrc_cryptmeeressource.cpp
compiler_rcc_clean:
	-$(DEL_FILE) rcc/qrc_cryptmeeressource.cpp
rcc/qrc_cryptmeeressource.cpp: cryptmeeressource.qrc \
		languages/lang_de_DE.qm \
		pix/public_keys_export.png \
		pix/CryptMee.png \
		pix/clipboard_from.png \
		pix/public_keys_import.png \
		pix/public_keys.png \
		pix/clipboard_to.png \
		pix/gpg-mail.png \
		pix/gpg-text.png \
		pix/gpg-keys.png \
		pix/private_keys.png \
		pix/public_keys_edit.png
	d:/SDKs/Nokia-Qt/Madde/targets/harmattan_10.2011.34-1_rt1.2/bin/rcc.exe -name cryptmeeressource cryptmeeressource.qrc -o rcc/qrc_cryptmeeressource.cpp

compiler_image_collection_make_all: ui/qmake_image_collection.cpp
compiler_image_collection_clean:
	-$(DEL_FILE) ui/qmake_image_collection.cpp
compiler_moc_source_make_all:
compiler_moc_source_clean:
compiler_uic_make_all:
compiler_uic_clean:
compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean: compiler_moc_header_clean compiler_rcc_clean 

####### Compile

obj/main.o: main.cpp gnupgconnector.h \
		keyreader.h \
		keyobject.h \
		configuration.h \
		mailreader.h \
		mailobject.h \
		mailreaderthread.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o obj/main.o main.cpp

obj/mailreaderthread.o: mailreaderthread.cpp mailreaderthread.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o obj/mailreaderthread.o mailreaderthread.cpp

obj/mailreader.o: mailreader.cpp mailreader.h \
		mailobject.h \
		mailreaderthread.h \
		configuration.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o obj/mailreader.o mailreader.cpp

obj/mailobject.o: mailobject.cpp mailobject.h \
		mailreader.h \
		mailreaderthread.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o obj/mailobject.o mailobject.cpp

obj/keyreader.o: keyreader.cpp keyreader.h \
		keyobject.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o obj/keyreader.o keyreader.cpp

obj/keyobject.o: keyobject.cpp keyobject.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o obj/keyobject.o keyobject.cpp

obj/gnupgconnector.o: gnupgconnector.cpp gnupgconnector.h \
		keyreader.h \
		keyobject.h \
		configuration.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o obj/gnupgconnector.o gnupgconnector.cpp

obj/qmlapplicationviewer.o: qmlapplicationviewer/qmlapplicationviewer.cpp qmlapplicationviewer/qmlapplicationviewer.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o obj/qmlapplicationviewer.o qmlapplicationviewer/qmlapplicationviewer.cpp

obj/moc_qmlapplicationviewer.o: moc/moc_qmlapplicationviewer.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o obj/moc_qmlapplicationviewer.o moc/moc_qmlapplicationviewer.cpp

obj/moc_mailreaderthread.o: moc/moc_mailreaderthread.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o obj/moc_mailreaderthread.o moc/moc_mailreaderthread.cpp

obj/moc_mailreader.o: moc/moc_mailreader.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o obj/moc_mailreader.o moc/moc_mailreader.cpp

obj/moc_gnupgconnector.o: moc/moc_gnupgconnector.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o obj/moc_gnupgconnector.o moc/moc_gnupgconnector.cpp

obj/qrc_cryptmeeressource.o: rcc/qrc_cryptmeeressource.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o obj/qrc_cryptmeeressource.o rcc/qrc_cryptmeeressource.cpp

####### Install

install_itemfolder_01: first FORCE
	@$(CHK_DIR_EXISTS) $(INSTALL_ROOT)/opt/CryptMee/qml/ || $(MKDIR) $(INSTALL_ROOT)/opt/CryptMee/qml/ 
	-$(INSTALL_DIR) d:/chrm-profile/Projects/QtProjects/CryptMee/qml/CryptMee $(INSTALL_ROOT)/opt/CryptMee/qml/


uninstall_itemfolder_01:  FORCE
	-$(DEL_FILE) -r $(INSTALL_ROOT)/opt/CryptMee/qml/CryptMee
	-$(DEL_DIR) $(INSTALL_ROOT)/opt/CryptMee/qml/ 


install_itemfolder_splash: first FORCE
	@$(CHK_DIR_EXISTS) $(INSTALL_ROOT)/opt/CryptMee/splash/ || $(MKDIR) $(INSTALL_ROOT)/opt/CryptMee/splash/ 
	-$(INSTALL_DIR) d:/chrm-profile/Projects/QtProjects/CryptMee/pix/splash $(INSTALL_ROOT)/opt/CryptMee/splash/


uninstall_itemfolder_splash:  FORCE
	-$(DEL_FILE) -r $(INSTALL_ROOT)/opt/CryptMee/splash/splash
	-$(DEL_DIR) $(INSTALL_ROOT)/opt/CryptMee/splash/ 


install_icon: first FORCE
	@$(CHK_DIR_EXISTS) $(INSTALL_ROOT)/usr/share/icons/hicolor/80x80/apps/ || $(MKDIR) $(INSTALL_ROOT)/usr/share/icons/hicolor/80x80/apps/ 
	-$(INSTALL_FILE) d:/chrm-profile/Projects/QtProjects/CryptMee/CryptMee80.png $(INSTALL_ROOT)/usr/share/icons/hicolor/80x80/apps/


uninstall_icon:  FORCE
	-$(DEL_FILE) -r $(INSTALL_ROOT)/usr/share/icons/hicolor/80x80/apps/CryptMee80.png
	-$(DEL_DIR) $(INSTALL_ROOT)/usr/share/icons/hicolor/80x80/apps/ 


install_desktopfile: first FORCE
	@$(CHK_DIR_EXISTS) $(INSTALL_ROOT)/usr/share/applications/ || $(MKDIR) $(INSTALL_ROOT)/usr/share/applications/ 
	-$(INSTALL_FILE) d:/chrm-profile/Projects/QtProjects/CryptMee/CryptMee_harmattan.desktop $(INSTALL_ROOT)/usr/share/applications/


uninstall_desktopfile:  FORCE
	-$(DEL_FILE) -r $(INSTALL_ROOT)/usr/share/applications/CryptMee_harmattan.desktop
	-$(DEL_DIR) $(INSTALL_ROOT)/usr/share/applications/ 


install_target: first FORCE
	@$(CHK_DIR_EXISTS) $(INSTALL_ROOT)/opt/CryptMee/bin/ || $(MKDIR) $(INSTALL_ROOT)/opt/CryptMee/bin/ 
	-$(INSTALL_PROGRAM) "$(QMAKE_TARGET)" "$(INSTALL_ROOT)/opt/CryptMee/bin/$(QMAKE_TARGET)"

uninstall_target:  FORCE
	-$(DEL_FILE) "$(INSTALL_ROOT)/opt/CryptMee/bin/$(QMAKE_TARGET)"
	-$(DEL_DIR) $(INSTALL_ROOT)/opt/CryptMee/bin/ 


install:  install_itemfolder_01 install_itemfolder_splash install_icon install_desktopfile install_target  FORCE

uninstall: uninstall_itemfolder_01 uninstall_itemfolder_splash uninstall_icon uninstall_desktopfile uninstall_target   FORCE

FORCE:
