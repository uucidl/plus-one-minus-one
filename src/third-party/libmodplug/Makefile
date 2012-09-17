


include config.mk

LIB=knos/demos/modplug/libmodplug/libmodplug.a

MODPLUG_BASIC_SUPPORT=1

# base OBJS
OBJS+=  modplug.o tables.o sndmix.o sndfile.o snd_fx.o \
        snd_flt.o snd_dsp.o fastmix.o \
        load_mod.o load_xm.o load_it.o load_s3m.o load_wav.o
CFLAGS+=-DNO_REVERB -DFASTSOUNDLIB
CXXFLAGS+=-DNO_REVERB -DFASTSOUNDLIB
CXXFLAGS+=-fno-rtti

ifdef MMCMP_SUPPORT
OBJS+=mmcmp.o
CFLAGS+=-DMMCMP_SUPPORT
CXXFLAGS+=-DMMCMP_SUPPORT
endif

ifndef MODPLUG_BASIC_SUPPORT
OBJS+=load_umx.o load_ult.o load_stm.o  load_ptm.o load_okt.o \
load_mtm.o load_med.o load_mdl.o  load_far.o \
load_dsm.o load_dmf.o load_dbm.o load_ams.o load_amf.o load_669.o \
load_j2b.o load_mt2.o load_psm.o 
else
CFLAGS+=-DMODPLUG_BASIC_SUPPORT
CXXFLAGS+=-DMODPLUG_BASIC_SUPPORT
endif

OBJS:=$(addprefix knos/demos/modplug/libmodplug/,$(OBJS))

include ../../../lib.mk
