include $(top_srcdir)/common.mk
libdir = $(plugindir)

EXTRA_DIST = EnlightenmentPlugin.h EnlightenmentDevice.h EnlightenmentPort.h \
             EnlightenmentSocket.h

if HAVE_LIBUSBDMX
  lib_LTLIBRARIES = libolaenlightenment.la
  libolaenlightenment_la_SOURCES = EnlightenmentPlugin.cpp EnlightenmentDevice.cpp \
                          EnlightenmentPort.cpp
  libolaenlightenment_la_CXXFLAGS = $(COMMON_CXXFLAGS) $(libusbdmx_CFLAGS)
  libolaenlightenment_la_LIBADD = $(libusbdmx_LIBS) \
                          ../../common/libolacommon.la
endif

