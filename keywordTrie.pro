TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


SOURCES +=	main.cpp \
				keywordTrie.cpp

HEADERS +=	keywordTrie.h

CONFIG += link_pkgconfig
PKGCONFIG += libgvc

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE *= -O3

