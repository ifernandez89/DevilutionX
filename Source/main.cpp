#ifdef USE_SDL3
#include <SDL3/SDL.h>
#else
#include <SDL.h>
#include <SDL_main.h>
#endif

#include "phase2_logging.h"

#ifdef __SWITCH__
#include "platform/switch/network.h"
#include "platform/switch/random.hpp"
#include "platform/switch/romfs.hpp"
#endif
#ifdef __3DS__
#include "platform/ctr/system.h"
#endif
#ifdef __vita__
#include "platform/vita/network.h"
#include "platform/vita/random.hpp"
#endif
#ifdef NXDK
#include <nxdk/mount.h>
#endif
#ifdef GPERF_HEAP_MAIN
#include <gperftools/heap-profiler.h>
#endif

#include "diablo.h"

#if !defined(__APPLE__)
extern "C" const char *__asan_default_options() // NOLINT(bugprone-reserved-identifier, readability-identifier-naming)
{
	return "halt_on_error=0";
}
#endif

extern "C" int main(int argc, char **argv)
{
	PHASE2_LOG("main() started - WebAssembly runtime active");
	PHASE2_ENGINE_STEP("MAIN_ENTRY", "C main() function executing");
	
#ifdef __SWITCH__
	switch_romfs_init();
	switch_enable_network();
#ifdef PACKET_ENCRYPTION
	randombytes_switchrandom_init();
#endif
#endif
#ifdef __3DS__
	ctr_sys_init();
#endif
#ifdef __vita__
	vita_enable_network();
#ifdef PACKET_ENCRYPTION
	randombytes_vitarandom_init();
#endif
#endif
#ifdef NXDK
	nxMountDrive('E', "\\Device\\Harddisk0\\Partition1\\");
#endif
#ifdef GPERF_HEAP_MAIN
	HeapProfilerStart("main");
#endif
	
	PHASE2_MEMORY_CHECK("main()");
	PHASE2_ENGINE_STEP("DIABLO_MAIN_CALL", "Calling DiabloMain()");
	
	const int result = devilution::DiabloMain(argc, argv);
	
	PHASE2_ENGINE_STEP("DIABLO_MAIN_RETURN", "DiabloMain() returned successfully");
	PHASE2_LOG("main() completed successfully");
	
#ifdef GPERF_HEAP_MAIN
	HeapProfilerStop();
#endif
	return result;
}
