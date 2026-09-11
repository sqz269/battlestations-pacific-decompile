#pragma once

#include "bsp/native_string.hpp"
#include <cstdint>
#include <memory>

namespace bsp {
class MemoryStream;
class PhysicalFile;

using SoundFileOpenCallback = std::int32_t (__stdcall*)(const char*, std::int32_t,
    std::uint32_t*, void**, void**);
using SoundFileCloseCallback = std::int32_t (__stdcall*)(void*, void*);
using SoundFileReadCallback = std::int32_t (__stdcall*)(void*, void*, std::uint32_t,
    std::uint32_t*, void*);
using SoundFileSeekCallback = std::int32_t (__stdcall*)(void*, std::uint32_t, void*);

// Explicit binding of global VFS 0109CEEC. Resolve may mutate the canonical
// string even on false; open receives a distinct copy and exactly flags=2.
// open transfers one intrusive reference to an actual callable stream owner.
// Required slots: +0 final release, +1C seek(low,high,origin), +24 read, and
// +2C size(optional high word). Its signed interlocked count is at owner+4.
// Image-address profile words alone are NOT a callable stream dispatch table.
struct NativeSoundFileContext {
    NativeStringStorage& strings;
    void* user;
    bool (*resolve)(void* user, NativeString& name);
    void* (*open_read_only)(void* user, const NativeString& name, std::uint32_t flags);
};

// Host orchestration only. Bind before FMOD can call open; keep the binding,
// context, VFS and storage alive and fixed until FMOD stops all callbacks and
// releases its file handles. Returns the previous binding for later restoration.
NativeSoundFileContext* bind_sound_file_context(NativeSoundFileContext*) noexcept;

// Original Win32 stack ABIs. Required native inputs are not null-guarded;
// unicode/userdata are ignored. Open leaves userdata and failed-open size alone.
// Exceptions from supplied VFS/storage contracts retain C++ unwind cleanup;
// hosts must keep throwing operations outside the installed FMOD callback path.
std::int32_t __stdcall sound_file_open_00a7d410(const char* name, std::int32_t unicode,
    std::uint32_t* size, void** handle, void** userdata);
std::int32_t __stdcall sound_file_close_00a7b750(void* handle, void* userdata);
std::int32_t __stdcall sound_file_read_00a79930(void* handle, void* destination,
    std::uint32_t requested, std::uint32_t* actual, void* userdata);
std::int32_t __stdcall sound_file_seek_00a79970(void* handle,
    std::uint32_t position, void* userdata);

struct SoundFileCallbackBundle {
    SoundFileOpenCallback open = sound_file_open_00a7d410;
    SoundFileCloseCallback close = sound_file_close_00a7b750;
    SoundFileReadCallback read = sound_file_read_00a79930;
    SoundFileSeekCallback seek = sound_file_seek_00a79970;
};

// 00BE41A0: original ECX stream, stack optional high output, RET4. Calls the
// current +30 size virtual, writes the high DWORD only when nonnull, returns
// low EAX. New C++ entrypoint; the adapter below supplies its callable bridge.
std::uint32_t sound_stream_size_low_00be41a0(void* handle, std::uint32_t* high);

// Explicit host adapters over existing real stream implementations. A partial
// native-callable table implements only the slots needed above (and size +30).
// This is not the complete game stream ABI or the canonical native pool owner.
// Each successful creation returns a count-one owner; close consumes that one
// reference. shared_ptr retains the SAME stream/cursor without cloning/reset.
// Return null for null/unusable typed input; allocation can throw. Callers must
// serialize each cursor and not use a closed adapter. No original-image address
// is installed as a function pointer.
void* create_sound_memory_stream_adapter(std::shared_ptr<MemoryStream>);
void* create_sound_physical_stream_adapter(std::shared_ptr<PhysicalFile>);

struct SoundFileStreamAdapterStatus {
    bool last_operation_succeeded = true;
    std::uint32_t last_system_error = 0; // PhysicalFile's actual DWORD error.
};
// Only accepts a still-live owner returned by the two adapter factories.
// MemoryStream guard failures report false/error0; their outputs stay unchanged.
// These diagnostics do not alter the native callback's result or seek policy.
SoundFileStreamAdapterStatus sound_file_stream_adapter_status(void* handle) noexcept;
} // namespace bsp
