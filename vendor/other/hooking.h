#pragma once
#include "../dlfcn-win32-1.4.2/src/dlfcn.h"
#include <cstdio>
#include <android/log.h>
#include <mutex>
#include <cstdint>
#include "../Dobby/include/dobby.h"

#define LOG_TAG "libviolet_hook"

namespace hooking {
    static void* getHandle() {
        static void* handle = nullptr;
        static std::once_flag once;
        std::call_once(once, []() {
            handle = dlopen("libgame.so", RTLD_NOW);
            if (!handle) {
                __android_log_print(ANDROID_LOG_WARN, LOG_TAG, "dlopen(libgame.so) failed: %s - trying global handle", dlerror());
                handle = dlopen(NULL, RTLD_NOW);
                if (!handle) {
                    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "dlopen(NULL) failed: %s", dlerror());
                } else {
                    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "using global handle %p", handle);
                }
            } else {
                __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "dlopen(libgame.so) -> %p", handle);
            }
        });
        return handle;
    }

    // Core hook function: 'func' must be a void* pointing to the replacement function,
    // and 'origFunc' must be a void** where the original pointer will be stored.
    inline void hook(const char* symbol, void* func, void** origFunc) {
        void* h = getHandle();
        if (!h) {
            __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "hook: no handle to resolve %s", symbol);
            return;
        }
        void* addr = dlsym(h, symbol);
        if (!addr) {
            __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "dlsym('%s') failed: %s", symbol, dlerror());
            return;
        }
        __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "hook: resolving %s -> %p, installing %p", symbol, addr, func);
        DobbyHook(addr, func, origFunc);
    }
}

// Helper macro that safely casts a typed function pointer to void* and passes
// the address of the trampoline variable as void**.
#define HOOK(symbol, newfunc, trampoline) do { \
    /*
     * Use reinterpret_cast between function-pointer types first (allowed),
     * then convert that function-pointer to void*; this avoids errors when
     * the replacement function has a different signature.
     */ \
    void* __newf = reinterpret_cast<void*>(reinterpret_cast<void(*)()>(newfunc)); \
    hooking::hook(symbol, __newf, reinterpret_cast<void**>(&(trampoline))); \
} while (0)


class HookManager {
public:
    static void *getPointerFromSymbol(void *handle, const char *symbol) {
        return reinterpret_cast<void *>(dlsym(handle, symbol));
    }
};