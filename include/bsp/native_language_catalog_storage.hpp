#pragma once
#include "bsp/native_stream_text_scanner.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
struct NativeVfsEnumerationContext;
struct alignas(4) NativeLanguageCatalogRowStorage {std::byte bytes[0x20];};
struct alignas(4) NativeLanguageCatalogVectorStorage {std::byte bytes[0x0c];};
static_assert(sizeof(NativeLanguageCatalogRowStorage)==0x20);
static_assert(sizeof(NativeLanguageCatalogVectorStorage)==0x0c);

struct NativeLanguageCatalogAllocationCalls {
    virtual ~NativeLanguageCatalogAllocationCalls()=default;
    virtual void* allocate_00bf55be(std::uint32_t wrapped_bytes);
    virtual void free_00bf6989(void*);
};
// Caller retains interrupted operations and explicitly resolves their actual
// allocations before acknowledging cleanup. No native FH3 unwind is projected.
struct NativeLanguageCatalogStorageOperation final {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    void* owner{};
    const void* source{};
    void* candidate{};
    void* current_row{};
    std::uint32_t copied_rows{},destroyed_rows{},completed_members{};
    std::uint32_t native_site{},row_site{};
    std::int32_t unwind_state{-1},row_unwind_state{-1};
    NativeStreamTextScannerOperation scanner;
    ~NativeLanguageCatalogStorageOperation();
    void acknowledge_diagnostic_cleanup() noexcept;
    NativeLanguageCatalogStorageOperation()=default;
    NativeLanguageCatalogStorageOperation(const NativeLanguageCatalogStorageOperation&)=delete;
    NativeLanguageCatalogStorageOperation& operator=(const NativeLanguageCatalogStorageOperation&)=delete;
};
// 8D5890: ECX actual20h destination, stack source, EAX destination, RET4.
// Zero/copy each actual8h string in ascending order. Identity zeroes all four
// headers without returning their old allocations. Each nonidentity copy tests
// CURRENT source length after resize and copies CURRENT destination length.
void* copy_construct_native_language_row_008d5890(void*,const void*,
    NativeStringRawPoolContext&,NativeLanguageCatalogStorageOperation&);
// 8D4F50: ECX actual20h row, RET. Return four current strings in descending
// member order. Capture each data pointer before its length; leave headers dead.
void destroy_native_language_row_008d4f50(void*,NativeStringRawPoolContext&,
    NativeLanguageCatalogStorageOperation&);
// Actual vector layout: pointer/count/capacity at0/4/8; global F88974 uses it.
// 8D59C0: ECX vector, stack signed requested, RET4. Clamp>=1; grow only.
// Copy current rows, destroy current old rows, free CURRENT old backing, then
// publish captured new backing/requested capacity. Count is never reset.
void reserve_native_language_catalog_008d59c0(void*,std::int32_t,
    NativeStringRawPoolContext&,NativeLanguageCatalogAllocationCalls&,
    NativeLanguageCatalogStorageOperation&);
// 8D6AF0: ECX vector, stack source row, RET4. Equality-only full check, wrapped
// signed doubling/clamp, then CURRENT insertion slot; source may not dangle
// across real growth. A zero computed slot skips construction but advances count.
void append_native_language_catalog_008d6af0(void*,const void*,
    NativeStringRawPoolContext&,NativeLanguageCatalogAllocationCalls&,
    NativeLanguageCatalogStorageOperation&);
// 8D47F0: ECX pointer-to-scanner, RET. Capture scanner once, destroy/free it,
// then clear the CURRENT holder field. Uses existing full raw scanner context.
void release_native_language_scanner_008d47f0(void*,NativeStreamTextScannerContext&,
    NativeLanguageCatalogStorageOperation&);
// 553C80: ECX actual8h header, stack prefix/unsigned position, EAX raw result,
// RET8. Guard failure preserves data-pointer upper24 bits and clears AL.
std::uint32_t native_language_prefix_result_00553c80(
    const void*,const char*,std::uint32_t) noexcept;

struct NativeLanguageResourceEnumerationCalls {
    virtual ~NativeLanguageResourceEnumerationCalls()=default;
    virtual void* allocate_sentinel_004c3020();
    virtual void normalize_00bee690(void*,NativeStringRawPoolContext&);
    virtual void enumerate_00bdd990(void* manager,const void* directory,
        const void* extension,std::uint32_t flags,void* output,NativeVfsEnumerationContext&);
};
struct NativeLanguageResourceEnumerationOperation final {
    using Phase=NativeLanguageCatalogStorageOperation::Phase;
    Phase phase{Phase::fresh};
    void* output{};
    alignas(4) std::byte directory[8];
    std::uint32_t native_site{};
    std::int32_t unwind_state{-1};
    ~NativeLanguageResourceEnumerationOperation();
    void acknowledge_diagnostic_cleanup() noexcept;
    NativeLanguageResourceEnumerationOperation()=default;
    NativeLanguageResourceEnumerationOperation(const NativeLanguageResourceEnumerationOperation&)=delete;
    NativeLanguageResourceEnumerationOperation& operator=(const NativeLanguageResourceEnumerationOperation&)=delete;
};
// 886280: ECX manager; stack output-list/directory/extension/flags; EAXoutput,
// RET10h. Allocate actual sentinel, preserve output+0, clone/normalize a raw
// directory temporary, dispatch existing BDD990, then return its current buffer.
// Both supplied pool contexts must borrow the same actual publication cells.
void* enumerate_native_language_resources_00886280(void* manager,void* output,
    const void* directory,const void* extension,std::uint32_t flags,
    NativeStringRawPoolContext&,NativeVfsEnumerationContext&,
    NativeLanguageResourceEnumerationCalls&,NativeLanguageResourceEnumerationOperation&);
} // namespace bsp
