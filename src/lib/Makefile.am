AM_CFLAGS = \
	$(WARNING_CFLAGS) \
	-Wno-unused-parameter \
	$(CODE_COVERAGE_CFLAGS)

AM_LDFLAGS = \
	$(CODE_COVERAGE_LIBS)

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	-DINSTALLED_CF_PATTERN=\"$(fluxsecuritycfdir)/*.toml\" \
	$(SODIUM_CFLAGS) $(JANSSON_CFLAGS) $(MUNGE_CFLAGS)

lib_LTLIBRARIES = \
	libflux-security.la

noinst_LTLIBRARIES = \
	libsecurity.la

fluxsecurityinclude_HEADERS = \
	context.h \
	sign.h

nodist_fluxsecurityinclude_HEADERS = \
	version.h

libflux_security_la_SOURCES =
libflux_security_la_LIBADD = \
	$(builddir)/libsecurity.la \
	$(top_builddir)/src/libca/libca.la \
	$(top_builddir)/src/libutil/libutil.la \
	$(top_builddir)/src/libtomlc99/libtomlc99.la \
	$(SODIUM_LIBS) $(JANSSON_LIBS) $(MUNGE_LIBS)

libflux_security_la_LDFLAGS = \
	-Wl,--version-script=$(srcdir)/libflux-security.map \
	-version-info @LIBFLUX_SECURITY_VERSION_INFO@ \
	-shared -export-dynamic --disable-static

libsecurity_la_SOURCES = \
	context.c \
	context_private.h \
	sign.c \
	sign_mech.h \
	sign_none.c \
	sign_munge.c \
	sign_curve.c \
	version.c

TESTS = \
	test_context.t \
	test_sign.t \
	test_version.t

check_PROGRAMS = \
	$(TESTS)

TEST_EXTENSIONS = .t
T_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
	$(top_srcdir)/config/tap-driver.sh

test_cppflags = \
	$(AM_CPPFLAGS)

test_ldadd = \
	$(AM_LDFLAGS) \
	$(builddir)/libsecurity.la \
	$(top_builddir)/src/libca/libca.la \
	$(top_builddir)/src/libutil/libutil.la \
	$(top_builddir)/src/libtomlc99/libtomlc99.la \
	$(top_builddir)/src/libtap/libtap.la \
	$(SODIUM_LIBS) $(JANSSON_LIBS) $(MUNGE_LIBS)

test_context_t_SOURCES = test/context.c
test_context_t_CPPFLAGS = $(test_cppflags)
test_context_t_LDADD = $(test_ldadd)

test_sign_t_SOURCES = test/sign.c
test_sign_t_CPPFLAGS = $(test_cppflags)
test_sign_t_LDADD = $(test_ldadd)

test_version_t_SOURCES = test/version.c
test_version_t_CPPFLAGS = $(test_cppflags)
test_version_t_LDADD = $(test_ldadd)

if WITH_PKG_CONFIG
pkgconfig_DATA = flux-security.pc
endif

EXTRA_DIST = libflux-security.map
