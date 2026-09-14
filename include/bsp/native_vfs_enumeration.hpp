#pragma once

#include "bsp/native_string_vector.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS enumeration requires MSVC Win32.
#endif

namespace bsp {
class ActualNativeStringPoolStorage;
struct SingletonLifetimeCallbacks;

// BE1130 captures the CURRENT provider numeric table+14 entry and passes the
// original actual8h suffix/extension, low flag byte and actual0Ch result
// vector. The application must implement every reached provider target;
// unsupported entries fail explicitly. No projected provider is accepted.
class NativeVfsProviderEnumerationDispatch {
public:
    virtual ~NativeVfsProviderEnumerationDispatch() = default;
    virtual void enumerate_entry(std::uintptr_t captured_entry, void* actual_provider,
        const void* actual_suffix, const void* actual_extension,
        std::uint32_t flags, NativeStringVectorStorage& actual_output) = 0;
};

class NativeVfsEnumerationDuplicateLog {
public:
    virtual ~NativeVfsEnumerationDuplicateLog() = default;
    virtual void rejected_duplicate(const char* native_name_or_empty) = 0;
};

struct NativeVfsEnumerationContext {
    ActualNativeStringPoolStorage& strings;
    const SingletonLifetimeCallbacks& invalid_parameters;
    NativeVfsProviderEnumerationDispatch& providers;
    NativeVfsEnumerationDuplicateLog& duplicates;
    const char* empty_name_0109cef0;
    // Original readable numeric visitor profile; entries +4=BE1130 and
    // +8=BDBEB0 are checked at the current dispatch, not called as native code.
    const void* actual_visitor_profile_00d6846c;
};

// BDD990: ECX actual manager; stack directory,extension,flags,output-list;
// RET10h. Output is the caller's actual 0Ch intrusive string list, not a
// projected vector. The native callback does not normalize/alias the directory.
void enumerate_native_vfs_resources_00bdd990(void* actual_manager,
    const void* actual_directory, const void* actual_extension,
    std::uint32_t flags, void* actual_output_list, NativeVfsEnumerationContext&);

// Enumeration-specialized BDD0A0 traversal of the actual mount tree. Other
// visitor profiles are handled by their existing dedicated source paths.
void visit_native_vfs_enumeration_mounts_00bdd0a0(void* actual_manager,
    const void* actual_directory, void* actual_14h_visitor,
    NativeVfsEnumerationContext&);
} // namespace bsp
