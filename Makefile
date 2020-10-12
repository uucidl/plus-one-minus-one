CC_SRC:=src/library/cstr_map_stl_impl.cc src/library/map_stl_impl.cc src/third-party/libmodplug/fastmix.cc src/third-party/libmodplug/libmodplug_sndfile.cc src/third-party/libmodplug/load_amf.cc src/third-party/libmodplug/load_dbm.cc src/third-party/libmodplug/load_dmf.cc src/third-party/libmodplug/load_dsm.cc src/third-party/libmodplug/load_far.cc src/third-party/libmodplug/load_it.cc src/third-party/libmodplug/load_j2b.cc src/third-party/libmodplug/load_mdl.cc src/third-party/libmodplug/load_med.cc src/third-party/libmodplug/load_mod.cc src/third-party/libmodplug/load_mt2.cc src/third-party/libmodplug/load_mtm.cc src/third-party/libmodplug/load_ptm.cc src/third-party/libmodplug/load_s3m.cc src/third-party/libmodplug/load_stm.cc src/third-party/libmodplug/load_ult.cc src/third-party/libmodplug/load_umx.cc src/third-party/libmodplug/load_wav.cc src/third-party/libmodplug/load_xm.cc src/third-party/libmodplug/mmcmp.cc src/third-party/libmodplug/modplug.cc src/third-party/libmodplug/snd_dsp.cc src/third-party/libmodplug/snd_flt.cc src/third-party/libmodplug/snd_fx.cc src/third-party/libmodplug/sndmix.cc src/third-party/libmodplug/tables.cc

SRC:=src/1.c src/alogic.c src/blur.c src/chamberlin_2p.c src/gscreen.c src/ice.c src/icefx.c src/image.c src/image_fetcher.c src/image_load_jpeg.c src/image_load_png.c src/image_save_png.c src/image_save_targa.c src/metro.c src/projector.c src/ps.c src/rub.c src/stutter.c src/tap.c src/text-display.c src/traffic.c src/velocity.c src/vlogic.c src/vloo.c src/vmix.c src/audio/osc.c src/audio/progress.c src/audio/resource_audio_effect.c src/audio/wavetable.c src/audio/wrapper.c src/freetype/ft_renderer.c src/freetype/outline.c src/lib/chance-init.c src/lib/cokus.c src/lib/http_loader.c src/lib/locale.c src/library/cstr_map.c src/library/http_stream.c src/library/map.c src/library/memory.c src/library/memory_stream.c src/library/sock.c src/library/sock_stream.c src/library/stack.c src/library/stdio_stream.c src/library/stream.c src/library/strings.c src/library/thread-helper.c src/library/time.c src/library/vector.c src/lib/resource_loaders.c src/lib/url.c src/lib/url_open.c src/lib/virtual_loader_boot.c src/lib/virtual_loader.c src/messaging/class.c src/messaging/context.c src/messaging/definitions.c src/messaging/dispatcher.c src/messaging/libffi_definitions.c src/messaging/receiver.c src/messaging/router.c src/modplug/modplug_player.c src/network/channel.c src/network/http_handler.c src/network/shared_transport.c src/network/transport_callbacks.c src/scripting/atom.c src/scripting/bytecode_stream.c src/scripting/compile.c src/scripting/dictionary.c src/system/audio_effect.c src/system/demo.c src/system/effect.c src/system/event.c src/system/event_listener.c src/system/frame_converter.c src/system/kgo.c src/system/kgo_disk_driver.c src/system/kgo_driver.c src/system/kgo_sdlgl_driver.c src/system/main.c src/system/main-default.c src/system/midi-event.c src/system/midi-null.c src/system/opengl_adapter.c src/system/opengl_effect.c src/system/opengl_to_video_frame_converter.c src/system/pan.c src/system/pan_driver.c src/system/pan_sdl_driver.c src/system/video_adapter.c src/system/video_effect.c src/system/video_to_opengl_frame_converter.c src/widgets/gauge.c src/system/architectures/sdl/keyboard.c src/system/architectures/sdl/sdl_event_listener.c


OBJECTS=$(SRC:.c=.o) $(CC_SRC:.cc=.o)
DEPS=$(SRC:.c=.d) $(CC_SRC:.cc=.d)

CFLAGS+=-MMD -std=gnu11 -g -DLINUX -DMAP_STL_IMPL -DCSTR_MAP_STL_IMPL -DKGO_DEFAULT='"sdlgl"' -DPAN_DEFAULT='"sdl"' -DKNOS_RELEASE=-1 -DKNOS_BUILD=0 -DUSE_BUILTIN_APPLY -DMMCMP_SUPPORT -DCCGVERSION="" -DMODPLUG_BASIC_SUPPORT `pkg-config sdl2 --cflags` `pkg-config libjpeg --cflags` `pkg-config freetype2 --cflags` `pkg-config --cflags libffi` `pkg-config --cflags libpng` -Werror -Isrc/
CXXFLAGS+=-MMD -g -DLINUX -DMAP_STL_IMPL -DCSTR_MAP_STL_IMPL -DMODPLUG_BASIC_SUPPORT -Werror -Isrc/
LDFLAGS=-lpthread -lm `pkg-config sdl2 --libs` `pkg-config gl --libs` `pkg-config freetype2 --libs` `pkg-config libpng --libs` `pkg-config libjpeg --libs`

all: a.out

a.out: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -o $@
.PHONY: clean all

clean:
	$(RM) $(OBJECTS) $(DEPS) $(BINS)

-include $(DEPS)
