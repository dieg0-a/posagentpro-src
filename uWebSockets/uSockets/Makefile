# WITH_BORINGSSL=1 enables BoringSSL support, linked statically (preferred over OpenSSL)
# You need to call "make boringssl" before
ifeq ($(WITH_BORINGSSL),1)
	override CFLAGS += -Iboringssl/include -pthread -DLIBUS_USE_OPENSSL
	override LDFLAGS += -pthread boringssl/build/ssl/libssl.a boringssl/build/crypto/libcrypto.a -lstdc++
else
	# WITH_OPENSSL=1 enables OpenSSL 1.1+ support
	# For now we need to link with C++ for OpenSSL support, but should be removed with time
	ifeq ($(WITH_OPENSSL),1)
		override CFLAGS += -DLIBUS_USE_OPENSSL
		# With problems on macOS, make sure to pass needed LDFLAGS required to find these
		override LDFLAGS += -lssl -lcrypto -lstdc++
	else
		# WITH_WOLFSSL=1 enables WolfSSL 4.2.0 support (mutually exclusive with OpenSSL)
		ifeq ($(WITH_WOLFSSL),1)
			# todo: change these
			override CFLAGS += -DLIBUS_USE_WOLFSSL -I/usr/local/include
			override LDFLAGS += -L/usr/local/lib -lwolfssl
		else
			override CFLAGS += -DLIBUS_NO_SSL
		endif
	endif
endif

# WITH_LIBUV=1 builds with libuv as event-loop
ifeq ($(WITH_LIBUV),1)
	override CFLAGS += -DLIBUS_USE_LIBUV
	override LDFLAGS += -luv
endif

# WITH_ASIO builds with boot asio event-loop
ifeq ($(WITH_ASIO),1)
	override CFLAGS += -DLIBUS_USE_ASIO
	override LDFLAGS += -lstdc++ -lpthread
	override CXXFLAGS += -pthread -DLIBUS_USE_ASIO
endif

# WITH_GCD=1 builds with libdispatch as event-loop
ifeq ($(WITH_GCD),1)
	override CFLAGS += -DLIBUS_USE_GCD
	override LDFLAGS += -framework CoreFoundation
endif

# WITH_ASAN builds with sanitizers
ifeq ($(WITH_ASAN),1)
	override CFLAGS += -fsanitize=address -g
	override LDFLAGS += -fsanitize=address
endif

ifeq ($(WITH_QUIC),1)
	override CFLAGS += -DLIBUS_USE_QUIC -pthread -std=c11 -Isrc -Ilsquic/include
	override LDFLAGS += -pthread -lz -lm uSockets.a lsquic/src/liblsquic/liblsquic.a
else
	override CFLAGS += -std=c11 -Isrc
	override LDFLAGS += uSockets.a
endif

# By default we build the uSockets.a static library
default:
	rm -f *.o
	$(CC) $(CFLAGS) -flto -O3 -c src/*.c src/eventing/*.c src/crypto/*.c
# Also link in Boost Asio support
ifeq ($(WITH_ASIO),1)
	$(CXX) $(CXXFLAGS) -Isrc -std=c++14 -flto -O3 -c src/eventing/asio.cpp
endif

# For now we do rely on C++17 for OpenSSL support but we will be porting this work to C11
ifeq ($(WITH_OPENSSL),1)
	$(CXX) $(CXXFLAGS) -std=c++17 -flto -O3 -c src/crypto/*.cpp
endif
ifeq ($(WITH_BORINGSSL),1)
	$(CXX) $(CXXFLAGS) -std=c++17 -flto -O3 -c src/crypto/*.cpp
endif
	$(AR) rvs uSockets.a *.o

# BoringSSL needs cmake and golang
.PHONY: boringssl
boringssl:
	cd boringssl && mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j8

# Builds all examples
.PHONY: examples
examples: default
	for f in examples/*.c; do $(CC) -flto -O3 $(CFLAGS) -o $$(basename "$$f" ".c") "$$f" $(LDFLAGS); done

swift_examples:
	swiftc -O -I . examples/swift_http_server/main.swift uSockets.a -o swift_http_server

clean:
	rm -f *.o
	rm -f *.a
	rm -rf .certs
