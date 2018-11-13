default: ncbi-json ncbi-jwt ncbi-oauth-test

fuzz: ncbi-oauth-fuzz

BINDIR ?= $(CURDIR)/bin
LIBDIR ?= $(CURDIR)/lib
OBJDIR ?= $(CURDIR)/obj

$(BINDIR) $(LIBDIR) $(OBJDIR):
	mkdir -p $@

# only for Linux and MacOS
OBJX = o
LOBX = pic.o

# tell make to forget some of its built-in rules
.SUFFIXES:

# C++ compiler to use
GPP ?= g++ -std=c++11

# rule to build object files from source
$(OBJDIR)/%.$(LOBX): ncbi-json/%.cpp
	$(GPP) $(CFLAGS) -g -c $< -o $@ -Incbi-json -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.$(LOBX): ncbi-jwt/%.cpp
	$(GPP) $(CFLAGS) -g -c $< -o $@ -Incbi-jwt -Iinc -fPIC -MD -Wall

$(OBJDIR)/%.$(OBJX): ncbi-oauth-test/%.cpp
	$(GPP) $(CFLAGS) -g -c $< -o $@ -Incbi-oauth-test -Iinc -Igoogletest/googletest/include -MD -Wall
$(OBJDIR)/%.$(OBJX): googletest/googletest/src/%.cc
	$(GPP) $(CFLAGS) -g -c $< -o $@ -Incbi-oauth-test -Iinc -Igoogletest/googletest -Igoogletest/googletest/include -MD

# include dependencies
include $(wildcard $(OBJDIR)/*.d)

OAUTHLIBS =                  \
	$(LIBDIR)/libncbi-json.a \
	$(LIBDIR)/libncbi-jwt.a

## ncbi-json
LIBJSONSRC =       \
	parser         \
	json-object    \
	json-array     \
	json-wrapper   \
	json-primitive \
	json-value     \
	json-exception

LIBJSONOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .$(LOBX),$(LIBJSONSRC)))

ncbi-json: $(LIBDIR) $(LIBDIR)/libncbi-json.a

$(LIBDIR)/libncbi-json.a: $(OBJDIR) $(LIBJSONOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJSONOBJ)

## ncbi-jwt
LIBJWTSRC =            \
	jwt-factory        \
	jwt-claims         \
	jws-factory        \
	jwa-none           \
	jwa-hmac           \
	jwa-rsa            \
	jwa-factory        \
	jwt-lock           \
	jwt-exception      \
	base64

LIBJWTOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .$(LOBX),$(LIBJWTSRC)))

ncbi-jwt: $(LIBDIR) $(LIBDIR)/libncbi-jwt.a

$(LIBDIR)/libncbi-jwt.a: $(OBJDIR) $(LIBJWTOBJ) $(MAKEFILE)
	ar -rc $@ $(LIBJWTOBJ)

## mbedtls
MBEDLIBS =                    \
	$(LIBDIR)/libmbedcrypto.a \
	$(LIBDIR)/libmbedx509.a   \
	$(LIBDIR)/libmbedtls.a

$(LIBDIR)/libmbedcrypto.a: mbedtls/libmbedcrypto.a
	cp $< $@

mbedtls/libmbedcrypto.a:
	$(MAKE) -C mbedtls CFLAGS=-I../inc libmbedcrypto.a

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

OAUTHTESTLIB =   \
	-L$(LIBDIR)  \
	-lncbi-jwt   \
	-lncbi-json  \
	-lmbedcrypto \
	-lmbedx509   \
	-lmbedtls    \
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

.PHONY: default
