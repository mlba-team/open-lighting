include $(top_srcdir)/common.mk
libdir = $(plugindir)
SUBDIRS = messages
EXTRA_DIST = ArtNetDevice.h ArtNetNode.h ArtNetPackets.h ArtNetPlugin.h \
             ArtNetPort.h

if USE_ARTNET
# This is a library which doesn't contain any coupling to olad
noinst_LTLIBRARIES = libolaartnetnode.la
libolaartnetnode_la_SOURCES = ArtNetNode.cpp
libolaartnetnode_la_LIBADD = ../../common/libolacommon.la

# The OLA artnet plugin
lib_LTLIBRARIES = libolaartnet.la
libolaartnet_la_SOURCES = ArtNetPlugin.cpp ArtNetDevice.cpp ArtNetPort.cpp
libolaartnet_la_LIBADD = libolaartnetnode.la \
                         messages/libolaartnetconf.la

# Test programs
if BUILD_TESTS
TESTS = ArtNetTester
endif
check_PROGRAMS = $(TESTS)
ArtNetTester_SOURCES = ArtNetNodeTest.cpp
ArtNetTester_CXXFLAGS = $(COMMON_TESTING_FLAGS)
ArtNetTester_LDADD = $(COMMON_TESTING_LIBS) \
                     libolaartnetnode.la
endif
