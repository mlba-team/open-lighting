include $(top_srcdir)/common.mk

noinst_LTLIBRARIES = libolaproto.la
nodist_libolaproto_la_SOURCES = Ola.pb.cc
libolaproto_la_LIBADD = $(libprotobuf_LIBS)
# required, otherwise we get build errors
libolaproto_la_CXXFLAGS = $(WARNING_CXXFLAGS)

EXTRA_DIST = Ola.proto

BUILT_SOURCES = Ola.pb.cc Ola.pb.h

Ola.pb.cc Ola.pb.h: Ola.proto
	$(PROTOC)  --cpp_out ./ Ola.proto

clean-local:
	rm -f *.pb.{h,cc}
