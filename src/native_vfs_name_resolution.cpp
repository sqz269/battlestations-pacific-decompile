#include "bsp/native_vfs_name_resolution.hpp"
#include "bsp/native_vfs_device_route.hpp"
#include "bsp/native_vfs_lookup_routes.hpp"
#include "bsp/native_vfs_open_logging.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_lua_script_overrides.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U = std::uint32_t;
using I = std::int32_t;
static_assert(sizeof(void*) == 4, "native VFS resolution requires Win32");
void* at(void* p, U n = 0) noexcept { return reinterpret_cast<void*>(reinterpret_cast<U>(p) + n); }
const void* at(const void* p, U n = 0) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<U>(p) + n);
}
U word(const void* p, U n = 0) noexcept { return *static_cast<const volatile U*>(at(p, n)); }
void put(void* p, U n, U v) noexcept { *static_cast<volatile U*>(at(p, n)) = v; }
void* pointer(const void* p, U n = 0) noexcept { return reinterpret_cast<void*>(word(p, n)); }
std::uint8_t byte(const void* p, U n) noexcept {
    return *static_cast<const volatile std::uint8_t*>(at(p, n));
}
void require(bool ok, const char* message) { if (!ok) throw std::invalid_argument(message); }

struct Temporary {
    NativeString value;
    bool live{};
    void* header() noexcept { return &value; }
    void empty() noexcept { put(header(), 0, 0); put(header(), 4, 0); live = true; }
    void destroy(NativeStringStorage& strings) noexcept {
        if (!live) return;
        live = false;
        destroy_native_string_header_0041dd20(header(), strings);
    }
    void destroy_captured(NativeStringStorage& strings, void* data, U length) noexcept {
        live = false;
        if (data) strings.release(static_cast<char*>(data), length + 1u);
    }
};
struct ResolutionState {
    NativeVfsNameResolutionPhase phase{NativeVfsNameResolutionPhase::fresh};
    U site{}, failure{};
    void* manager{};
    void* output{};
    NativeVfsNameResolutionContext* context{};
    Temporary original, direct_name;
    alignas(4) unsigned char visitor[0x14]; // Native padding5..7 is not initialized.
    bool visitor_live{};
    Temporary basename, extension, full_stem;
    Temporary group_candidate, empty_candidate, tree_candidate;
    Temporary constructed, dot, stem_dot, stem_extension;
    U log_builder[6];
    U selected_group[2]{};
    U prefix_iterator[2]{};
    U tree_iterator[2]{};
    void* tree_upper{};
    void* extension_owner{};
    void* extension_node{};
    ActualNativeStringPoolStorage& strings() const { return context->device.lookup.physical.strings; }
    const SingletonLifetimeCallbacks& crt() const { return context->device.lookup.physical.invalid_parameters; }
};
void invalid(ResolutionState& s, U site) {
    s.site = site;
    const auto& crt = s.crt();
    crt.invalid_parameter(crt.context); // Native returning handler; execution continues.
}
void assign_live(ResolutionState& s, void* output, const void* source, U resize_site, U copy_site) {
    if (output == source) return;
    s.site = resize_site;
    resize_native_string_header_0041dd40(output, s.strings(), word(source), true);
    if (word(source)) {
        const auto length = word(output);
        const auto* input = pointer(source, 4);
        auto* destination = pointer(output, 4);
        s.site = copy_site;
        if (length) std::memmove(destination, input, length);
    }
}
void copy_temporary(ResolutionState& s, Temporary& t, const void* source, U resize_site, U copy_site) {
    put(t.header(), 0, 0); put(t.header(), 4, 0);
    assign_live(s, t.header(), source, resize_site, copy_site);
    t.live = true; // Initial-copy failure has no caller destruction state.
}
void literal(ResolutionState& s, Temporary& t, const char* text, U site) {
    s.site = site;
    construct_native_string_cstring_0041e870(t.header(), text, s.strings());
    t.live = true;
}
void assign_captured(ResolutionState& s, void* output, Temporary& input,
    U resize_site, U copy_site) {
    // These success branches retain EDI=data and ESI=length across resize.
    auto* data = pointer(input.header(), 4);
    const auto length = word(input.header());
    if (output != input.header()) {
        s.site = resize_site;
        resize_native_string_header_0041dd40(output, s.strings(), length, true);
        if (length) {
            const auto count = word(output);
            auto* destination = pointer(output, 4);
            s.site = copy_site;
            if (count) std::memmove(destination, data, count);
        }
    }
    input.destroy_captured(s.strings(), data, length);
}
void cleanup_constructed(ResolutionState& s) noexcept {
    s.stem_extension.destroy(s.strings()); s.stem_dot.destroy(s.strings());
    s.dot.destroy(s.strings()); s.constructed.destroy(s.strings());
}

// BDC680, including captured normal dot cleanup versus current-header FH3.
bool try_constructed(ResolutionState& s, void* prefix_output,
    const void* stem, const void* extension) {
    try {
        copy_temporary(s, s.constructed, prefix_output, 0x00bdc6bd, 0x00bdc6d4);
        put(s.dot.header(), 0, 0); put(s.dot.header(), 4, 0);
        s.site = 0x00bdc6f1;
        resize_native_string_header_0041dd40(s.dot.header(), s.strings(), 1, true);
        auto* dot_data = pointer(s.dot.header(), 4);
        if (dot_data) {
            s.site = 0x00bdc70c;
            std::memmove(dot_data, ".", word(s.dot.header()) + 1u);
        }
        s.dot.live = true;
        s.site = 0x00bdc728;
        auto* first = concatenate_native_string_headers_004261a0(stem,
            s.stem_dot.header(), s.dot.header(), s.strings());
        s.stem_dot.live = true;
        s.site = 0x00bdc73e;
        auto* suffix = concatenate_native_string_headers_004261a0(first,
            s.stem_extension.header(), extension, s.strings());
        s.stem_extension.live = true;
        const auto added = word(suffix);
        if (added) {
            const auto old = word(s.constructed.header());
            s.site = 0x00bdc75f;
            resize_native_string_header_0041dd40(s.constructed.header(), s.strings(), old + added, true);
            const auto* from = pointer(suffix, 4);
            auto* to = at(pointer(s.constructed.header(), 4), old);
            s.site = 0x00bdc770;
            std::memmove(to, from, added);
            dot_data = pointer(s.dot.header(), 4); // BDC775; only the nonempty branch reloads.
        }
        s.stem_extension.destroy(s.strings());
        s.stem_dot.destroy(s.strings());
        s.dot.destroy_captured(s.strings(), dot_data, word(s.dot.header()));
        // Capture current manager+08 once, after all three release callbacks.
        const auto entry = word(pointer(s.manager), 8);
        s.site = 0x00bdc7f3;
        require(entry == 0x00bdd440,
            "unreconstructed current native VFS manager candidate-existence method");
        const bool found = exists_native_vfs_file_00bdd440(s.manager,
            s.constructed.header(), s.context->device.lookup) != 0;
        if (found) assign_live(s, prefix_output, s.constructed.header(), 0x00bdc80b, 0x00bdc823);
        s.constructed.destroy(s.strings());
        return found;
    } catch (...) {
        if (!s.failure) s.failure = s.site;
        cleanup_constructed(s);
        throw;
    }
}

// BDA2C0 is the upper bound over the SAME actual20h nodes consumed by BDA260.
void* upper_bound(ResolutionState& s, void* tree, const void* key) {
    auto* selected = pointer(tree, 4);
    auto* node = pointer(selected, 4);
    while (byte(node, 0x1d) == 0) {
        int comparison;
        if (word(key) == 0) comparison = word(node, 0x0c) ? -1 : 0;
        else if (word(node, 0x0c) == 0) comparison = 1;
        else {
            const auto* right = static_cast<const char*>(pointer(node, 0x10));
            const auto* left = static_cast<const char*>(pointer(key, 4));
            s.site = 0x00bda2f1;
            comparison = _stricmp(left, right);
        }
        if (comparison < 0) { selected = node; node = pointer(node); }
        else node = pointer(node, 8);
    }
    return selected;
}
bool try_extension_prefixes(ResolutionState& s, void* output,
    const void* stem, const void* extension) {
    auto* const tree = at(s.manager, 0x54);
    s.site = 0x00bddace;
    s.tree_upper = upper_bound(s, tree, extension); // Upper before lower, not reversed.
    if (!tree) invalid(s, 0x00bddadd);
    s.site = 0x00bddae5;
    auto* first = lower_bound_native_physical_index_00bda260(tree, extension);
    if (!tree) { invalid(s, 0x00bddaf0); if (!tree) invalid(s, 0x00bddaf9); }
    if (first == s.tree_upper) return false;
    put(s.tree_iterator, 0, reinterpret_cast<U>(tree));
    put(s.tree_iterator, 4, reinterpret_cast<U>(first));
    try {
        for (;;) {
            auto* owner = pointer(s.tree_iterator);
            if (!owner || owner != tree) invalid(s, 0x00bddb18);
            if (pointer(s.tree_iterator, 4) == s.tree_upper) return false;
            auto* end = pointer(tree, 4);
            if (!owner || owner != tree) invalid(s, 0x00bddb36);
            if (pointer(s.tree_iterator, 4) == end) return false;
            if (!owner) invalid(s, 0x00bddb49);
            if (pointer(s.tree_iterator, 4) == pointer(owner, 4)) invalid(s, 0x00bddb57);
            auto* source = at(pointer(s.tree_iterator, 4), 0x14);
            copy_temporary(s, s.tree_candidate, source, 0x00bddb7c, 0x00bddb94);
            s.site = 0x00bddbb5;
            if (try_constructed(s, s.tree_candidate.header(), stem, extension)) {
                assign_captured(s, output, s.tree_candidate, 0x00bddc0f, 0x00bddc20);
                return true;
            }
            s.tree_candidate.destroy(s.strings());
            s.site = 0x00bddbe9;
            advance_native_physical_index_00bd9860(s.tree_iterator, s.crt());
        }
    } catch (...) {
        if (!s.failure) s.failure = s.site;
        s.tree_candidate.destroy(s.strings());
        throw;
    }
}

// BDB1E0: first matching actual group only; values are node+8 string headers.
void find_group(ResolutionState& s, const void* extension) {
    auto* group = pointer(pointer(s.manager, 0x64));
    for (;;) {
        if (group == pointer(s.manager, 0x64)) {
            put(s.selected_group, 0, reinterpret_cast<U>(at(s.manager, 0x60)));
            put(s.selected_group, 4, word(s.manager, 0x64));
            return;
        }
        if (group == pointer(s.manager, 0x64)) invalid(s, 0x00bdb20c);
        auto* value = pointer(pointer(group, 0x14));
        for (;;) {
            if (group == pointer(s.manager, 0x64)) invalid(s, 0x00bdb225);
            if (value == pointer(group, 0x14)) break;
            if (value == pointer(group, 0x14)) invalid(s, 0x00bdb23f);
            const auto wanted = word(extension);
            if (word(value, 8) == wanted) {
                int comparison;
                if (!word(value, 8)) comparison = wanted ? -1 : 0;
                else if (!wanted) comparison = 1;
                else {
                    s.site = 0x00bdb26e;
                    comparison = _stricmp(static_cast<const char*>(pointer(value, 0x0c)),
                        static_cast<const char*>(pointer(extension, 4)));
                }
                if (!comparison) {
                    put(s.selected_group, 0, reinterpret_cast<U>(at(s.manager, 0x60)));
                    put(s.selected_group, 4, reinterpret_cast<U>(group));
                    return;
                }
            }
            if (value == pointer(group, 0x14)) invalid(s, 0x00bdb284);
            value = pointer(value);
        }
        if (group == pointer(s.manager, 0x64)) invalid(s, 0x00bdb29a);
        group = pointer(group);
    }
}
void* checked_group_payload(ResolutionState& s) { // BDAE40, returning validations.
    if (!pointer(s.selected_group)) invalid(s, 0x00bdae48);
    auto* owner = pointer(s.selected_group);
    auto* node = pointer(s.selected_group, 4);
    if (node == pointer(owner, 4)) invalid(s, 0x00bdae57);
    return at(pointer(s.selected_group, 4), 8);
}
void advance_prefix_iterator(ResolutionState& s) { // 4BF5A0, not a container copy.
    if (!pointer(s.prefix_iterator)) invalid(s, 0x004bf5a8);
    auto* owner = pointer(s.prefix_iterator);
    auto* node = pointer(s.prefix_iterator, 4);
    if (node == pointer(owner, 4)) invalid(s, 0x004bf5b7);
    put(s.prefix_iterator, 4, word(pointer(s.prefix_iterator, 4)));
}
void require_group(ResolutionState& s, U site, bool owner_test = false) {
    auto* owner = pointer(s.selected_group);
    if (owner_test && !owner) invalid(s, site);
    if (pointer(s.selected_group, 4) == pointer(owner, 4)) invalid(s, site);
}

struct PathSites { U group, owner, sentinel, copy, copied, probe, advance, assign, assigned; };
// Later paths have the same native loop with different live string arguments.
bool paths(ResolutionState& s, const void* stem, const void* extension,
    const PathSites& sites, bool first_full = false) {
    void* owner;
    if (first_full) {
        s.site = 0x00bddeee;
        owner = at(checked_group_payload(s), 0x14);
    } else owner = at(pointer(s.selected_group, 4), 0x1c);
    auto* node = pointer(pointer(owner, 4));
    put(s.prefix_iterator, 0, reinterpret_cast<U>(owner));
    put(s.prefix_iterator, 4, reinterpret_cast<U>(node));
    for (;;) {
        if (first_full && !pointer(s.selected_group)) invalid(s, 0x00bddf0d);
        require_group(s, sites.group);
        auto* current_group = pointer(s.selected_group, 4);
        const bool basename_original = sites.probe == 0x00bde180;
        auto* end = basename_original ? pointer(owner, 4) : pointer(current_group, 0x20);
        if ((first_full && !owner) || owner != at(current_group, 0x1c)) invalid(s, sites.owner);
        if (node == end) return false;
        if (first_full && !owner) invalid(s, 0x00bddf43);
        auto* checked_owner = basename_original ? at(pointer(s.selected_group, 4), 0x1c) : owner;
        if (node == pointer(checked_owner, 4)) invalid(s, sites.sentinel);
        if (first_full) {
            s.site = 0x00bddf5a;
            copy_construct_native_string_header_00426060(s.group_candidate.header(), at(node, 8), s.strings());
            s.group_candidate.live = true;
        } else copy_temporary(s, s.group_candidate, at(node, 8), sites.copy, sites.copied);
        if (!first_full && (sites.probe == 0x00bde460 || sites.probe == 0x00bde88e)) {
            if (s.extension_node == pointer(s.extension_owner, 4))
                invalid(s, sites.probe == 0x00bde460 ? 0x00bde445 : 0x00bde873);
            extension = at(s.extension_node, 8); // Reload actual alternate node after copy callbacks.
        }
        s.site = sites.probe;
        if (try_constructed(s, s.group_candidate.header(), stem, extension)) {
            if (first_full) {
                s.site = 0x00bddfc2;
                assign_native_string_header_00425f40(s.output, s.group_candidate.header(), s.strings());
                s.group_candidate.destroy(s.strings());
            } else assign_captured(s, s.output, s.group_candidate, sites.assign, sites.assigned);
            return true;
        }
        s.group_candidate.destroy(s.strings());
        if (first_full) {
            s.site = 0x00bddfa8;
            advance_prefix_iterator(s);
            node = pointer(s.prefix_iterator, 4);
            owner = pointer(s.prefix_iterator);
        } else {
            checked_owner = basename_original ? at(pointer(s.selected_group, 4), 0x1c) : owner;
            if (node == pointer(checked_owner, 4)) invalid(s, sites.advance);
            node = pointer(node);
            put(s.prefix_iterator, 4, reinterpret_cast<U>(node));
        }
    }
}
bool alternate_differs(ResolutionState& s, U site) {
    const void* header = at(s.extension_node, 8);
    if (!word(header)) return word(s.extension.header()) != 0;
    if (!word(s.extension.header())) return true;
    s.site = site;
    return _stricmp(static_cast<const char*>(pointer(header, 4)),
        static_cast<const char*>(pointer(s.extension.header(), 4))) != 0;
}
void start_extensions(ResolutionState& s, U site, bool new_owner) {
    require_group(s, site);
    if (new_owner) s.extension_owner = at(pointer(s.selected_group, 4), 0x10);
    s.extension_node = pointer(pointer(s.extension_owner, 4));
}
bool extension_end(ResolutionState& s, U group_site, U owner_site, U node_site) {
    require_group(s, group_site);
    auto* group = pointer(s.selected_group, 4);
    auto* end = pointer(group, 0x14);
    if (s.extension_owner != at(group, 0x10)) invalid(s, owner_site);
    if (s.extension_node == end) return true;
    if (s.extension_node == pointer(s.extension_owner, 4)) invalid(s, node_site);
    return false;
}
void next_extension(ResolutionState& s, U site) {
    if (s.extension_node == pointer(s.extension_owner, 4)) invalid(s, site);
    s.extension_node = pointer(s.extension_node);
}

bool direct_resolution(ResolutionState& s, const void* input, void* output) {
    auto& strings = s.strings();
    put(s.visitor, 0, 0x00d683f4);
    *static_cast<volatile std::uint8_t*>(at(s.visitor, 4)) = 0;
    put(s.visitor, 8, 0); put(s.visitor, 0x0c, 0); put(s.visitor, 0x10, 0xffffffffu);
    s.visitor_live = true;
    try {
        copy_temporary(s, s.direct_name, input, 0x00bdd73c, 0x00bdd753);
        s.site = 0x00bdd764;
        normalize_native_resource_path_header_00bee690(s.direct_name.header(), strings);
        s.site = 0x00bdd775;
        visit_native_vfs_lookup_mounts_00bdd0a0(s.manager, s.direct_name.header(),
            s.visitor, s.context->device.lookup);
        const bool found = byte(s.visitor, 4) != 0;
        if (found) assign_live(s, output, at(s.visitor, 8), 0x00bdd795, 0x00bdd7ac);
        s.direct_name.destroy(strings);
        s.visitor_live = false;
        s.site = found ? 0x00bdd7e3 : 0x00bdd82d;
        destroy_native_vfs_device_visitor_00bdb680(s.visitor, strings);
        return found;
    } catch (...) {
        if (!s.failure) s.failure = s.site;
        s.direct_name.destroy(strings);
        if (s.visitor_live) {
            s.visitor_live = false;
            destroy_native_vfs_device_visitor_00bdb680(s.visitor, strings);
        }
        throw;
    }
}
void cleanup_candidates(ResolutionState& s) noexcept {
    s.group_candidate.destroy(s.strings()); s.empty_candidate.destroy(s.strings());
    s.full_stem.destroy(s.strings()); s.extension.destroy(s.strings()); s.basename.destroy(s.strings());
}
void resize_fill(ResolutionState& s, Temporary& t, U size, U site) {
    const auto old = word(t.header());
    s.site = site;
    resize_native_string_header_0041dd40(t.header(), s.strings(), size, true);
    const auto now = word(t.header());
    if (old < now) std::memset(at(pointer(t.header(), 4), old), 0x20, now - old);
}
bool resolve_candidates(ResolutionState& s) {
    s.site = 0x00bddcac;
    lowercase_native_string_header_004bcc00(s.output);
    const auto length = static_cast<I>(word(s.output)); // Captured EDX controls the slash loop.
    for (I i = 0; i < length; ++i) {
        auto* current = static_cast<char*>(at(pointer(s.output, 4), static_cast<U>(i)));
        if (*current == '\\') *current = '/';
    }
    s.site = 0x00bddcd8;
    if (direct_resolution(s, s.output, s.output)) return true;
    s.site = 0x00bddcf1;
    const auto slash = reverse_find_native_string_bytes_004bcb80(
        *static_cast<const NativeString*>(s.output), "/", 0x7fffffff);
    U start = slash == 0xffffffffu ? 0u : slash;
    auto* data = pointer(s.output, 4);
    if (!data || !word(s.output)) return false;
    if (static_cast<I>(start) < 0) start = 0;
    else if (start > word(s.output)) return false;
    s.site = 0x00bddd30;
    const auto dot = start + static_cast<U>(std::strcspn(static_cast<const char*>(at(data, start)), "."));
    if (dot == word(s.output) || static_cast<I>(dot) <= 0) return false;
    try {
        data = pointer(s.output, 4);
        if (!data) data = const_cast<char*>(s.context->empty_name_0109cef0);
        literal(s, s.basename, static_cast<const char*>(at(data, static_cast<U>(slash) + 1u)), 0x00bddd66);
        data = pointer(s.output, 4);
        if (!data) data = const_cast<char*>(s.context->empty_name_0109cef0);
        literal(s, s.extension, static_cast<const char*>(at(data, dot + 1u)), 0x00bddd88);
        s.site = 0x00bddd9a;
        copy_construct_native_string_header_00426060(s.full_stem.header(), s.output, s.strings());
        s.full_stem.live = true;
        resize_fill(s, s.basename, dot - static_cast<U>(slash) - 1u, 0x00bdddb6);
        resize_fill(s, s.full_stem, dot, 0x00bddde4);
        const auto captured_full_length = word(s.full_stem.header());
        auto* captured_full_data = pointer(s.full_stem.header(), 4);
        s.site = 0x00bdde1c;
        if (try_extension_prefixes(s, s.output, s.full_stem.header(), s.extension.header())) {
            s.full_stem.destroy_captured(s.strings(), captured_full_data, captured_full_length);
            cleanup_candidates(s);
            return true;
        }
        s.site = 0x00bddeae;
        find_group(s, s.extension.header());
        auto* group_owner = pointer(s.selected_group);
        auto* group_end = pointer(s.manager, 0x64);
        if (!group_owner || group_owner != at(s.manager, 0x60)) invalid(s, 0x00bddec5);
        if (pointer(s.selected_group, 4) != group_end) {
            s.site = 0x00bddedd;
            if (unequal_native_string_headers_00449af0(s.full_stem.header(), s.basename.header()) &&
                paths(s, s.full_stem.header(), s.extension.header(),
                    {0x00bddf1b,0x00bddf32,0x00bddf4d,0,0,0x00bddf77,0,0,0}, true)) {
                cleanup_candidates(s); return true;
            }
        }
        s.site = 0x00bde07d;
        if (unequal_native_string_headers_00449af0(s.full_stem.header(), s.basename.header())) {
            s.site = 0x00bde098;
            if (try_extension_prefixes(s, s.output, s.basename.header(), s.extension.header())) {
                cleanup_candidates(s); return true;
            }
        }
        group_owner = pointer(s.selected_group);
        group_end = pointer(s.manager, 0x64);
        if (!group_owner || group_owner != at(s.manager, 0x60)) invalid(s, 0x00bde0bb);
        if (pointer(s.selected_group, 4) == group_end) { cleanup_candidates(s); return false; }
        if (!group_owner) invalid(s, 0x00bde0d0);
        if (pointer(s.selected_group, 4) == pointer(group_owner, 4)) invalid(s, 0x00bde0da);
        if (paths(s, s.basename.header(), s.extension.header(),
            {0x00bde0f9,0x00bde10c,0x00bde125,0x00bde148,0x00bde160,0x00bde180,0x00bde1b9,0x00bde1e1,0x00bde1f2})) {
            cleanup_candidates(s); return true;
        }
        s.site = 0x00bde21c;
        if (unequal_native_string_headers_00449af0(s.full_stem.header(), s.basename.header())) {
            start_extensions(s, 0x00bde236, true);
            while (!extension_end(s, 0x00bde250, 0x00bde263, 0x00bde271)) {
                if (alternate_differs(s, 0x00bde299)) {
                    if (s.extension_node == pointer(s.extension_owner, 4)) invalid(s, 0x00bde2aa);
                    s.site = 0x00bde2bd;
                    if (try_extension_prefixes(s, s.output, s.full_stem.header(), at(s.extension_node, 8))) {
                        cleanup_candidates(s); return true;
                    }
                }
                next_extension(s, 0x00bde2cf);
            }
            start_extensions(s, 0x00bde2ec, false);
            while (!extension_end(s, 0x00bde307, 0x00bde31a, 0x00bde32c)) {
                if (alternate_differs(s, 0x00bde354)) {
                    s.empty_candidate.empty();
                    if (s.extension_node == pointer(s.extension_owner, 4)) invalid(s, 0x00bde37e);
                    s.site = 0x00bde392;
                    if (try_constructed(s, s.empty_candidate.header(), s.full_stem.header(), at(s.extension_node, 8))) {
                        assign_captured(s, s.output, s.empty_candidate, 0x00bde4f7, 0x00bde508);
                        cleanup_candidates(s); return true;
                    }
                    require_group(s, 0x00bde3ac);
                    if (paths(s, s.full_stem.header(), at(s.extension_node, 8),
                        {0x00bde3cd,0x00bde3e0,0x00bde3f2,0x00bde413,0x00bde42b,0x00bde460,0x00bde496,0x00bde58c,0x00bde59d})) {
                        cleanup_candidates(s); return true;
                    }
                    s.empty_candidate.destroy(s.strings());
                }
                next_extension(s, 0x00bde4d3);
            }
        }
        start_extensions(s, 0x00bde62f, true);
        while (!extension_end(s, 0x00bde649, 0x00bde65c, 0x00bde66e)) {
            if (alternate_differs(s, 0x00bde696)) {
                if (s.extension_node == pointer(s.extension_owner, 4)) invalid(s, 0x00bde6a7);
                s.site = 0x00bde6ba;
                if (try_extension_prefixes(s, s.output, s.basename.header(), at(s.extension_node, 8))) {
                    cleanup_candidates(s); return true;
                }
            }
            next_extension(s, 0x00bde6c8);
        }
        start_extensions(s, 0x00bde750, false);
        while (!extension_end(s, 0x00bde76b, 0x00bde77e, 0x00bde790)) {
            if (alternate_differs(s, 0x00bde7b8)) {
                require_group(s, 0x00bde7d5);
                if (paths(s, s.basename.header(), at(s.extension_node, 8),
                    {0x00bde7fd,0x00bde810,0x00bde822,0x00bde841,0x00bde859,0x00bde88e,0x00bde8c0,0x00bde8fd,0x00bde90e})) {
                    cleanup_candidates(s); return true;
                }
            }
            next_extension(s, 0x00bde8d9);
        }
        cleanup_candidates(s);
        return false;
    } catch (...) {
        if (!s.failure) s.failure = s.site;
        cleanup_candidates(s);
        throw;
    }
}
void log_resolved_name(void* manager, const void* original, const void* resolved,
    NativeVfsOpenLoggingContext& context, U* builder, U& site) {
    if (!context.file_log_0109cee8 || !byte(manager, 0x79)) return;
    site = 0x00bdeb6a;
    auto* current = construct_native_log_builder_00426500(builder, context.strings);
    try {
        site = 0x00bdeb92;
        current = append_native_log_cstring_00bd1a60(current, "<SRCH><", context.strings);
        site = 0x00bdeb99;
        current = append_native_log_header_00bd1a20(current, original, context.strings);
        site = 0x00bdeba0;
        current = append_native_log_cstring_00bd1a60(current, "|", context.strings);
        site = 0x00bdeba7;
        current = append_native_log_header_00bd1a20(current, resolved, context.strings);
        site = 0x00bdebae;
        append_native_log_cstring_00bd1a60(current, ">", context.strings);
    } catch (...) {
        destroy_native_log_builder_00425f80(builder, context.strings);
        throw;
    }
    site = 0x00bdebbe;
    destroy_native_log_builder_00425f80(builder, context.strings);
}
} // namespace

struct NativeVfsNameResolutionAcquired::Impl : ResolutionState {};
NativeVfsNameResolutionAcquired::NativeVfsNameResolutionAcquired() : impl_(std::make_unique<Impl>()) {}
NativeVfsNameResolutionAcquired::~NativeVfsNameResolutionAcquired() {
    if (impl_->phase != NativeVfsNameResolutionPhase::fresh &&
        impl_->phase != NativeVfsNameResolutionPhase::complete) std::terminate();
}
NativeVfsNameResolutionPhase NativeVfsNameResolutionAcquired::phase() const noexcept { return impl_->phase; }
U NativeVfsNameResolutionAcquired::active_call_site() const noexcept { return impl_->site; }
U NativeVfsNameResolutionAcquired::failure_site() const noexcept { return impl_->failure; }

void log_native_vfs_resolved_name_00bdeb40(void* manager, const void* original,
    const void* resolved, NativeVfsOpenLoggingContext& context) {
    U builder[6];
    U site{};
    log_resolved_name(manager, original, resolved, context, builder, site);
}
bool resolve_native_vfs_direct_name_00bdd6e0(void* manager, const void* input,
    void* output, NativeVfsNameResolutionContext& context,
    NativeVfsNameResolutionAcquired& acquired) {
    auto& s = *acquired.impl_;
    require(s.phase == NativeVfsNameResolutionPhase::fresh,
        "native VFS direct resolution cannot replay its frame");
    require(context.device.lookup.device == &context.device,
        "native VFS direct resolution requires one actual device/lookup domain");
    s.manager = manager; s.output = output; s.context = &context;
    s.phase = NativeVfsNameResolutionPhase::candidates;
    try {
        const bool found = direct_resolution(s, input, output);
        s.phase = NativeVfsNameResolutionPhase::complete;
        return found;
    } catch (...) {
        if (!s.failure) s.failure = s.site;
        s.phase = NativeVfsNameResolutionPhase::failed;
        throw;
    }
}
bool resolve_native_vfs_existing_name_00bdf4c0(void* manager, void* name,
    NativeVfsNameResolutionContext& context, NativeVfsNameResolutionAcquired& acquired) {
    auto& s = *acquired.impl_;
    require(s.phase == NativeVfsNameResolutionPhase::fresh, "native VFS resolution cannot replay its frame");
    require(context.device.lookup.device == &context.device &&
        &context.device.lookup.physical.strings == &context.logging.strings && context.empty_name_0109cef0,
        "native VFS resolution requires one actual device/lookup/string domain");
    s.manager = manager; s.output = name; s.context = &context;
    s.phase = NativeVfsNameResolutionPhase::normalizing;
    try {
        s.site = 0x00bdf4e3;
        normalize_native_resource_path_header_00bee690(name, s.strings());
        copy_temporary(s, s.original, name, 0x00bdf509, 0x00bdf521);
        s.phase = NativeVfsNameResolutionPhase::candidates;
        s.site = 0x00bdf534;
        const bool result = resolve_candidates(s);
        if (result) {
            s.phase = NativeVfsNameResolutionPhase::logging;
            s.site = 0x00bdf547;
            log_resolved_name(manager, s.original.header(), name, context.logging, s.log_builder, s.site);
        }
        s.original.destroy(s.strings());
        s.phase = NativeVfsNameResolutionPhase::complete;
        return result;
    } catch (...) {
        if (!s.failure) s.failure = s.site;
        s.original.destroy(s.strings());
        s.phase = NativeVfsNameResolutionPhase::failed;
        throw;
    }
}
} // namespace bsp
