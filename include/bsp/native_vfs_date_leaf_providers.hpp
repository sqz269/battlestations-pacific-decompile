#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

struct SingletonLifetimeCallbacks;

struct NativeFileStoreNameIterator {
    void* owner_00;
    void* node_04;
};
static_assert(sizeof(NativeFileStoreNameIterator) == 8);
static_assert(offsetof(NativeFileStoreNameIterator, node_04) == 4);

// Full 00443D00: two actual eight-byte name-header pointers on the native
// stack, RET8, AL boolean. Zero stored length is empty regardless of data.
// Otherwise compare the current C strings through CRT _stricmp, without a
// stored-length tie-break. Read right data before left data. No header checks.
bool less_native_string_headers_00443d00(
    const void* actual_left_header, const void* actual_right_header);

// Full 00BE54D0: ECX actual primary tree, name stack, RET4, EAX node.
// Tree+4 is its head; head+4 is the root. Nodes have links+0/+4/+8, name+C/+10
// and sentinel byte+19. No node copy, normalization, allocation or validation.
void* lower_bound_native_file_store_name_00be54d0(
    void* actual_tree, const void* actual_name_header);

// Full 00BE5A50: ECX actual tree, output/name stack, RET8, EAX output.
// Lower-bound runs before the null-owner CRT check. The head is reread on
// fallback after comparison. Publish output owner then node, using captured
// values even when output aliases the tree or a node. BF6713 may return.
void* find_native_file_store_name_00be5a50(void* actual_tree,
    void* actual_iterator_output, const void* actual_name_header,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Full 00BE5C80: ECX actual FileStore, hidden output/name stack, RET8,
// EAX output. Capture provider+18 head before zeroing all five output DWORDs
// in descending offset order; search the current tree at provider+14. A node
// different from that captured head writes five FFFFFFFF words, descending.
// Uses the existing returning CRT boundary for iterator-owner validation.
void* query_native_file_store_date_00be5c80(void* actual_file_store,
    void* actual_date_output, const void* actual_name_header,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Full 23-byte leaves BB9D50 and BBB640: ECX unused, hidden output/name stack,
// EAX output, RET8. Neither reads its name or provider. Five descending zero
// stores; no archive access. Profiles D64390 and D643C4 respectively. MSAR's
// concrete factory does not establish normal-startup reachability.
void* query_native_mpkg_file_date_00bb9d50(
    void* actual_date_output, const void* actual_name_header) noexcept;
void* query_native_msar_file_date_00bbb640(
    void* actual_date_output, const void* actual_name_header) noexcept;

// Caller supplies actual Win32 backing storage and a callable invalid_parameter
// binding for reached checks. No FileStore/std::map/NativeString overlay is
// created. CRT and invalid-parameter dispatch remain explicit host boundaries;
// these C++ APIs do not claim the original binary calling convention or SEH ABI.
// Evidence: docs/NATIVE_VFS_DATE_LEAF_PROVIDERS.md and companion audit report.

} // namespace bsp
