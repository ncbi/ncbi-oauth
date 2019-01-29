default: all

ALL_LIBS =      \
	ncbi-base64 \
	ncbi-json   \
	ncbi-jwa    \
	ncbi-jwk    \
	ncbi-jws    \
	ncbi-jwt    \
#	ncbi-token  \
#	ncbi-jwe

all: $(ALL_LIBS) ncbi-oauth-test

fuzz: ncbi-oauth-fuzz

BINDIR ?= $(CURDIR)/bin
LIBDIR ?= $(CURDIR)/lib
OBJDIR ?= $(CURDIR)/obj

$(BINDIR) $(LIBDIR) $(OBJDIR):
	mkdir -p $@

# determine OS
OS = unknown
UNAME = $(shell uname -s)
ifeq (Darwin, $(UNAME))
	OS = mac
	CFLAGS += -DMAC -DBSD -DUNIX
	UUIDLIB =
endif
ifeq (Linux, $(UNAME))
	OS = linux
	CFLAGS += -DLINUX -DUNIX
	UUIDLIB = -luuid
endif

# only for Linux and MacOS
OBJX = o
LOBX = pic.o

# tell make to forget some of its built-in rules
.SUFFIXES:

# C++ compiler to use
GPP ?= g++ -std=c++11

# rule to build object files from source
$(OBJDIR)/%.$(LOBX): base64/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) $(CFLAGS) -Ibase64 -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.$(LOBX): json/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -Ijson -Ibase64 -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.$(LOBX): json/%.cpp
	$(GPP) -g -c $< -DJSON_TESTING -o $@ $(CFLAGS) -Ijson -Ibase64 -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.$(LOBX): jwa/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -Ijwa -Ibase64 -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.$(LOBX): jwa/%.cpp
	$(GPP) -g -c $< -DJWA_TESTING -o $@ $(CFLAGS) -Ijwa -Ibase64 -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.$(LOBX): jwk/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -Ijwk -Ibase64 -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.$(LOBX): jwk/%.cpp
	$(GPP) -g -c $< -DJWK_TESTING -o $@ $(CFLAGS) -Ijwk -Ibase64 -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.$(LOBX): jws/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -Ijws -Ibase64 -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.$(LOBX): jws/%.cpp
	$(GPP) -g -c $< -DJWS_TESTING -o $@ $(CFLAGS) -Ijws -Ibase64 -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.$(LOBX): jwe/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -Ijwe -Ibase64 -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.$(LOBX): jwe/%.cpp
	$(GPP) -g -c $< -DJWE_TESTING -o $@ $(CFLAGS) -Ijwe -Ibase64 -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.$(LOBX): jwt/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -Ijwt -Ibase64 -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.$(LOBX): jwt/%.cpp
	$(GPP) -g -c $< -DJWT_TESTING -o $@ $(CFLAGS) -Ijwt -Ibase64 -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.$(LOBX): token/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -Itoken -Ibase64 -Iinc -fPIC -MD -Wall
$(OBJDIR)/%.tst.$(LOBX): token/%.cpp
	$(GPP) -g -c $< -DTOKEN_TESTING -o $@ $(CFLAGS) -Itoken -Ibase64 -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.$(OBJX): ncbi-oauth-test/%.cpp
	$(GPP) -g -c $< -o $@ $(CFLAGS) -Incbi-oauth-test -Ibase64 -Iinc -Igoogletest/googletest/include -MD -Wall
$(OBJDIR)/%.$(OBJX): googletest/googletest/src/%.cc
	$(GPP) -g -c $< -o $@ $(CFLAGS) -Incbi-oauth-test -Ibase64 -Iinc -Igoogletest/googletest -Igoogletest/googletest/include -MD

# include dependencies
include $(wildcard $(OBJDIR)/*.d)

OAUTHLIBS =                     \
	$(LIBDIR)/libncbi-jwt.a     \
	$(LIBDIR)/libncbi-json.a    \
	$(LIBDIR)/libncbi-tst-jwt.a

## ncbi-base64
LIBB64SRC =  \
	base64   \
	jwp      \
	jwx      \
	memset_s

LIBB64OBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .$(LOBX),$(LIBB64SRC)))

ncbi-base64: $(LIBDIR) $(LIBDIR)/libncbi-base64.a

$(LIBDIR)/libncbi-base64.a: $(OBJDIR) $(LIBB64OBJ) $(MAKEFILE)
	ar -rc $@ $(LIBB64OBJ)

## ncbi-json
LIBJSONSRC =       \
	json           \
	json-object    \
	json-array     \
	json-value     \
	json-wrapper   \
	json-primitive

LIBJSONOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .$(LOBX),$(LIBJSONSRC)))

ncbi-json: $(LIBDIR) $(LIBDIR)/libncbi-json.a

$(LIBDIR)/libncbi-json.a: $(OBJDIR) $(LIBJSONOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJSONOBJ)

## ncbi-jwa
LIBJWASRC =        \
	jwa-mgr        \
	jwa-none       \
	jwa-hmac       \
	jwa-registry   \

LIBJWAOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .$(LOBX),$(LIBJWASRC)))

ncbi-jwa: $(LIBDIR) $(LIBDIR)/libncbi-jwa.a

$(LIBDIR)/libncbi-jwa.a: $(OBJDIR) $(LIBJWAOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJWAOBJ)

## ncbi-jwk
LIBJWKSRC =        \
	jwk-mgr        \
	jwk-set        \
	jwk-key

LIBJWKOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .$(LOBX),$(LIBJWKSRC)))

ncbi-jwk: $(LIBDIR) $(LIBDIR)/libncbi-jwk.a

$(LIBDIR)/libncbi-jwk.a: $(OBJDIR) $(LIBJWKOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJWKOBJ)

## ncbi-jws
LIBJWSSRC =        \
	jws-mgr

LIBJWSOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .$(LOBX),$(LIBJWSSRC)))

ncbi-jws: $(LIBDIR) $(LIBDIR)/libncbi-jws.a

$(LIBDIR)/libncbi-jws.a: $(OBJDIR) $(LIBJWSOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJWSOBJ)

## ncbi-jwt
LIBJWTSRC =            \
	jwt-mgr            \
	jwt-claims         \
	jwt-unverified     \
	jwt-lock

LIBJWTOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .$(LOBX),$(LIBJWTSRC)))

ncbi-jwt: $(LIBDIR) $(LIBDIR)/libncbi-jwt.a

$(LIBDIR)/libncbi-jwt.a: $(OBJDIR) $(LIBJWTOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJWTOBJ)

## ncbi-test-jwt
LIBTSTJWTSRC =         \
	jwt-mgr.tst        \
	jwt-claims         \
	jwt-unverified     \
	jwt-lock           \
	jws-mgr.tst        \
	jwk-mgr            \
	jwk-set            \
	jwk-key            \
	jwa-mgr            \
	jwa-none           \
	jwa-hmac           \
	jwa-registry.tst   \

LIBTSTJWTOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .$(LOBX),$(LIBTSTJWTSRC)))

ncbi-tst-jwt: $(LIBDIR) $(LIBDIR)/libncbi-tst-jwt.a

$(LIBDIR)/libncbi-tst-jwt.a: $(OBJDIR) $(LIBTSTJWTOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBTSTJWTOBJ)

## ncbi-token
LIBTOKSRC =            \
	token-store

LIBTOKOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .$(LOBX),$(LIBTOKSRC)))

ncbi-token: $(LIBDIR) $(LIBDIR)/libncbi-token.a

$(LIBDIR)/libncbi-token.a: $(OBJDIR) $(LIBTOKOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBTOKOBJ)

## mbedtls
MBEDLIBS =                    \
	$(LIBDIR)/libmbedcrypto.a \
	$(LIBDIR)/libmbedx509.a   \
	$(LIBDIR)/libmbedtls.a

mbedtls: $(MBEDLIBS)

$(LIBDIR)/libmbedcrypto.a: mbedtls/libmbedcrypto.a
	cp $< $@

mbedtls/libmbedcrypto.a:
	$(MAKE) -C mbedtls DEBUG=1 CFLAGS=-I../inc libmbedcrypto.a

$(LIBDIR)/libmbedx509.a: mbedtls/libmbedx509.a
	cp $< $@

mbedtls/libmbedx509.a:
	$(MAKE) -C mbedtls CFLAGS=-I../inc libmbedx509.a

$(LIBDIR)/libmbedtls.a: mbedtls/libmbedtls.a
	cp $< $@

mbedtls/libmbedtls.a:
	$(MAKE) -C mbedtls CFLAGS=-I../inc libmbedtls.a

## ncbi-oauth-test
OAUTHTESTSRC =    \
	main          \
	gtest-all     \
	ncbi-json-ut  \
	ncbi-jwt-ut

OAUTHTESTOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .$(OBJX),$(OAUTHTESTSRC)))

OAUTHTESTLIB =     \
	-L$(LIBDIR)    \
	-lncbi-tst-jwt \
	-lncbi-json    \
	-lncbi-base64  \
	-lmbedcrypto   \
	-lmbedx509     \
	-lmbedtls      \
	$(UUIDLIB)     \
	-lpthread

ncbi-oauth-test: $(BINDIR) $(BINDIR)/ncbi-oauth-test
	$(BINDIR)/ncbi-oauth-test

$(BINDIR)/ncbi-oauth-test: $(OBJDIR) $(OAUTHTESTOBJ) $(OAUTHLIBS) $(MBEDLIBS) $(MAKEFILE)
	$(GPP) $(CFLAGS) -g -o $@ $(OAUTHTESTOBJ) $(OAUTHTESTLIB)

## ncbi-oauth-fuzz
#
# requires clang 7
#

FUZZ_GPP = clang-7
FUZZ_FLAGS = -fsanitize=fuzzer,address,signed-integer-overflow
# with coverage/profile:
#FUZZ_FLAGS = -fsanitize=fuzzer,address,signed-integer-overflow -fprofile-instr-generate -fcoverage-mapping

# if there is a dictionary file, mention it here as -dict=$(path-to-file)
# FUZZ_DICT = -dict=fuzz-dict
FUZZ_DICT =

FUZZ_RUNS ?= -1

OAUTHFUZZSRC =    \
	fuzz-main

OAUTHFUZZOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .$(OBJX),$(OAUTHFUZZSRC))) \
	$(LIBJSONOBJ)

OAUTHFUZZLIB =   \
	-L$(LIBDIR)  \
	-lncbi-jwt   \
	-lmbedcrypto \
	-lmbedx509   \
	-lmbedtls    \
	-lpthread

has-clang:
	clang-7 -v >/dev/null 2>/dev/null || ( echo "fuzzing requires clang version 7 (clang-7) to be in the PATH" && exit 1 )

ncbi-oauth-fuzz: has-clang
	$(MAKE) OBJDIR=$(CURDIR)/obj/fuzz GPP=$(FUZZ_GPP) CFLAGS="$(FUZZ_FLAGS)" $(BINDIR)/ncbi-oauth-fuzz
	mkdir -p fuzz-corpus fuzz-seeds; $(BINDIR)/ncbi-oauth-fuzz -runs=$(FUZZ_RUNS) $(FUZZ_DICT) fuzz-corpus fuzz-seeds

$(BINDIR)/ncbi-oauth-fuzz: $(OBJDIR) $(OAUTHFUZZOBJ) $(MAKEFILE)
	$(GPP) $(CFLAGS) -g -o $@ $(OAUTHFUZZOBJ) $(OAUTHFUZZLIB)

clean:
	rm -rf $(OBJDIR) $(LIBDIR) $(BINDIR)
	$(MAKE) -C mbedtls clean

.PHONY: default all ncbi-json ncbi-jwt ncbi-token mbedtls
