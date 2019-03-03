# Makefile for LPCNet

CC=gcc
CFLAGS+=-Wall -W -Wextra -Wno-unused-function -O3 -g -I../include 

AVX2:=$(shell cat /proc/cpuinfo | grep -c avx2)
AVX:=$(shell cat /proc/cpuinfo | grep -c avx)
NEON:=$(shell cat /proc/cpuinfo | grep -c neon)

ifneq ($(AVX2),0)
CFLAGS+=-mavx2 -mfma 
else
# AVX2 machines will also match on AVX
ifneq ($(AVX),0)
CFLAGS+=-mavx
endif
endif

ifneq ($(NEON),0)
CFLAGS+=-mfpu=neon -march=armv8-a -mtune=cortex-a53
endif

all: bin/dump_data bin/test_lpcnet bin/test_vec bin/dump_data_t bin/lpc_from_cep bin/test_lpcnet_t

dump_data_objs := src/dump_data.o src/freq.o src/kiss_fft.o src/pitch.o src/celt_lpc.o
dump_data_deps := $(dump_data_objs:.o=.d)
bin/dump_data: $(dump_data_objs)
	gcc -o $@ $(CFLAGS) $(dump_data_objs) -lm

-include $dump_data_deps(_deps)

dump_data_t_objs := src/dump_data_t.o src/freq.o src/kiss_fft.o src/pitch.o src/celt_lpc.o
dump_data_t_deps := $(dump_data_t_objs:.o=.d)
bin/dump_data_t: $(dump_data_t_objs)
	gcc -o $@ $(CFLAGS) $(dump_data_t_objs) -lm

-include $dump_data_t_deps(_deps)

lpc_from_cep_objs := src/lpc_from_cep.o src/freq.o src/kiss_fft.o src/pitch.o src/celt_lpc.o
lpc_from_cep_deps := $(lpc_from_cep_objs:.o=.d)
bin/lpc_from_cep: $(lpc_from_cep_objs)
	gcc -o $@ $(CFLAGS) $(lpc_from_cep_objs) -lm

-include $lpc_from_cep_deps(_deps)

test_lpcnet_t_objs := src/test_lpcnet_t.o src/lpcnet.o src/nnet.o src/nnet_data.o src/freq.o src/kiss_fft.o src/pitch.o src/celt_lpc.o
test_lpcnet_t_deps := $(test_lpcnet_t_objs:.o=.d)
bin/test_lpcnet_t: $(test_lpcnet_t_objs)
	gcc -o $@ $(CFLAGS) $(test_lpcnet_t_objs) -lm

-include $test_lpcnet_t_deps(_deps)

test_lpcnet_objs := src/lpcnet.o src/nnet.o src/nnet_data.o src/freq.o src/kiss_fft.o src/pitch.o src/celt_lpc.o
test_lpcnet_deps := $(test_lpcnet_objs:.o=.d)
bin/test_lpcnet: $(test_lpcnet_objs)
	gcc -o $@ $(CFLAGS) $(test_lpcnet_objs) -lm

-include $(test_lpcnet_deps)

lib:
	g++ -fPIC -shared -o liblpc.so src/lpcnet.c src/nnet.c src/nnet_data.c src/freq.c src/kiss_fft.c src/pitch.c src/celt_lpc.c -lm -Wall -W -Wextra -Wno-unused-function -O3 -g -mavx2 -mfma

test_vec_objs := src/test_vec.o
test_vec_deps := $(test_vec_objs:.o=.d)
bin/test_vec: $(test_vec_objs)
	gcc -o $@ $(CFLAGS) $(test_vec_objs) -lm

-include $(test_vec_deps)

test: test_vec
	./test_vec

clean:
	rm -f dump_data test_lpcnet test_vec
	rm -f $(dump_data_objs) $(dump_data_deps) 
	rm -f $(test_lpcnet_objs) $(test_lpcnet_deps) 
	rm -f $(test_vec_objs) $(test_vec_deps) 
