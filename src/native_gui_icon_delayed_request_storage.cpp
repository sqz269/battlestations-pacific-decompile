#include "bsp/native_gui_icon_delayed_request_storage.hpp"
#include "bsp/native_gui_named_value_array_append_storage.hpp"
#include "bsp/native_filestore_factory.hpp"
#include "bsp/native_filestore_provider_lifetime.hpp"
#include "bsp/native_filestore_request.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_crt_stricmp.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(const void* p, Word d = 0) noexcept {
    return *static_cast<const volatile Word*>(pointer(bits(p) + d));
}
void put(Word& home, Word value) noexcept { static_cast<volatile Word&>(home) = value; }
Word live(const Word& home) noexcept { return static_cast<const volatile Word&>(home); }
void put_byte(Word& home, std::uint8_t value) noexcept {
    *reinterpret_cast<volatile std::uint8_t*>(&home) = value;
}
void require(bool ok, const char* text) { if (!ok) throw std::logic_error(text); }
void return_buffer(void* data, Word bytes, NativeStringRawPoolContext& strings) {
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, data, bytes, strings.actual_small_returns_disabled_01090aa4);
}
} // namespace
struct NativeGuiIconDelayedRequestStorageFrame::Storage {
    Storage() {} // No Native home seeds, including nested append frame.
    Word temporary[3], preserve_home, source_home, native_state;
    Word file_homes[2]; // [name pointer, callback], ordinary real outgoing HOMEs.
    NativeGuiNamedValueArrayAppendStorageFrame append;
};
NativeGuiIconDelayedRequestStorageFrame::NativeGuiIconDelayedRequestStorageFrame()
    : storage_(new Storage) {}
NativeGuiIconDelayedRequestStorageFrame::~NativeGuiIconDelayedRequestStorageFrame() = default;

void request_native_gui_icon_delayed_loads_storage_00ab5f00(void* actual,
    volatile Word (&homes)[3], NativeGuiIconDelayedRequestStorageFrame& frame,
    NativeGuiIconDelayedRequestStorageContext& c) {
    auto& a = frame.observation_; auto& f = *frame.storage_;
    require(a.phase == NativeGuiIconDelayedRequestStorageFrame::Phase::fresh,
        "Icon delayed request requires a distinct fresh retained frame");
    a.phase = NativeGuiIconDelayedRequestStorageFrame::Phase::entered;
    a.actual_singleton = actual; a.argument_home_identity = &homes[0];
    put(f.native_state, 0);
    Word index = 0, delta = 0, length_register = 0; // Register carriers, not seeded Native homes.
    try {
        a.active_native_site = 0x00ab5f1e;
        if (static_cast<std::int32_t>(read(actual, 8)) > 0) {
            length_register = homes[0];
            do {
                const Word base = read(actual, 4);
                void* const row = pointer(base + delta);
                const Word row_length = read(row);
                if (row_length == length_register) {
                    Word comparison;
                    if (row_length == 0) comparison = length_register == 0 ? 0u : 0xffffffffu;
                    else if (length_register == 0) comparison = 1;
                    else {
                        const Word incoming_data = homes[1];
                        const Word row_data = read(row, 4);
                        a.active_native_site = 0x00ab5f5f;
                        comparison = static_cast<Word>(compare_native_crt_strings_00bf7fbf(
                            static_cast<const char*>(pointer(row_data)),
                            static_cast<const char*>(pointer(incoming_data)),
                            c.locale_changed_0109de1c, c.stricmp_callees));
                        length_register = homes[0]; // ONLY after this actual call.
                    }
                    if (comparison == 0) {
                        const Word current_base = read(actual, 4);
                        const Word current_owner = homes[2];
                        if (read(pointer(current_base + delta), 8) == current_owner) {
                            a.duplicate = true;
                            break;
                        }
                    }
                }
                ++index; delta += 0xcu;
            } while (static_cast<std::int32_t>(index) < static_cast<std::int32_t>(read(actual, 8)));
        }
        if (!a.duplicate) {
            require(c.prepare_file_call != nullptr, "Icon delayed request requires its actual raw Runtime supplier");
            auto& call = c.prepare_file_call(c.file_provider); // Cold, before native4FC150.
            a.file_call = &call;
            require(call.phase_ == NativeGuiIconDelayedRequestFileInvocation::Phase::fresh,
                "Icon delayed FileStore call cannot replay");
            call.validate_source_entry();
            call.phase_ = NativeGuiIconDelayedRequestFileInvocation::Phase::entered;
            try {
                a.active_native_site = call.site_ = 0x00ab5f90;
                void* const factory = get_native_filestore_factory_004fc150(call.factory_);
                a.factory = factory;
                put(f.file_homes[1], 0x00ab58d0); // Identity only, never a callable DWORD.
                put(f.file_homes[0], bits(const_cast<const Word*>(&homes[0])));
                a.active_native_site = call.site_ = 0x00ab5fa1;
                void* const provider = get_native_file_store_provider_00be80b0(factory, call.provider_);
                a.provider = provider;
                a.active_native_site = call.site_ = 0x00ab5fa8;
                a.request_result = request_native_file_store_00be7cd0(provider,
                    pointer(live(f.file_homes[0])), live(f.file_homes[1]), call.request_, call.acquired_);
                a.request_returned = true;
                call.phase_ = NativeGuiIconDelayedRequestFileInvocation::Phase::normal_returned;
            } catch (...) {
                call.phase_ = NativeGuiIconDelayedRequestFileInvocation::Phase::unknown_source_escape;
                call.failure_ = std::current_exception();
                throw;
            }
            if (a.request_result) {
                put(f.temporary[0], 0); put(f.temporary[1], 0); put(f.temporary[2], 0);
                const Word requested_length = homes[0];
                put(f.preserve_home, 1); put_byte(f.native_state, 1);
                a.active_native_site = 0x00ab5fcd;
                resize_native_string_header_0041dd40_exact(f.temporary, c.strings,
                    requested_length, &f.preserve_home, c.feature_word_0109eea4);
                const Word source_length_test = homes[0];
                void* const captured_data = pointer(live(f.temporary[1]));
                const Word captured_length = live(f.temporary[0]);
                if (source_length_test != 0) {
                    const Word source_data = homes[1];
                    a.active_native_site = 0x00ab5fe7;
                    copy_native_crt_bytes_00bf7680(captured_data, pointer(source_data),
                        captured_length, c.feature_word_0109eea4);
                }
                const Word owner = homes[2];
                put(f.source_home, bits(f.temporary)); // PUSH source before temp.owner store.
                put(f.temporary[2], owner);
                a.active_native_site = 0x00ab5fff;
                append_native_gui_named_value_array_storage_00ab4990(pointer(bits(actual) + 4u),
                    f.source_home, f.append, c.strings, c.feature_word_0109eea4);
                a.append_returned = true;
                put_byte(f.native_state, 0);
                if (captured_data) {
                    a.active_native_site = 0x00ab6013;
                    return_buffer(captured_data, captured_length + 1u, c.strings);
                }
                a.temporary_returned = true;
            }
        }
        a.active_native_site = a.duplicate ? 0x00ab605b : 0x00ab601f;
        void* const captured_data = pointer(homes[1]);
        put(f.native_state, 0xffffffffu);
        if (captured_data) {
            const Word bytes = (a.duplicate ? length_register : homes[0]) + 1u;
            a.active_native_site = 0x00ab603a;
            return_buffer(captured_data, bytes, c.strings);
        }
        a.parameter_returned = true;
        a.phase = NativeGuiIconDelayedRequestStorageFrame::Phase::normal_returned;
    } catch (...) {
        a.escaped_native_site = a.active_native_site;
        a.failure = std::current_exception();
        a.phase = NativeGuiIconDelayedRequestStorageFrame::Phase::unknown_source_escape;
        throw; // Retain actual reached effects; no guessed Native parameter action.
    }
}
} // namespace bsp
