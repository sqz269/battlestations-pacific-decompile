#pragma once
#include "bsp/native_language_catalog_storage.hpp"
#include "bsp/native_profile_hints_owner.hpp"
#include <array>
#include <optional>

namespace bsp {
class ActualNativeStringPoolStorage;
struct SingletonLifetimeCallbacks;
struct NativeLanguageCatalogProducerCalls {
    virtual ~NativeLanguageCatalogProducerCalls()=default;
    virtual void* allocate_scanner_00bf681b();
    virtual void free_00bf65ac(void*);
    // Existing 438E10 source uses the host CRT case-insensitive comparison.
    virtual int compare_00438e10(const char*,const char*);
};
struct NativeLanguageCatalogProducerContext {
    void* actual_catalog_00f88974;
    NativeStringRawPoolContext& strings;
    ActualNativeStringPoolStorage& actual_strings;
    const SingletonLifetimeCallbacks& invalid_parameters;
    NativeVfsEnumerationContext& enumeration;
    NativeLanguageResourceEnumerationCalls& enumeration_calls;
    NativeStreamTextScannerContext& scanner;
    NativeProfileHintsOwnerContext& hints;
    NativeLanguageCatalogAllocationCalls& catalog_calls;
    NativeLanguageCatalogProducerCalls& calls;
    const char* extension_00d15e10;
    const char* directory_00d15e08;
    // English, French, Italian, German, Spanish, English authentic.
    std::array<const char*,6> allowed_prefixes;
    // lanfile, lockit_id, voice_dir, fontpath, in native comparison order.
    std::array<const char*,4> keywords;
    const NativeStreamTextStackPreimages& scanner_stack_preimages;
};
// All contexts above must borrow the same raw pool and VFS publications.
// The operation owns the native frame's outstanding list/header/scanner state
// on source failure; the caller resolves it before acknowledging cleanup.
struct NativeLanguageCatalogProducerOperation final {
    using Phase=NativeLanguageCatalogStorageOperation::Phase;
    Phase phase{Phase::fresh};
    alignas(4) std::byte extension[8],directory[8],filename_argument[8],list[12],row[32];
    void* scanner{};
    void* scanner_holder{};
    void* captured_scanner{};
    void* current_member{};
    void* pending_string_data{};
    std::uint32_t pending_string_bytes{},native_site{},descriptors{},appended_rows{};
    std::int32_t unwind_state{-1};
    std::uint8_t ignored_string_success{};
    bool extension_live{},directory_live{},list_live{},filename_argument_live{};
    bool scanner_allocation_live{},scanner_constructed{},row_live{};
    std::optional<NativeLanguageResourceEnumerationOperation> enumeration;
    std::optional<NativeStreamTextScannerOperation> scanner_operation;
    std::optional<NativeProfileHintsOwnerOperation> hints;
    std::optional<NativeLanguageCatalogStorageOperation> storage;
    // Native list+0 is an uninitialized preserved word. Supply its preimage;
    // no source default or semantic interpretation is imposed on that word.
    explicit NativeLanguageCatalogProducerOperation(std::uint32_t list_word_preimage) noexcept;
    ~NativeLanguageCatalogProducerOperation();
    void acknowledge_diagnostic_cleanup() noexcept;
    NativeLanguageCatalogProducerOperation(const NativeLanguageCatalogProducerOperation&)=delete;
    NativeLanguageCatalogProducerOperation& operator=(const NativeLanguageCatalogProducerOperation&)=delete;
};
// Complete normal 8D7BC0, native no inputs/RET. Count!=0 skips all work. Open
// each descriptor before filtering; actual hints+8 bypasses filename prefixes.
// Parse four fields into raw pooled headers; ignore BEF020's success byte.
// Unknown tokens retry without consumption and may never return, as native.
// Append copied row, destroy/free captured scanner, then reverse-destroy row.
// Clear remaining list storage, reload/free sentinel, leave dead frame headers.
// No native ABI/FH3/SEH, private stack aliasing or token overflow claim.
void build_native_language_catalog_008d7bc0(
    NativeLanguageCatalogProducerContext&,NativeLanguageCatalogProducerOperation&);
} // namespace bsp
