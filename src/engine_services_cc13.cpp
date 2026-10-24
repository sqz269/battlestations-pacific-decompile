// Segments 96-97 leaves with a reviewed ledger name and no reconstruction record (cc13-files1).
// Reconstructed and build-tested only: not ABI-compatible, not fixture- or game-validated, and not
// wired into any game host. See docs/CC13_FILES_1.md and the address notes in
// bsp/engine_services_cc13.hpp.

#include "bsp/engine_services_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"

#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_vfs_mount_records.hpp"
#include "bsp/native_string_byte_append.hpp"
#include "bsp/native_string_vector_lookup.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdio>
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp::engine_services_cc13 {

static_assert(sizeof(void*) == 4, "native layouts are Win32");
static_assert(sizeof(NativeString) == 8, "native string is 8 bytes");
static_assert(sizeof(NativeIterator) == 8, "checked iterator is 8 bytes");
static_assert(sizeof(NativeContainerHeader) == 12, "container header is 0Ch bytes");
static_assert(sizeof(NativeTrackedSection) == 0x1C, "tracked section is 1Ch bytes");
static_assert(sizeof(CRITICAL_SECTION) == 0x18, "the recursion count follows a 18h-byte section");
static_assert(sizeof(PrefixTreeNode) == 0x20, "prefix tree node is 20h bytes");
static_assert(sizeof(ErrorContextNode) == 0x20, "error context node is 20h bytes");
static_assert(offsetof(ErrorContextNode, color_1c) == 0x1C && offsetof(ErrorContextNode, isnil_1d) == 0x1D,
              "error context node colour/isnil offsets");
static_assert(offsetof(PrefixTreeNode, color_1c) == 0x1C && offsetof(PrefixTreeNode, isnil_1d) == 0x1D,
              "prefix tree node colour/isnil offsets");

namespace {

constexpr std::uint32_t kSingletonBaseVtable = 0x00CE3818u; // BSP_SingletonBase_ScalarDelete table
constexpr std::uint32_t kRefCountedBaseVtable = 0x00CEB130u; // BSP_RefCounted_InvokeDeletingDestructor

unsigned char* bytes(void* p) noexcept { return static_cast<unsigned char*>(p); }
const unsigned char* bytes(const void* p) noexcept { return static_cast<const unsigned char*>(p); }

template<class T> T& field(void* p, std::size_t offset) noexcept {
    return *reinterpret_cast<T*>(bytes(p) + offset);
}
template<class T> const T& field(const void* p, std::size_t offset) noexcept {
    return *reinterpret_cast<const T*>(bytes(p) + offset);
}

void store_vtable(void* object, std::uint32_t vtable) noexcept { field<std::uint32_t>(object, 0) = vtable; }

void* allocate(std::size_t size) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, size, size});
}

// The native count arithmetic is 32-bit two's complement.
std::int32_t add32(std::int32_t n, std::int32_t k) noexcept {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(n) + static_cast<std::uint32_t>(k));
}

void release_object(void* object) {
    auto* const count = reinterpret_cast<volatile LONG*>(bytes(object) + 4);
    if (InterlockedDecrement(count) == 0) {
        using Slot0 = void(__thiscall*)(void*);
        reinterpret_cast<Slot0>(field<void*>(field<void*>(object, 0), 0))(object);
    }
}

template<class Fn> Fn vtable_slot(void* object, std::size_t offset) noexcept {
    return reinterpret_cast<Fn>(field<void*>(field<void*>(object, 0), offset));
}

// Zero the destination header, then the BE0A30 copy fragment (self-copy guard, resize with preserve,
// copy the CURRENT destination length when the source length is non-zero).
void construct_string_copy(NativeString& destination, const NativeString& source, NativeStringStorage& strings) {
    ::new (&destination) NativeString();
    destination.copy_from_00be0a30_fragment(strings, source);
}
NativeString& string_at(void* p, std::size_t offset) noexcept { return field<NativeString>(p, offset); }
const NativeString& string_at(const void* p, std::size_t offset) noexcept { return field<NativeString>(p, offset); }

// The listings' inline name equality: equal lengths, then empty/empty without a data access, otherwise
// _stricmp == 0.
bool names_equal(std::uint32_t a_length, const char* a_data, std::uint32_t b_length, const char* b_data) {
    if (a_length != b_length) return false;
    int r;
    if (a_length == 0) r = -static_cast<int>(b_length != 0);
    else if (b_length == 0) r = 1;
    else r = _stricmp(a_data, b_data);
    return r == 0;
}

// EnterCriticalSection, then the +18h recursion count; the reverse on exit.
class TrackedLock final {
public:
    explicit TrackedLock(NativeTrackedSection* section) : section_(section) {
        if (section_) {
            EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(section_->section));
            ++section_->recursion_18;
        }
    }
    ~TrackedLock() {
        if (section_) {
            --section_->recursion_18;
            LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(section_->section));
        }
    }
    TrackedLock(const TrackedLock&) = delete;
    TrackedLock& operator=(const TrackedLock&) = delete;
private:
    NativeTrackedSection* section_;
};

// The lifetime manager's section (00415350()->+10h), as src/live_effect_manager_lifetime.cpp takes it.
class ManagerLock final {
public:
    explicit ManagerLock(SingletonLifetimeDomain& domain)
        : section_(domain.get_manager_00415350()->system_owner().section_10) {
        if (section_) {
            singleton_enter_critical_section(*section_);
            ++section_->recursion_18;
        }
    }
    ~ManagerLock() {
        if (section_) {
            --section_->recursion_18;
            singleton_leave_critical_section(*section_);
        }
    }
    ManagerLock(const ManagerLock&) = delete;
    ManagerLock& operator=(const ManagerLock&) = delete;
private:
    SystemSingletonCriticalSection* section_;
};

// Temporary native string released through 0041DD20 on every exit.
class TemporaryString final {
public:
    explicit TemporaryString(NativeStringStorage& strings) : strings_(strings) {}
    ~TemporaryString() { destroy_native_string_header_0041dd20(&value, strings_); }
    TemporaryString(const TemporaryString&) = delete;
    TemporaryString& operator=(const TemporaryString&) = delete;
    NativeString value;
private:
    NativeStringStorage& strings_;
};

// Shared VFS path preparation (00BE1B90 kind 1 and 00BE2210): '\' -> '/' over the signed length, then
// append '/' unless data[length - 1] is already '/' (data defaults to the empty buffer 0109CEF0, whose
// preceding byte the context supplies).
void slash_terminate(NativeString& path, VfsPathContext& context) {
    const std::int32_t length = static_cast<std::int32_t>(path.length());
    char* const data = path.data();
    for (std::int32_t i = 0; i < length; ++i) {
        if (data[i] == '\\') data[i] = '/';
    }
    const char last = data != nullptr ? data[path.length() - 1u] : context.byte_0109ceef;
    if (last != '/') append_native_string_byte_0054aa70(&path, '/', context.strings);
}

// ---- Red-black tree core over the MSVC 2005 node shape (left, parent, right, colour, isnil). ----
template<class Node> Node* tree_head(NativeContainerHeader& tree) noexcept { return static_cast<Node*>(tree.head); }

template<class Node> void rotate_left(NativeContainerHeader& tree, Node* where) noexcept {
    Node* const head = tree_head<Node>(tree);
    Node* const pnode = where->right;
    where->right = pnode->left;
    if (!pnode->left->isnil_1d) pnode->left->parent = where;
    pnode->parent = where->parent;
    if (where == head->parent) head->parent = pnode;
    else if (where == where->parent->left) where->parent->left = pnode;
    else where->parent->right = pnode;
    pnode->left = where;
    where->parent = pnode;
}

template<class Node> void rotate_right(NativeContainerHeader& tree, Node* where) noexcept {
    Node* const head = tree_head<Node>(tree);
    Node* const pnode = where->left;
    where->left = pnode->right;
    if (!pnode->right->isnil_1d) pnode->right->parent = where;
    pnode->parent = where->parent;
    if (where == head->parent) head->parent = pnode;
    else if (where == where->parent->right) where->parent->right = pnode;
    else where->parent->left = pnode;
    pnode->right = where;
    where->parent = pnode;
}

template<class Node> Node* tree_min(Node* node) noexcept {
    while (!node->left->isnil_1d) node = node->left;
    return node;
}
template<class Node> Node* tree_max(Node* node) noexcept {
    while (!node->right->isnil_1d) node = node->right;
    return node;
}

// The _Insert body shared by 00BE0370 and 00BF2270 after the node is bought (colour 0 = red).
template<class Node> void link_and_rebalance(NativeContainerHeader& tree, NativeIterator& result, bool add_left,
                                             Node* where, Node* node) {
    Node* const head = tree_head<Node>(tree);
    ++tree.size;
    if (where == head) {
        head->parent = node;
        head->left = node;
        head->right = node;
    } else if (!add_left) {
        where->right = node;
        if (where == head->right) head->right = node;
    } else {
        where->left = node;
        if (where == head->left) head->left = node;
    }
    Node* x = node;
    while (x->parent->color_1c == 0) {
        Node* const p = x->parent;
        Node* const g = p->parent;
        if (p == g->left) {
            Node* const y = g->right;
            if (y->color_1c == 0) {
                p->color_1c = 1;
                y->color_1c = 1;
                x->parent->parent->color_1c = 0;
                x = x->parent->parent;
            } else {
                if (x == p->right) {
                    rotate_left(tree, p);
                    x = p;
                }
                x->parent->color_1c = 1;
                x->parent->parent->color_1c = 0;
                rotate_right(tree, x->parent->parent);
            }
        } else {
            Node* const y = g->left;
            if (y->color_1c == 0) {
                p->color_1c = 1;
                y->color_1c = 1;
                x->parent->parent->color_1c = 0;
                x = x->parent->parent;
            } else {
                if (x == p->left) {
                    rotate_right(tree, p);
                    x = p;
                }
                x->parent->color_1c = 1;
                x->parent->parent->color_1c = 0;
                rotate_left(tree, x->parent->parent); // inline in 00BE0370
            }
        }
    }
    tree_head<Node>(tree)->parent->color_1c = 1;
    result.node = node;
    result.container = &tree;
}

void check_tree_size(const NativeContainerHeader& tree) {
    if (tree.size > 0x0FFFFFFDu) throw std::length_error("map/set<T> too long");
}

} // namespace

// ---------------------------------------------------------------------------------------------------
// Allocation and small frees.

void raw_free(void* block) noexcept { singleton_lifetime_free(block); }

void* allocate_native_elements(std::uint32_t count, std::uint32_t element_size) {
    if (count != 0 && 0xFFFFFFFFu / count < element_size) throw std::bad_alloc();
    return allocate(static_cast<std::size_t>(count * element_size));
}

void free_owned_block(void** slot) noexcept {
    if (*slot != nullptr) {
        singleton_lifetime_free(*slot);
        *slot = nullptr;
    }
}

std::int64_t query_performance_counter_00be3240() noexcept {
    LARGE_INTEGER value;
    QueryPerformanceCounter(&value);
    return value.QuadPart;
}

// ---------------------------------------------------------------------------------------------------
// Constant leaves.

bool return_false() noexcept { return false; }
bool return_true() noexcept { return true; }
void no_operation() noexcept {}
std::uint32_t win32_platform_window_style_00beca30() noexcept { return 0x00C40000u; }

std::uint32_t allocation_stats_zeroed_budget_00be2720() noexcept {
    std::uint32_t local[0x34 / 4];
    std::memset(local, 0, sizeof local);
    return local[0x1C / 4];
}

std::uint32_t read_published_word(const std::uint32_t& word) noexcept { return word; }
void* fingerprint_payload_data_00be9620(void* payload) noexcept { return bytes(payload) + 4; }
std::uint32_t read_dword_field(const void* object, std::size_t offset) noexcept {
    return field<std::uint32_t>(object, offset);
}

bool scene_type_is_kind_of_00c30230(const std::uint32_t (&types_0109e9c4)[3], std::uint32_t type) noexcept {
    for (std::uint32_t entry : types_0109e9c4) {
        if (entry == type) return true;
    }
    return false; // EAX 0109E900: AL 0
}

// 00BEE141..00BEE16A at 24-bit precision control: FLD/FSTP [ESP] (the scalar), then
// FMUL/FSTP per lane, each product rounded to 24 bits before the store.
void scale_vector4_pc24_00bee140(float* out, const float* scalar, const float* v) noexcept {
    const float s = *scalar;
    out[0] = static_cast<float>(cc13x87::x87_mul24(s, v[0]));
    out[1] = static_cast<float>(cc13x87::x87_mul24(v[1], s));
    out[2] = static_cast<float>(cc13x87::x87_mul24(v[2], s));
    out[3] = static_cast<float>(cc13x87::x87_mul24(s, v[3]));
}

void scale_vector4_00bee140(float* out, const float* scalar, const float* v) noexcept {
    if constexpr (kScaleVector4Pc24Cc13Bound) {
        scale_vector4_pc24_00bee140(out, scalar, v);
    } else {
        const float s = *scalar;
        out[0] = s * v[0];
        out[1] = v[1] * s;
        out[2] = v[2] * s;
        out[3] = s * v[3];
    }
}

// ---------------------------------------------------------------------------------------------------
// Virtual-table and singleton-base teardown.

void* scalar_delete_storing_vtable(void* object, std::uint32_t flags, std::uint32_t vtable) noexcept {
    store_vtable(object, vtable);
    if ((flags & 1u) != 0) singleton_lifetime_free(object);
    return object;
}

void reset_singleton_base(void* object, std::uint32_t& published) noexcept {
    published = 0;
    store_vtable(object, kSingletonBaseVtable);
}

void* scalar_delete_singleton_base(void* object, std::uint32_t flags, std::uint32_t& published) noexcept {
    reset_singleton_base(object, published);
    if ((flags & 1u) != 0) singleton_lifetime_free(object);
    return object;
}

void* scalar_delete_via(void* object, std::uint32_t flags, DestroyRoutine routine, EngineServiceCalls& calls) {
    switch (routine) {
    case DestroyRoutine::allocation_stats_00be27f0: calls.destroy_allocation_stats_00be27f0(object); break;
    case DestroyRoutine::window_platform_base_00be2b10: calls.destroy_window_platform_base_00be2b10(object); break;
    case DestroyRoutine::filestore_payload_00be5d70: calls.destroy_filestore_payload_00be5d70(object); break;
    case DestroyRoutine::frame_clock_base_00bedea0: calls.destroy_frame_clock_base_00bedea0(object); break;
    case DestroyRoutine::physical_read_request_00bf3880: calls.destroy_physical_read_request_names_00bf3880(object); break;
    case DestroyRoutine::vfs_request_payload_00be1220: calls.destroy_vfs_request_payload_00be1220(object); break;
    }
    if ((flags & 1u) != 0) singleton_lifetime_free(object);
    return object;
}

// ---------------------------------------------------------------------------------------------------
// Intrusive reference slots.

void release_ref_slot(void** slot) {
    if (*slot != nullptr) {
        release_object(*slot);
        *slot = nullptr;
    }
}

void** release_ref_slot_return_self_00be9b80(void** slot) {
    release_ref_slot(slot);
    return slot;
}

void release_ref_slot_keep_00be44a0(void** slot) {
    if (*slot != nullptr) release_object(*slot);
}

void release_ref_slot_unchecked_00be4500(void** slot) {
    release_object(*slot);
    *slot = nullptr;
}

void** assign_ref_slot_00bef1e0(void** slot, void* object) {
    release_ref_slot(slot);
    *slot = object;
    return slot;
}

void** vfs_open_stream_into_00be4380(void** out, void* vfs_0109ceec, std::uint32_t name, std::uint32_t flags) {
    using Open = void*(__thiscall*)(void*, std::uint32_t, std::uint32_t);
    *out = vtable_slot<Open>(vfs_0109ceec, 4)(vfs_0109ceec, name, flags);
    return out;
}

void stream_ref_reopen_00be44c0(void** slot, void* vfs_0109ceec, std::uint32_t name, std::uint32_t flags) {
    release_ref_slot_keep_00be44a0(slot);
    using Open = void*(__thiscall*)(void*, std::uint32_t, std::uint32_t);
    *slot = vtable_slot<Open>(vfs_0109ceec, 4)(vfs_0109ceec, name, flags);
}

// ---------------------------------------------------------------------------------------------------
// Native stream wrappers.

void stream_write_dword_00be40b0(void* stream, std::uint32_t value, std::uint32_t arg) {
    using Write = void(__thiscall*)(void*, const void*, std::uint32_t, std::uint32_t);
    vtable_slot<Write>(stream, 0x28)(stream, &value, 4, arg);
}

void stream_write_float_00be40f0(void* stream, float value, std::uint32_t arg) {
    using Write = void(__thiscall*)(void*, const void*, std::uint32_t, std::uint32_t);
    vtable_slot<Write>(stream, 0x28)(stream, &value, 4, arg);
}

void* stream_write_string_00be4120(void* stream, const void* text) {
    using Write = void(__thiscall*)(void*, const void*, std::uint32_t);
    vtable_slot<Write>(stream, 0x50)(stream, text, 0);
    return stream;
}

void* stream_read_dword_00be4160(void* stream, std::uint32_t* out) {
    using Read = std::uint32_t(__thiscall*)(void*, std::uint32_t);
    *out = vtable_slot<Read>(stream, 0x34)(stream, 0);
    return stream;
}

void* stream_read_float_00be4180(void* stream, float* out) {
    using Read = float(__thiscall*)(void*, std::uint32_t);
    *out = vtable_slot<Read>(stream, 0x44)(stream, 0);
    return stream;
}

void** stream_ref_read_dword_00be43f0(void** slot, std::uint32_t* out) {
    stream_read_dword_00be4160(*slot, out);
    return slot;
}

void** stream_ref_read_float_00be4410(void** slot, float* out) {
    stream_read_float_00be4180(*slot, out);
    return slot;
}

void stream_writer_write_dword_00bf0340(void** writer, std::uint32_t value, std::uint32_t* remaining) {
    using Write = void(__thiscall*)(void*, std::uint32_t, std::uint32_t*);
    std::uint32_t slot = value; // the argument slot doubles as the out-parameter
    vtable_slot<Write>(*writer, 0x54)(*writer, value, &slot);
    *remaining -= slot;
}

void stream_writer_write_float_00bf0360(void** writer, float value, std::uint32_t* remaining) {
    using Write = void(__thiscall*)(void*, float, std::uint32_t*);
    std::uint32_t slot;
    std::memcpy(&slot, &value, sizeof slot);
    vtable_slot<Write>(*writer, 0x58)(*writer, value, &slot);
    *remaining -= slot;
}

void stream_writer_write_string_00bf0390(void** writer, const void* text, std::uint32_t* remaining) {
    using Write = void(__thiscall*)(void*, const void*, std::uint32_t*);
    std::uint32_t slot = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(text));
    vtable_slot<Write>(*writer, 0x5C)(*writer, text, &slot);
    *remaining -= slot;
}

void stream_writer_write_bytes_00bf03b0(void** writer, const void* data, std::uint32_t size,
                                        std::uint32_t* remaining) {
    using Write = void(__thiscall*)(void*, const void*, std::uint32_t, std::uint32_t*);
    std::uint32_t slot = size;
    vtable_slot<Write>(*writer, 0x28)(*writer, data, size, &slot);
    *remaining -= slot;
}

void* memory_stream_cast_00bef4d0(void* stream, std::uint32_t memory_type_0109dba0) {
    if (stream != nullptr) {
        using IsKind = bool(__thiscall*)(void*, std::uint32_t);
        if (vtable_slot<IsKind>(stream, 0x0C)(stream, memory_type_0109dba0)) return stream;
    }
    return nullptr;
}

// ---------------------------------------------------------------------------------------------------
// Checked iterators.

namespace {
void* container_head(const NativeIterator& it) noexcept {
    return static_cast<const NativeContainerHeader*>(it.container)->head;
}
void validate_dereference(const NativeIterator& it, InvalidParameterSink& sink) {
    if (it.container == nullptr) sink.invalid_parameter_00bf6713();
    if (it.node == container_head(it)) sink.invalid_parameter_00bf6713();
}
} // namespace

void* tree_iterator_value(const NativeIterator& it, InvalidParameterSink& sink) {
    validate_dereference(it, sink);
    return bytes(it.node) + 0x0C;
}

void* list_iterator_value(const NativeIterator& it, InvalidParameterSink& sink) {
    validate_dereference(it, sink);
    return bytes(it.node) + 8;
}

bool iterator_equals(const NativeIterator& it, const NativeIterator& other, InvalidParameterSink& sink) {
    if (it.container == nullptr || it.container != other.container) sink.invalid_parameter_00bf6713();
    return it.node == other.node;
}

bool iterator_not_equals(const NativeIterator& it, const NativeIterator& other, InvalidParameterSink& sink) {
    if (it.container == nullptr || it.container != other.container) sink.invalid_parameter_00bf6713();
    return it.node != other.node;
}

NativeIterator& list_iterator_increment(NativeIterator& it, InvalidParameterSink& sink) {
    validate_dereference(it, sink);
    it.node = static_cast<NativeListNode*>(it.node)->next;
    return it;
}

NativeIterator& list_iterator_post_increment_00bec830(NativeIterator& it, NativeIterator& result,
                                                      InvalidParameterSink& sink) {
    result = it;
    list_iterator_increment(it, sink);
    return result;
}

NativeIterator& iterator_pre_step(NativeIterator& it, IteratorStep step, EngineServiceCalls& calls) {
    switch (step) {
    case IteratorStep::vfs_mount_advance_00bd97e0: calls.advance_vfs_mount_iterator_00bd97e0(it); break;
    case IteratorStep::physical_index_advance_00bd9860: calls.advance_physical_index_iterator_00bd9860(it); break;
    case IteratorStep::vfs_mount_retreat_00bd99b0: calls.retreat_vfs_mount_iterator_00bd99b0(it); break;
    case IteratorStep::physical_index_retreat_00bd9a40: calls.retreat_physical_index_iterator_00bd9a40(it); break;
    case IteratorStep::resident_advance_00be4c30: calls.advance_filestore_resident_iterator_00be4c30(it); break;
    case IteratorStep::resident_retreat_00be4d20: calls.retreat_filestore_resident_iterator_00be4d20(it); break;
    case IteratorStep::pending_advance_00be4e40: calls.advance_filestore_pending_iterator_00be4e40(it); break;
    case IteratorStep::pending_retreat_00be4db0: calls.retreat_filestore_pending_iterator_00be4db0(it); break;
    }
    return it;
}

// ---------------------------------------------------------------------------------------------------
// Container headers.

namespace {
void link_tree_head(NativeContainerHeader& tree, void* head, std::size_t isnil_offset) noexcept {
    tree.head = head;
    field<std::uint8_t>(head, isnil_offset) = 1;
    field<void*>(tree.head, 4) = tree.head;
    field<void*>(tree.head, 0) = tree.head;
    field<void*>(tree.head, 8) = tree.head;
    tree.size = 0;
}
} // namespace

NativeContainerHeader& construct_tree_header(NativeContainerHeader& tree, TreeHead kind, EngineServiceCalls& calls) {
    switch (kind) {
    case TreeHead::physical_index_00bdaba0: link_tree_head(tree, calls.allocate_tree_head_20_00bdaba0(), 0x1D); break;
    case TreeHead::vfs_mount_00bdabf0: link_tree_head(tree, calls.allocate_tree_head_24_00bdabf0(), 0x21); break;
    case TreeHead::filestore_resident_00be55e0:
        link_tree_head(tree, calls.allocate_filestore_resident_head_00be55e0(), 0x19);
        break;
    case TreeHead::filestore_pending_00be5690:
        link_tree_head(tree, calls.allocate_filestore_pending_head_00be5690(), 0x19);
        break;
    }
    return tree;
}

NativeContainerHeader& construct_pointer_list_00bdb3a0(NativeContainerHeader& list, EngineServiceCalls& calls) {
    list.head = calls.allocate_list_head_0c_00bda960();
    list.size = 0;
    return list;
}

NativeContainerHeader& construct_search_group_list_00bdb3d0(NativeContainerHeader& list, EngineServiceCalls& calls) {
    list.head = calls.allocate_list_head_28_00bda980();
    list.size = 0;
    return list;
}

NativeContainerHeader& construct_text_queue_00becd40(NativeContainerHeader& list, EngineServiceCalls& calls) {
    list.head = calls.create_text_queue_sentinel_00bec710();
    list.size = 0;
    return list;
}

void clear_tree(NativeContainerHeader& tree, TreeSubtree kind, EngineServiceCalls& calls) {
    void* const root = field<void*>(tree.head, 4);
    switch (kind) {
    case TreeSubtree::physical_index_00bdf7a0: calls.destroy_physical_index_subtree_00bdf7a0(root); break;
    case TreeSubtree::vfs_mount_00bdf7e0: calls.destroy_vfs_mount_subtree_00bdf7e0(root); break;
    case TreeSubtree::filestore_pending_00be66c0: calls.erase_filestore_pending_subtree_00be66c0(root); break;
    case TreeSubtree::filestore_resident_00be6720: calls.erase_filestore_resident_subtree_00be6720(root); break;
    }
    field<void*>(tree.head, 4) = tree.head;
    tree.size = 0;
    field<void*>(tree.head, 0) = tree.head;
    field<void*>(tree.head, 8) = tree.head;
}

void destruct_physical_index_tree(NativeContainerHeader& tree, EngineServiceCalls& calls) {
    NativeIterator result{};
    calls.erase_physical_index_range_00be0c30(result, tree, {&tree, field<void*>(tree.head, 0)}, {&tree, tree.head});
    singleton_lifetime_free(tree.head);
    tree.head = nullptr;
    tree.size = 0;
}

void destruct_vfs_mount_tree(NativeContainerHeader& tree, EngineServiceCalls& calls) {
    NativeIterator result{};
    calls.erase_vfs_mount_range_00be0d00(result, tree, {&tree, field<void*>(tree.head, 0)}, {&tree, tree.head});
    singleton_lifetime_free(tree.head);
    tree.head = nullptr;
    tree.size = 0;
}

void clear_pointer_list_00bdaae0(NativeContainerHeader& list) noexcept {
    auto* const head = static_cast<NativeListNode*>(list.head);
    NativeListNode* node = head->next;
    head->next = head;
    static_cast<NativeListNode*>(list.head)->prev = static_cast<NativeListNode*>(list.head);
    list.size = 0;
    if (node != list.head) {
        NativeListNode* next;
        do {
            next = node->next;
            singleton_lifetime_free(node);
            node = next;
        } while (next != list.head);
    }
}

void remove_from_pointer_list_00bdad10(NativeContainerHeader& list, const std::uint32_t* value,
                                       InvalidParameterSink& sink) {
    // The listing also tests the iterator's container (ECX) for null before each use; a reference
    // container cannot be null, so those tests are omitted.
    auto* const head = static_cast<NativeListNode*>(list.head);
    const std::uint32_t key = *value;
    NativeListNode* node = head->next;
    for (;;) {
        NativeListNode* const current = node;
        if (current == head) break;
        if (current == list.head) sink.invalid_parameter_00bf6713();
        if (field<std::uint32_t>(current, 8) == key) {
            if (current == list.head) sink.invalid_parameter_00bf6713();
            node = current->next;
            if (current != list.head) {
                current->prev->next = node;
                current->next->prev = current->prev;
                singleton_lifetime_free(current);
                --list.size;
            }
        } else {
            if (current == list.head) sink.invalid_parameter_00bf6713();
            node = current->next;
        }
    }
}

void erase_text_queue_node_00bec990(NativeContainerHeader& list, NativeIterator& result, NativeIterator where,
                                    InvalidParameterSink& sink) {
    if (where.container == nullptr) sink.invalid_parameter_00bf6713();
    auto* const node = static_cast<NativeListNode*>(where.node);
    if (node == container_head(where)) sink.invalid_parameter_00bf6713();
    NativeListNode* const next = node->next;
    if (node != list.head) {
        node->prev->next = next;
        node->next->prev = node->prev;
        singleton_lifetime_free(node);
        --list.size;
    }
    result.node = next;
    result.container = where.container;
}

namespace {
void grow_slots(NativeArrayHeader& array) {
    array.capacity = add32(add32(array.capacity, array.capacity), 2);
    auto* const fresh = static_cast<std::uint32_t*>(
        singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots,
                                     static_cast<std::size_t>(static_cast<std::uint32_t>(array.capacity) * 4u),
                                     static_cast<std::size_t>(static_cast<std::uint32_t>(array.capacity) * 4u)}));
    std::uint32_t* destination = fresh;
    // Unsigned count loop; each destination is tested for null before the store.
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(array.count); ++i, ++destination) {
        if (destination != nullptr) *destination = static_cast<std::uint32_t*>(array.data)[i];
    }
    if (array.data != nullptr) singleton_lifetime_free(array.data);
    array.data = fresh;
}
} // namespace

void grow_pointer_array_00bf2ee0(NativeArrayHeader& array) { grow_slots(array); }

void push_back_pointer_array_00bf3120(NativeArrayHeader& array, const std::uint32_t* value) {
    if (array.count == array.capacity) grow_slots(array);
    std::uint32_t* const slot = static_cast<std::uint32_t*>(array.data) + array.count;
    if (slot != nullptr) *slot = *value;
    array.count = add32(array.count, 1);
}

void push_back_physical_stream_slot_00bf5100(NativeArrayHeader& array, const std::uint32_t* value,
                                             EngineServiceCalls& calls) {
    if (array.count == array.capacity) {
        std::int32_t capacity = add32(array.capacity, array.capacity);
        if (capacity < 2) capacity = 1;
        calls.reserve_physical_stream_slots_00bf30c0(array, capacity);
    }
    std::uint32_t* const slot = static_cast<std::uint32_t*>(array.data) + array.count;
    if (slot != nullptr) *slot = *value;
    array.count = add32(array.count, 1);
}

// ---------------------------------------------------------------------------------------------------
// String-bearing records.

void release_string_at(void* object, std::size_t offset, NativeStringStorage& strings) noexcept {
    destroy_native_string_header_0041dd20(bytes(object) + offset, strings);
}

void destroy_string_pair_00bdb7c0(void* pair, NativeStringStorage& strings) noexcept {
    release_string_at(pair, 8, strings);
    release_string_at(pair, 0, strings);
}

void* scalar_delete_string_record(void* object, std::uint32_t flags, std::size_t offset,
                                  NativeStringStorage& strings) noexcept {
    release_string_at(object, offset, strings);
    if ((flags & 1u) != 0) singleton_lifetime_free(object);
    return object;
}

void* construct_named_value_record_00bdb920(void* record, const NativeString& name, std::uint32_t value,
                                            std::uint8_t flag, NativeStringStorage& strings) {
    construct_string_copy(string_at(record, 0), name, strings);
    field<std::uint32_t>(record, 8) = value;
    field<std::uint8_t>(record, 0x0C) = flag;
    return record;
}

void* copy_named_value_record_00bdcdf0(void* record, const void* source, NativeStringStorage& strings) {
    construct_string_copy(string_at(record, 0), string_at(source, 0), strings);
    field<std::uint32_t>(record, 8) = field<std::uint32_t>(source, 8);
    field<std::uint8_t>(record, 0x0C) = field<std::uint8_t>(source, 0x0C);
    return record;
}

void* copy_string_int_pair(void* record, const void* source, NativeStringStorage& strings) {
    construct_string_copy(string_at(record, 0), string_at(source, 0), strings);
    field<std::uint32_t>(record, 8) = field<std::uint32_t>(source, 8);
    return record;
}

void* construct_string_pair_00bdcbb0(void* pair, const NativeString& first, const NativeString& second,
                                     NativeStringStorage& strings) {
    construct_string_copy(string_at(pair, 0), first, strings);
    construct_string_copy(string_at(pair, 8), second, strings);
    return pair;
}

void* copy_string_pair_00bdcea0(void* pair, const void* source, NativeStringStorage& strings) {
    construct_string_copy(string_at(pair, 0), string_at(source, 0), strings);
    construct_string_copy(string_at(pair, 8), string_at(source, 8), strings);
    return pair;
}

NativeString& copy_selected_profile_name_00be8ca0(const void* owner, NativeString& out,
                                                  NativeStringStorage& strings) {
    construct_string_copy(out, *field<const NativeString*>(owner, 0x30), strings);
    return out;
}

void* construct_debug_feature_record_00be87d0(void* record) noexcept {
    ::new (&string_at(record, 0)) NativeString();
    std::memset(bytes(record) + 8, 0, 0x80);
    return record;
}

void* copy_debug_feature_record_00be8d90(void* record, const void* source, NativeStringStorage& strings) {
    construct_string_copy(string_at(record, 0), string_at(source, 0), strings);
    std::memcpy(bytes(record) + 8, bytes(source) + 8, 0x20 * 4);
    return record;
}

void append_debug_feature_record_00be8fd0(NativeArrayHeader& array, const void* record,
                                          NativeStringStorage& strings, EngineServiceCalls& calls) {
    if (array.count == array.capacity) {
        std::int32_t capacity = add32(array.capacity, array.capacity);
        if (capacity < 2) capacity = 1;
        calls.reserve_debug_feature_records_00be8df0(array, capacity);
    }
    unsigned char* const place = bytes(array.data) + static_cast<std::uint32_t>(array.count) * 0x88u;
    if (place != nullptr) copy_debug_feature_record_00be8d90(place, record, strings);
    array.count = add32(array.count, 1);
}

void push_back_stream_writer_chunk_00bf0810(NativeArrayHeader& array, const void* chunk,
                                            NativeStringStorage& strings, EngineServiceCalls& calls) {
    if (array.count == array.capacity) {
        std::int32_t capacity = add32(array.capacity, array.capacity);
        if (capacity < 2) capacity = 1;
        calls.reserve_stream_writer_chunks_00bf05d0(array, capacity);
    }
    unsigned char* const place = bytes(array.data) + static_cast<std::uint32_t>(array.count) * 0x0Cu;
    if (place != nullptr) copy_string_int_pair(place, chunk, strings);
    array.count = add32(array.count, 1);
}

void pop_back_stream_writer_chunk_00bf0540(NativeArrayHeader& array, NativeStringStorage& strings) noexcept {
    if (array.count != 0) {
        release_string_at(array.data, static_cast<std::uint32_t>(array.count) * 0x0Cu - 0x0Cu, strings);
        array.count = add32(array.count, -1);
    }
}

void push_back_vfs_pair_00bdec00(NativeArrayHeader& vector, const void* pair, EngineServiceCalls& calls) {
    if (vector.count == vector.capacity) {
        std::int32_t capacity = add32(vector.capacity, vector.capacity);
        if (capacity < 2) capacity = 1;
        calls.reserve_vfs_pair_vector_00bdcd10(vector, capacity);
    }
    unsigned char* const place = bytes(vector.data) + static_cast<std::uint32_t>(vector.count) * 0x10u;
    if (place != nullptr) calls.copy_vfs_string_pair_00bdcc60(place, pair);
    vector.count = add32(vector.count, 1);
}

NativeString* string_vector_grow_to_index_00bf0ac0(NativeStringVectorStorage& vector, std::int32_t index,
                                                   NativeStringStorage& strings) {
    static const char kEmpty_00ce3a0c[1] = {'\0'}; // the image's empty string literal
    while (vector.count_04 <= index) {
        TemporaryString empty(strings);
        empty.value.resize_0041dd40(strings, 0, true);
        if (empty.value.data() != nullptr) std::memcpy(empty.value.data(), kEmpty_00ce3a0c, empty.value.length() + 1u);
        append_native_string_vector_004cdc20(vector, empty.value, strings);
    }
    return reinterpret_cast<NativeString*>(vector.data_00) + index;
}

void push_back_string_vector_array_00bf0bd0(NativeArrayHeader& array, const NativeStringVectorStorage& value,
                                            NativeStringStorage& strings, EngineServiceCalls& calls) {
    if (array.count == array.capacity) {
        std::int32_t capacity = add32(array.capacity, array.capacity);
        if (capacity < 2) capacity = 1;
        calls.reserve_string_table_rows_00b80c70(array, capacity);
    }
    auto* const place = reinterpret_cast<NativeStringVectorStorage*>(
        bytes(array.data) + static_cast<std::uint32_t>(array.count) * 0x0Cu);
    if (place != nullptr) {
        place->data_00 = nullptr;
        place->count_04 = 0;
        place->capacity_08 = 0;
        copy_native_string_vector_00543e50(*place, value, strings);
    }
    array.count = add32(array.count, 1);
}

NativeStringVectorStorage* string_vector_array_grow_to_index_00bf0c60(NativeArrayHeader& array, std::int32_t index,
    NativeStringStorage& strings, EngineServiceCalls& calls) {
    while (array.count <= index) {
        NativeStringVectorStorage empty{nullptr, 0, 0};
        push_back_string_vector_array_00bf0bd0(array, empty, strings, calls);
        resize_native_string_vector_00427110(empty, 0, strings);
        singleton_lifetime_free(empty.data_00);
    }
    return reinterpret_cast<NativeStringVectorStorage*>(bytes(array.data) + static_cast<std::uint32_t>(index) * 0x0Cu);
}

// ---------------------------------------------------------------------------------------------------
// VFS search groups, prefixes and aliases.

NativeIterator& find_string_in_list_00bdbfd0(NativeIterator& result, const NativeString& key,
                                             NativeIterator first, NativeIterator last) {
    auto* node = static_cast<NativeListNode*>(first.node);
    while (node != last.node) {
        if (names_equal(field<std::uint32_t>(node, 8), field<const char*>(node, 0x0C), key.length(), key.data())) break;
        node = node->next;
    }
    result.node = node;
    result.container = first.container;
    return result;
}

NativeIterator& vfs_find_search_group_00bdc230(void* vfs, NativeIterator& result, const NativeString& name,
                                               NativeStringStorage& strings, InvalidParameterSink& sink) {
    TemporaryString key(strings);
    key.value.copy_from_00be0a30_fragment(strings, name);
    lowercase_native_string_header_004bcc00(&key.value);
    auto& groups = field<NativeContainerHeader>(vfs, 0x60);
    auto* node = static_cast<NativeListNode*>(groups.head)->next;
    for (;;) {
        if (node == groups.head) {
            result.node = groups.head;
            result.container = &groups;
            return result;
        }
        if (node == groups.head) sink.invalid_parameter_00bf6713();
        if (names_equal(field<std::uint32_t>(node, 8), field<const char*>(node, 0x0C), key.value.length(),
                        key.value.data())) {
            result.container = &groups;
            result.node = node;
            return result;
        }
        if (node == groups.head) sink.invalid_parameter_00bf6713();
        node = node->next;
    }
}

bool vfs_exists_in_flagged_mounts_00bdd520(void* vfs, const NativeString& name, VfsPathContext& context) {
    TemporaryString normalized(context.strings);
    normalized.value.copy_from_00be0a30_fragment(context.strings, name);
    normalize_native_resource_path_header_00bee690(&normalized.value, context.strings);
    struct Visitor {
        std::uint32_t vtable;
        std::uint8_t found_04;
        std::uint8_t pad[3];
    } visitor{0x00D683A4u, 0, {}}; // BSP_VfsFlaggedExistsVisitor table
    context.calls.visit_matching_mounts_00bdd0a0(vfs, normalized.value, &visitor);
    const bool found = visitor.found_04 != 0;
    visitor.vtable = 0x00D68380u; // the visitor base table, restored before the copy is released
    return found;
}

void vfs_add_alias_00bdf590(void* vfs, const NativeString& alias, const NativeString& target, VfsPathContext& context) {
    NativeString pair[2];
    pair[0].copy_from_00be0a30_fragment(context.strings, alias);
    normalize_native_resource_path_header_00bee690(&pair[0], context.strings);
    pair[1].copy_from_00be0a30_fragment(context.strings, target);
    normalize_native_resource_path_header_00bee690(&pair[1], context.strings);
    push_back_vfs_pair_00bdec00(field<NativeArrayHeader>(vfs, 0x94), pair, context.calls);
    context.calls.destroy_vfs_string_pair_00bdb850(pair);
}

bool vfs_resolve_existing_name_copy_00bdf880(void* vfs, const NativeString& name, VfsPathContext& context) {
    TemporaryString copy(context.strings);
    copy.value.copy_from_00be0a30_fragment(context.strings, name);
    return context.calls.resolve_existing_vfs_name_00bdf4c0(vfs, copy.value);
}

void vfs_clear_aliases_00bdf940(void* vfs, EngineServiceCalls& calls) {
    calls.resize_vfs_pair_vector_00bdec70(field<NativeArrayHeader>(vfs, 0x94), 0);
}

std::uint32_t vfs_unmount_provider_00be06a0(void* vfs, void* provider, EngineServiceCalls& calls) {
    auto& mounts = field<NativeContainerHeader>(vfs, 0x3C);
    NativeIterator it{&mounts, field<void*>(mounts.head, 0)};
    for (;;) {
        if (it.container == nullptr || it.container != &mounts) calls.invalid_parameter_00bf6713();
        if (it.node == mounts.head) return 0;
        if (it.container == nullptr) calls.invalid_parameter_00bf6713();
        if (it.node == container_head(it)) calls.invalid_parameter_00bf6713();
        if (field<void*>(it.node, 0x18) == provider) break;
        calls.advance_vfs_mount_iterator_00bd97e0(it);
    }
    if (it.node == container_head(it)) calls.invalid_parameter_00bf6713();
    release_object(field<void*>(it.node, 0x18));
    NativeIterator erased{};
    calls.erase_vfs_mount_iterator_00be0080(erased, *static_cast<NativeContainerHeader*>(it.container), it);
    return 1;
}

void vfs_unregister_search_group_value_00be1b90(void* vfs, const NativeString& group, const NativeString& value,
                                                std::int32_t kind, VfsPathContext& context) {
    TemporaryString key(context.strings);
    key.value.copy_from_00be0a30_fragment(context.strings, value);
    lowercase_native_string_header_004bcc00(&key.value);
    if (kind == 1) slash_terminate(key.value, context);
    NativeIterator found{};
    vfs_find_search_group_00bdc230(vfs, found, group, context.strings, context.calls);
    void* const values = context.calls.select_search_group_value_list_00bdb2e0(found, kind);
    auto& groups = field<NativeContainerHeader>(vfs, 0x60);
    if (found.container == nullptr || found.container != &groups) context.calls.invalid_parameter_00bf6713();
    if (found.node == groups.head) return;
    void* const values_head = field<void*>(values, 4);
    for (auto* node = static_cast<NativeListNode*>(values_head)->next; node != values_head; node = node->next) {
        if (names_equal(field<std::uint32_t>(node, 8), field<const char*>(node, 0x0C), key.value.length(),
                        key.value.data())) {
            NativeIterator erased{};
            context.calls.erase_render_alias_list_node_004d0990(erased, values, node);
            if (found.container == nullptr) context.calls.invalid_parameter_00bf6713();
            if (found.node == container_head(found)) context.calls.invalid_parameter_00bf6713();
            if (field<std::uint32_t>(found.node, 0x18) == 0) {
                if (found.node == container_head(found)) context.calls.invalid_parameter_00bf6713();
                if (field<std::uint32_t>(found.node, 0x24) == 0) {
                    erase_search_group_node_00be1ac0(groups, erased, found, context.calls);
                }
            }
            break;
        }
        if (node == values_head) context.calls.invalid_parameter_00bf6713();
    }
}

void vfs_unregister_search_prefix_00be2210(void* vfs, const NativeString& group, const NativeString& path,
                                           VfsPathContext& context) {
    TemporaryString prefix(context.strings);
    prefix.value.copy_from_00be0a30_fragment(context.strings, path);
    lowercase_native_string_header_004bcc00(&prefix.value);
    slash_terminate(prefix.value, context);
    vfs_unregister_search_group_value_00be1b90(vfs, group, prefix.value, 1, context);
}

void* vfs_find_mount_by_name_and_path_00bdba50(void* vfs, const NativeString& name, const NativeString& path,
                                               const char* trim_set_00ce7898, VfsPathContext& context) {
    TemporaryString canonical(context.strings);
    context.calls.canonicalize_path_00bee390(canonical.value, path);
    trim_native_string_right_00584110(&canonical.value, trim_set_00ce7898, context.strings);
    auto& mounts = field<NativeContainerHeader>(vfs, 0x3C);
    NativeIterator it{&mounts, field<void*>(mounts.head, 0)};
    for (;;) {
        if (it.node == mounts.head) return nullptr;
        if (it.node == mounts.head) context.calls.invalid_parameter_00bf6713();
        void* const provider = field<void*>(it.node, 0x18);
        if (names_equal(field<std::uint32_t>(provider, 8), field<const char*>(provider, 0x0C), name.length(),
                        name.data())) {
            if (it.node == mounts.head) context.calls.invalid_parameter_00bf6713();
            if (names_equal(field<std::uint32_t>(it.node, 0x10), field<const char*>(it.node, 0x14),
                            canonical.value.length(), canonical.value.data())) {
                if (it.node == mounts.head) context.calls.invalid_parameter_00bf6713();
                return field<void*>(it.node, 0x18);
            }
        }
        context.calls.advance_vfs_mount_iterator_00bd97e0(it);
    }
}

NativeIterator& vfs_find_extension_prefix_pair_00bdc3f0(void* vfs, NativeIterator& result, const NativeString& extension,
                                                        const NativeString& prefix, VfsPathContext& context) {
    // The tree pointer (vfs+54h) is tested for null before each iterator use in the listing; it cannot be
    // null for a real object, so those tests are omitted.
    TemporaryString wanted(context.strings);
    wanted.value.copy_from_00be0a30_fragment(context.strings, prefix);
    TemporaryString key(context.strings);
    key.value.copy_from_00be0a30_fragment(context.strings, extension);
    lowercase_native_string_header_004bcc00(&wanted.value);
    lowercase_native_string_header_004bcc00(&key.value);
    slash_terminate(wanted.value, context);
    auto& tree = field<NativeContainerHeader>(vfs, 0x54);
    void* const hi = context.calls.upper_bound_extension_prefix_00bda2c0(tree, key.value);
    NativeIterator it{&tree, context.calls.lower_bound_indexed_name_00bda260(tree, key.value)};
    if (it.node != hi) {
        while (it.node != hi && it.node != tree.head) {
            if (it.node == tree.head) context.calls.invalid_parameter_00bf6713();
            if (equal_native_string_headers_00435c40(bytes(it.node) + 0x14, &wanted.value)) {
                result.container = &tree;
                result.node = it.node;
                return result;
            }
            context.calls.advance_physical_index_iterator_00bd9860(it);
        }
    }
    result.node = tree.head;
    result.container = &tree;
    return result;
}

void* fileblock_scope_construct_00be0ae0(void* scope, const NativeString& name, std::uint32_t arg, void* vfs_0109ceec,
                                         NativeStringStorage& strings, EngineServiceCalls& calls) {
    store_vtable(scope, kRefCountedBaseVtable);
    field<std::uint32_t>(scope, 4) = 1;
    store_vtable(scope, 0x00D68494u);
    field<std::uint32_t>(scope, 8) = 0;
    field<std::uint32_t>(scope, 0x0C) = 0;
    field<std::uint32_t>(scope, 0x10) = 0;
    construct_string_copy(string_at(scope, 0x14), name, strings);
    calls.prepare_fileblock_identifier_00bdf950(scope);
    calls.enter_fileblock_00be0980(vfs_0109ceec, &string_at(scope, 0x14), arg);
    return scope;
}

void filestore_pending_equal_range_00be5bb0(NativeContainerHeader& tree, NativeIterator (&result)[2], const void* key,
                                            EngineServiceCalls& calls) {
    void* const hi = calls.upper_bound_filestore_pending_00be5630(tree, key);
    void* const lo = calls.lower_bound_filestore_pending_00be5530(tree, key);
    result[0] = {&tree, lo};
    result[1] = {&tree, hi};
}

void* construct_search_group_payload_00be10a0(void* payload, const NativeString& name, NativeStringStorage& strings,
                                              EngineServiceCalls& calls) {
    construct_string_copy(string_at(payload, 0), name, strings);
    field<void*>(payload, 0x0C) = calls.allocate_pooled_string_list_head_004c3020();
    field<std::uint32_t>(payload, 0x10) = 0;
    field<void*>(payload, 0x18) = calls.allocate_pooled_string_list_head_004c3020();
    field<std::uint32_t>(payload, 0x1C) = 0;
    return payload;
}

void* copy_search_group_payload_00be13e0(void* payload, const void* source, NativeStringStorage& strings,
                                         EngineServiceCalls& calls) {
    construct_string_copy(string_at(payload, 0), string_at(source, 0), strings);
    calls.copy_pooled_string_list_004d48a0(bytes(payload) + 8, bytes(source) + 8);
    calls.copy_pooled_string_list_004d48a0(bytes(payload) + 0x14, bytes(source) + 0x14);
    return payload;
}

void construct_search_group_payload_at(void* place, const void* source, NativeStringStorage& strings,
                                       EngineServiceCalls& calls) {
    if (place != nullptr) copy_search_group_payload_00be13e0(place, source, strings, calls);
}

void* buy_search_group_node_00be1a20(void* next, void* prev, const void* payload, NativeStringStorage& strings,
                                     EngineServiceCalls& calls) {
    auto* const node = static_cast<NativeListNode*>(allocate(0x28));
    if (node != nullptr) node->next = static_cast<NativeListNode*>(next);
    if (node != nullptr) node->prev = static_cast<NativeListNode*>(prev);
    if (node != nullptr) copy_search_group_payload_00be13e0(bytes(node) + 8, payload, strings, calls);
    return node;
}

void erase_search_group_node_00be1ac0(NativeContainerHeader& list, NativeIterator& result, NativeIterator where,
                                      EngineServiceCalls& calls) {
    if (where.container == nullptr) calls.invalid_parameter_00bf6713();
    auto* const node = static_cast<NativeListNode*>(where.node);
    if (node == container_head(where)) calls.invalid_parameter_00bf6713();
    NativeListNode* const next = node->next;
    if (node != list.head) {
        node->prev->next = next;
        node->next->prev = node->prev;
        calls.destroy_vfs_request_payload_00be1220(bytes(node) + 8);
        singleton_lifetime_free(node);
        --list.size;
    }
    result.node = next;
    result.container = where.container;
}

void destruct_search_group_list_00be1b70(NativeContainerHeader& list, EngineServiceCalls& calls) {
    calls.clear_vfs_request_list_00be19e0(list);
    singleton_lifetime_free(list.head);
    list.head = nullptr;
}

void push_front_search_group_00be1d80(NativeContainerHeader& list, const void* payload,
                                      NativeStringStorage& strings, EngineServiceCalls& calls) {
    auto* const first = static_cast<NativeListNode*>(list.head)->next;
    auto* const node = static_cast<NativeListNode*>(buy_search_group_node_00be1a20(first, first->prev, payload,
                                                                                   strings, calls));
    calls.list_size_check_00bded80(list, 1);
    first->prev = node;
    node->prev->next = node;
}

PrefixTreeNode* buy_prefix_tree_node_00bdf700(PrefixTreeNode* left, PrefixTreeNode* parent, PrefixTreeNode* right,
                                              const void* pair, std::uint8_t color, NativeStringStorage& strings) {
    auto* const node = static_cast<PrefixTreeNode*>(allocate(sizeof(PrefixTreeNode)));
    if (node != nullptr) {
        node->left = left;
        node->parent = parent;
        node->right = right;
        copy_string_pair_00bdcea0(&node->key_0c, pair, strings);
        node->color_1c = color;
        node->isnil_1d = 0;
    }
    return node;
}

void prefix_tree_link_new_node_00be0370(NativeContainerHeader& tree, NativeIterator& result, bool add_left,
                                        PrefixTreeNode* where, const void* pair, NativeStringStorage& strings) {
    check_tree_size(tree);
    auto* const head = tree_head<PrefixTreeNode>(tree);
    PrefixTreeNode* const node = buy_prefix_tree_node_00bdf700(head, where, head, pair, 0, strings);
    link_and_rebalance(tree, result, add_left, where, node);
}

void prefix_tree_insert_equal_00be0b90(NativeContainerHeader& tree, InsertResult& result, const void* pair,
                                       NativeStringStorage& strings) {
    const NativeString& key = string_at(pair, 0);
    auto* where = tree_head<PrefixTreeNode>(tree);
    auto* node = where->parent;
    bool add_left = true;
    while (!node->isnil_1d) {
        int r;
        if (key.length() == 0) r = -static_cast<int>(node->key_0c.length() != 0);
        else if (node->key_0c.length() == 0) r = 1;
        else r = _stricmp(key.data(), node->key_0c.data());
        add_left = r < 0;
        where = node;
        node = add_left ? node->left : node->right;
    }
    prefix_tree_link_new_node_00be0370(tree, result.it, add_left, where, pair, strings);
    result.inserted = 1;
}

// ---------------------------------------------------------------------------------------------------
// Singletons and platform bases.

void* platform_base_register_00be2960(void* object, void*& published_0109cf04, SingletonLifetimeDomain& domain) {
    store_vtable(object, 0x00D68604u);
    ManagerLock lock(domain);
    published_0109cf04 = object;
    domain.get_manager_00415350()->register_object(published_0109cf04);
    return object;
}

void platform_base_unregister_00be2a00(void* object, void*& published_0109cf04, SingletonLifetimeDomain& domain) {
    store_vtable(object, 0x00D68604u);
    {
        ManagerLock lock(domain);
        domain.get_manager_00415350()->unregister_object(published_0109cf04);
        published_0109cf04 = nullptr;
    }
    store_vtable(object, kSingletonBaseVtable);
}

void* platform_base_scalar_delete_00be2aa0(void* object, std::uint32_t flags, void*& published_0109cf04,
                                           SingletonLifetimeDomain& domain) {
    platform_base_unregister_00be2a00(object, published_0109cf04, domain);
    if ((flags & 1u) != 0) singleton_lifetime_free(object);
    return object;
}

namespace {
// The 00BEA8B0 steps (the entry point is reconstructed in src/native_render_service_base.cpp).
void platform_destruct_base_steps(void* object, void*& published_00e1ae90, SingletonLifetimeDomain& domain) {
    store_vtable(object, 0x00D68BC4u);
    {
        ManagerLock lock(domain);
        domain.get_manager_00415350()->unregister_object(published_00e1ae90);
        published_00e1ae90 = nullptr;
    }
    store_vtable(object, kSingletonBaseVtable);
}
} // namespace

void* platform_deleting_destructor_00bea9a0(void* object, std::uint32_t flags, void*& published_00e1ae90,
                                            SingletonLifetimeDomain& domain) {
    store_vtable(object, 0x00D68BC8u);
    platform_destruct_base_steps(object, published_00e1ae90, domain);
    if ((flags & 1u) != 0) singleton_lifetime_free(object);
    return object;
}

void frame_clock_base_destroy_00bedfa0(void* object, void*& published_01090ab0, SingletonLifetimeDomain& domain) {
    store_vtable(object, 0x00D68D20u);
    {
        ManagerLock lock(domain);
        domain.get_manager_00415350()->unregister_object(published_01090ab0);
        published_01090ab0 = nullptr;
    }
    store_vtable(object, kSingletonBaseVtable);
}

void* frame_clock_variant_construct_00bedf60(void* object, EngineServiceCalls& calls) {
    calls.construct_frame_clock_base_00bede00(object);
    store_vtable(object, 0x00D68D24u);
    field<std::uint32_t>(object, 4) = 0;
    return object;
}

void* allocation_stats_derived_construct_00be28b0(void* object, EngineServiceCalls& calls) {
    calls.construct_allocation_stats_base_00be2750(object);
    store_vtable(object, 0x00D685E4u);
    return object;
}

void platform_run_loop_dispatch_00bea800(void* platform_0109cf04) {
    using Dispatch = void(__thiscall*)(void*);
    vtable_slot<Dispatch>(platform_0109cf04, 0x24)(platform_0109cf04);
}

std::int32_t win32_platform_window_proc_00bec3b0(void* platform_0109cf04, void* window, std::uint32_t message,
                                                 std::uint32_t wparam, std::int32_t lparam) {
    // The listing pushes the platform as an explicit first argument (ECX holds the vtable pointer, not
    // the object) and the slot cleans five stack arguments: a __stdcall entry.
    using Proc = std::int32_t(__stdcall*)(void*, void*, std::uint32_t, std::uint32_t, std::int32_t);
    return vtable_slot<Proc>(platform_0109cf04, 0x28)(platform_0109cf04, window, message, wparam, lparam);
}

void* window_platform_base_construct_00be2ac0(void* object, void*& published_0109cf04, SingletonLifetimeDomain& domain) {
    platform_base_register_00be2960(object, published_0109cf04, domain);
    store_vtable(object, 0x00D68608u);
    field<std::uint32_t>(object, 4) = 0;
    field<std::uint32_t>(object, 8) = 0;
    // FILD/FIDIV read the width and height BEFORE they are initialized below (the allocation's previous
    // contents). Extended-precision quotient, rounded once to float by FSTP; a zero height gives the
    // masked-exception infinity or NaN.
    const double aspect = static_cast<double>(field<std::int32_t>(object, 0x24)) /
                          static_cast<double>(field<std::int32_t>(object, 0x28));
    field<std::uint8_t>(object, 0x0C) = 0;
    field<std::uint8_t>(object, 0x0D) = 0;
    field<std::uint8_t>(object, 0x14) = 0;
    field<std::uint32_t>(object, 0x1C) = 0;
    field<std::uint32_t>(object, 0x20) = 0;
    field<std::uint8_t>(object, 0x2C) = 0;
    field<std::uint32_t>(object, 0x18) = 0x20;
    field<std::int32_t>(object, 0x24) = 0x280;
    field<std::int32_t>(object, 0x28) = 0x1E0;
    field<float>(object, 0x10) = static_cast<float>(aspect);
    return object;
}

void win32_platform_apply_window_mode_00bebfa0(void* platform, bool fullscreen, std::int32_t width,
                                               std::int32_t height, float wide_threshold_00d5bd98) {
    field<std::int32_t>(platform, 0x24) = width;
    field<std::int32_t>(platform, 0x28) = height;
    field<std::uint8_t>(platform, 0x0C) = fullscreen ? 1 : 0;
    const float aspect = fullscreen ? field<float>(platform, 0x3C)
                                    : static_cast<float>(static_cast<double>(width) / static_cast<double>(height));
    field<float>(platform, 0x10) = aspect;
    field<std::uint8_t>(platform, 0x0D) = aspect > wide_threshold_00d5bd98 ? 1 : 0; // FCOMI + JBE: NaN -> 0
    field<std::uint8_t>(platform, 0x14) = (width == 0x500 && height == 0x2D0) ? 1 : 0;
    const HWND window = field<HWND>(platform, 0x30);
    if (!fullscreen) {
        WINDOWINFO info{};
        GetWindowInfo(window, &info); // cbSize is not set by the listing either
        RECT rect{0, 0, width, height};
        AdjustWindowRect(&rect, 0x00C00000, FALSE);
        const int cx = rect.right - rect.left;
        const int cy = rect.bottom - rect.top;
        SetWindowLongA(window, GWL_STYLE, 0x00C00000);
        SetWindowLongA(window, GWL_EXSTYLE, 0x300);
        SetWindowPos(window, HWND_NOTOPMOST, info.rcWindow.left, info.rcWindow.top, cx, cy, 0x20);
        ShowWindow(window, 5);
        RedrawWindow(GetDesktopWindow(), nullptr, nullptr, 0x181);
        SetForegroundWindow(window);
        SetFocus(window);
        return;
    }
    SetWindowLongA(window, GWL_STYLE, static_cast<LONG>(0x80000000u));
    SetWindowLongA(window, GWL_EXSTYLE, 8);
    SetWindowPos(window, HWND_TOPMOST, 0, 0, field<std::int32_t>(platform, 0x24), field<std::int32_t>(platform, 0x28),
                 0x20);
    ShowWindow(window, 5);
    SetForegroundWindow(window);
    SetFocus(window);
}

void win32_platform_destroy_00beca00(void* platform, EngineServiceCalls& calls) {
    store_vtable(platform, 0x00D68CC4u);
    SystemParametersInfoA(0x11, 1, nullptr, 0);
    calls.destroy_text_queue_00bec730(bytes(platform) + 0x174);
    calls.destroy_window_platform_base_00be2b10(platform);
}

void* win32_platform_scalar_delete_00bece30(void* platform, std::uint32_t flags, EngineServiceCalls& calls) {
    win32_platform_destroy_00beca00(platform, calls);
    if ((flags & 1u) != 0) singleton_lifetime_free(platform);
    return platform;
}

namespace {
void* allocate_saturating(std::uint32_t count, std::uint32_t element_size) {
    const std::uint64_t total = static_cast<std::uint64_t>(count) * element_size;
    const std::uint32_t bytes_wanted = (total >> 32) != 0 ? 0xFFFFFFFFu : static_cast<std::uint32_t>(total);
    return allocate(bytes_wanted);
}
} // namespace

void* profiler_construct_00be3820(void* object, const ProfilerSlotColours& colours, EngineServiceCalls& calls) {
    static constexpr std::uint32_t kColours[17] = {
        0xFF00FF00u, 0xFFFF0000u, 0xFFFF3F00u, 0xFF0000FFu, 0xFFFFFF00u, 0xFF40FF00u, 0xFFFFFF00u, 0xFF8080FFu,
        0xFF8080FFu, 0xFFFF00FFu, 0xFF0000FFu, 0xFF0000FFu, 0xFF800000u, 0xFF008080u, 0xFFFFFF00u, 0xFF0000FFu,
        0xFFFFFFFFu};
    store_vtable(object, 0x00D6867Cu);
    const std::uint32_t n = colours.slot_count_00e15118 + 0x32u;
    void* records = allocate_saturating(n, 0x28);
    if (records != nullptr) {
        // `vector constructor iterator' over TMPL_Rec_ZeroInit_0053B9E0: ten DWORDs zeroed per record.
        std::memset(records, 0, static_cast<std::size_t>(n) * 0x28u);
    }
    field<void*>(object, 0x14) = records;
    field<void*>(object, 0x18) = allocate_saturating(n * 0x14u, 4);
    field<void*>(object, 0x1C) = allocate_saturating(n * 0x14u, 4);
    field<void*>(object, 0x20) = allocate_saturating(n, 4);
    field<void*>(object, 0x24) = allocate_saturating(n, 4);
    void* const last = allocate_saturating(n, 0x1C);
    field<std::uint32_t>(object, 4) = colours.word_00d7a2f0;
    field<void*>(object, 0x28) = last;
    field<std::uint32_t>(object, 0x2C) = 0;
    field<std::uint32_t>(object, 8) = colours.word_00d68680;
    field<std::uint32_t>(object, 0x0C) = 0;
    field<std::uint8_t>(object, 0x10) = 0;
    calls.profiler_clear_history_00be3380(object);
    for (std::int32_t i = 0; i < static_cast<std::int32_t>(n); ++i) {
        field<std::uint32_t*>(object, 0x24)[i] = 0xFFFFFF0Fu;
    }
    for (std::size_t k = 0; k < 17; ++k) {
        field<std::uint32_t*>(object, 0x24)[colours.indices[k]] = kColours[k];
    }
    return object;
}

void profiler_destruct_00be37a0(void* object, std::uint32_t& published_0109db50) noexcept {
    store_vtable(object, 0x00D6867Cu);
    for (std::size_t offset = 0x14; offset <= 0x28; offset += 4) singleton_lifetime_free(field<void*>(object, offset));
    reset_singleton_base(object, published_0109db50);
}

void* profiler_scalar_delete_00be3800(void* object, std::uint32_t flags, std::uint32_t& published_0109db50) noexcept {
    profiler_destruct_00be37a0(object, published_0109db50);
    if ((flags & 1u) != 0) singleton_lifetime_free(object);
    return object;
}

void* debug_feature_lock_construct_00be82a0(void* lock, EngineServiceCalls& calls) {
    store_vtable(lock, 0x00D68B68u);
    field<void*>(lock, 4) = calls.create_tracked_section_00bd1860();
    return lock;
}

void* debug_feature_lock_singleton_00be8360(void*& published_0109db74, SingletonLifetimeDomain& domain,
                                            EngineServiceCalls& calls) {
    if (published_0109db74 == nullptr) {
        ManagerLock lock(domain);
        if (published_0109db74 == nullptr) {
            void* const raw = allocate(8);
            published_0109db74 = raw != nullptr ? debug_feature_lock_construct_00be82a0(raw, calls) : nullptr;
            domain.get_manager_00415350()->register_object(published_0109db74);
        }
    }
    return published_0109db74;
}

void debug_feature_lock_destroy_00be8420(void* lock, void*& published_0109db74, EngineServiceCalls& calls) {
    store_vtable(lock, 0x00D68B68u);
    calls.destroy_tracked_section_0041cc80(bytes(lock) + 4);
    published_0109db74 = nullptr;
    store_vtable(lock, kSingletonBaseVtable);
}

void* debug_feature_lock_scalar_delete_00be8450(void* lock, std::uint32_t flags, void*& published_0109db74,
                                                EngineServiceCalls& calls) {
    debug_feature_lock_destroy_00be8420(lock, published_0109db74, calls);
    if ((flags & 1u) != 0) singleton_lifetime_free(lock);
    return lock;
}

namespace {
NativeTrackedSection* feature_section(DebugFeatureContext& context) {
    void* const lock = debug_feature_lock_singleton_00be8360(context.published_0109db74, context.domain, context.calls);
    return field<NativeTrackedSection*>(lock, 4);
}
} // namespace

bool debug_feature_has_00be84a0(const void* owner, const NativeString& name, DebugFeatureContext& context) {
    TrackedLock lock(feature_section(context));
    return find_native_string_vector_005efba0(field<NativeStringVectorStorage>(owner, 4), &name) >= 0;
}

NativeString* debug_feature_name_00be84f0(const void* owner, std::uint32_t index, DebugFeatureContext& context) {
    TrackedLock lock(feature_section(context));
    const std::uint32_t count = field<std::uint32_t>(owner, 8);
    if (count <= index) index = count - 1u;
    return reinterpret_cast<NativeString*>(field<std::uintptr_t>(owner, 4) + index * 8u);
}

std::uint32_t debug_feature_count_00be8540(const void* owner, DebugFeatureContext& context) {
    TrackedLock lock(feature_section(context));
    return field<std::uint32_t>(owner, 8);
}

bool debug_feature_any_enabled_00be8580(const void* owner, DebugFeatureContext& context) {
    TrackedLock lock(feature_section(context));
    return field<std::uint8_t>(owner, 0x24) != 0;
}

void debug_feature_disable_00be8720(void* owner, const NativeString& name, DebugFeatureContext& context) {
    TrackedLock lock(feature_section(context));
    context.calls.set_debug_feature_enabled_00be8680(owner, name, false);
}

// ---------------------------------------------------------------------------------------------------
// Structured resource reader and nodes.

namespace {
void* node_reader(void* const* handle) noexcept { return field<void*>(*handle, 8); }
std::uint32_t* node_remaining(void* const* handle) noexcept { return &field<std::uint32_t>(*handle, 0x20); }
} // namespace

void structured_node_read_u32_discarded_00be99e0(void** handle, EngineServiceCalls& calls) {
    calls.read_dword_slot34_and_debit_00bf0280(node_reader(handle), node_remaining(handle));
}

void structured_node_skip_u32_00be9a10(void** handle, EngineServiceCalls& calls) {
    calls.read_dword_and_debit_00bf02a0(node_reader(handle), node_remaining(handle));
}

void** structured_node_read_float_00be9a60(void** handle, float* out, EngineServiceCalls& calls) {
    *out = calls.read_float_and_account_00bf02c0(node_reader(handle), node_remaining(handle));
    return handle;
}

void** structured_node_read_u32_slot34_00be9a80(void** handle, std::uint32_t* out, EngineServiceCalls& calls) {
    *out = calls.read_dword_slot34_and_debit_00bf0280(node_reader(handle), node_remaining(handle));
    return handle;
}

void** structured_node_read_u32_00be9aa0(void** handle, std::uint32_t* out, EngineServiceCalls& calls) {
    *out = calls.read_dword_and_debit_00bf02a0(node_reader(handle), node_remaining(handle));
    return handle;
}

std::uint32_t structured_node_remaining_00be9b20(void* const* handle) noexcept {
    return field<std::uint32_t>(node_reader(handle), 0x64);
}

void** structured_node_read_string_into_00bea030(void** handle, NativeString& out, NativeStringStorage& strings,
                                                 EngineServiceCalls& calls) {
    TemporaryString read(strings);
    auto* const source = static_cast<NativeString*>(calls.read_structured_string_00bea010(handle, read.value));
    out.copy_from_00be0a30_fragment(strings, *source);
    return handle;
}

void structured_reader_destroy_name_stack_00be9f00(NativeString* names, NativeStringStorage& strings) noexcept {
    for (std::size_t i = 10; i-- > 0;) destroy_native_string_header_0041dd20(&names[i], strings);
}

void* structured_reader_construct_00bea0d0(void* reader, void* stream, EngineServiceCalls& calls) {
    calls.stream_reader_construct_00bf09a0(reader);
    for (std::size_t i = 0; i < 10; ++i) ::new (&string_at(reader, 0x10 + i * 8)) NativeString();
    field<std::uint32_t>(reader, 0x60) = 0;
    field<std::uint32_t>(reader, 0x64) = 0xFFFFFFFFu;
    field<std::uint32_t>(reader, 0x68) = 0;
    field<std::uint32_t>(reader, 0x6C) = 0;
    calls.stream_reader_assign_stream_00bf0430(reader, stream);
    return reader;
}

namespace {
void structured_node_header(void* node, void* reader, const NativeString& name, NativeStringStorage& strings) {
    store_vtable(node, kRefCountedBaseVtable);
    field<std::uint32_t>(node, 4) = 1;
    store_vtable(node, 0x00D68BB4u);
    field<void*>(node, 8) = reader;
    field<std::uint32_t>(node, 0x0C) = 0;
    construct_string_copy(string_at(node, 0x10), name, strings);
}
void push_node_name(void* node, NativeStringStorage& strings) {
    void* const reader = field<void*>(node, 8);
    std::uint32_t& depth = field<std::uint32_t>(reader, 0x60);
    NativeString& slot = string_at(reader, 0x10 + depth * 8u);
    ++depth;
    slot.copy_from_00be0a30_fragment(strings, string_at(node, 0x10));
}
} // namespace

void* structured_node_construct_reading_size_00bea4b0(void* node, void* reader, const NativeString& name,
                                                      NativeStringStorage& strings, EngineServiceCalls& calls) {
    structured_node_header(node, reader, name, strings);
    field<std::uint32_t>(node, 0x18) = 0;
    std::uint32_t local_count; // the size read debits a stack local the listing never initializes or reads
    const std::uint32_t size = calls.read_dword_slot34_and_debit_00bf0280(reader, &local_count);
    field<std::uint32_t>(node, 0x1C) = size;
    field<std::uint32_t>(node, 0x20) = size;
    push_node_name(node, strings);
    return node;
}

void* structured_node_construct_with_size_00bea5a0(void* node, void* reader, const NativeString& name,
                                                   std::uint32_t size, NativeStringStorage& strings) {
    structured_node_header(node, reader, name, strings);
    field<std::uint32_t>(node, 0x1C) = size;
    field<std::uint32_t>(node, 0x20) = size;
    field<std::uint32_t>(node, 0x18) = 0;
    push_node_name(node, strings);
    return node;
}

// ---------------------------------------------------------------------------------------------------
// Text scanner.

namespace {
void consume_scanner_token(void* scanner) noexcept {
    char* const token = reinterpret_cast<char*>(bytes(scanner) + 1);
    char* const saved = reinterpret_cast<char*>(bytes(scanner) + 0x401);
    std::size_t i = 0;
    char c;
    do {
        c = token[i];
        saved[i] = c;
        ++i;
    } while (c != '\0');
    field<std::uint8_t>(scanner, 0x801) = 0;
}
} // namespace

bool text_scanner_accept_keyword_00beed60(void* scanner, const char* keyword, EngineServiceCalls& calls) {
    const char* const token = calls.text_scanner_peek_token_00bee8e0(scanner);
    if (_stricmp(token, keyword) == 0) {
        consume_scanner_token(scanner);
        return true;
    }
    return false;
}

bool text_scanner_expect_keyword_00beef60(void* scanner, const char* keyword, EngineServiceCalls& calls) {
    const char* const token = calls.text_scanner_peek_token_00bee8e0(scanner);
    if (_stricmp(token, keyword) == 0) {
        consume_scanner_token(scanner);
        return true;
    }
    calls.text_scanner_skip_whitespace_00beedb0(scanner);
    return false;
}

bool text_scanner_read_bool_00bef0b0(void* scanner, char* out, EngineServiceCalls& calls) {
    const bool value = calls.text_scanner_peek_bool_00beec40(scanner, out);
    if (*out != '\0') {
        consume_scanner_token(scanner);
        return value;
    }
    calls.text_scanner_skip_whitespace_00beedb0(scanner);
    *out = '\0';
    return false;
}

// ---------------------------------------------------------------------------------------------------
// RAM images and physical streams.

void ram_image_destroy_00befc40(void* image, EngineServiceCalls& calls) {
    store_vtable(image, 0x00D68D88u);
    free_owned_block(&field<void*>(image, 0x18));
    calls.destroy_ref_counted_base_00bd30f0(image);
}

void* ram_image_scalar_delete_00befc70(void* image, std::uint32_t flags, EngineServiceCalls& calls) {
    ram_image_destroy_00befc40(image, calls);
    if ((flags & 1u) != 0) singleton_lifetime_free(image);
    return image;
}

std::uint32_t ram_image_load_tga_00beffa0(const NativeString& path, void* vfs_0109ceec,
                                          const char* empty_0109dbb8, NativeStringStorage& strings,
                                          EngineServiceCalls& calls) {
    const char* const text = path.data() != nullptr ? path.data() : empty_0109dbb8;
    void* stream;
    {
        TemporaryString name(strings);
        name.value.assign_0041e870(strings, text);
        using Open = void*(__thiscall*)(void*, const NativeString*, std::uint32_t);
        stream = vtable_slot<Open>(vfs_0109ceec, 4)(vfs_0109ceec, &name.value, 2);
    }
    const std::uint32_t result = calls.decode_tga_00befdc0(stream);
    release_object(stream);
    return result;
}

void physical_provider_block_destroy_00bf2f30(void* block) noexcept {
    singleton_lifetime_free(field<void*>(block, 0x0C));
    free_owned_block(&field<void*>(block, 4));
    CloseHandle(field<HANDLE>(block, 0));
}

void physical_stream_close_handle_00bf4f00(void* stream) noexcept {
    CloseHandle(field<HANDLE>(stream, 8));
    field<HANDLE>(stream, 8) = INVALID_HANDLE_VALUE;
}

void physical_stream_destroy_00bf4fc0(void* stream, EngineServiceCalls& calls) {
    store_vtable(stream, 0x00D691B0u);
    physical_stream_close_handle_00bf4f00(stream);
    store_vtable(stream, 0x00D5C104u);
    calls.destroy_ref_counted_base_00bd30f0(stream);
}

void* physical_stream_construct_and_open_00bf5520(void* stream, std::uint32_t name, std::uint32_t mode,
                                                  EngineServiceCalls& calls) {
    store_vtable(stream, kRefCountedBaseVtable);
    field<std::uint32_t>(stream, 4) = 1;
    field<std::uint32_t>(stream, 0x10) = 0;
    field<std::uint32_t>(stream, 0x14) = 0;
    field<std::uint32_t>(stream, 0x18) = 0;
    field<std::uint32_t>(stream, 0x1C) = 0;
    store_vtable(stream, 0x00D691B0u);
    field<HANDLE>(stream, 8) = INVALID_HANDLE_VALUE;
    calls.physical_stream_open_00bf52a0(stream, name, mode);
    return stream;
}

// ---------------------------------------------------------------------------------------------------
// PC save storage.

void pc_storage_zero_pair_00beaa80(std::uint32_t* a, std::uint32_t* b) noexcept {
    *a = 0;
    *b = 0;
}

const char* pc_storage_root_path_00beab30(const void* storage, const char* empty_0109db8c) noexcept {
    const char* const root = field<const char*>(storage, 0x534);
    return root != nullptr ? root : empty_0109db8c;
}

// Both path builders use an unbounded _sprintf into a 100h-byte stack buffer; the host bounds the write
// (truncating) where the image would overrun its frame.
void pc_storage_delete_slot_file_00beae60(void* storage, const NativeString& slot, const char* file_name,
                                          const char* empty_0109db8c) {
    char path[0x100];
    const char* const slot_text = slot.data() != nullptr ? slot.data() : empty_0109db8c;
    std::snprintf(path, sizeof path, "%s\\%s\\%s", pc_storage_root_path_00beab30(storage, empty_0109db8c), slot_text, file_name);
    DeleteFileA(path);
    const HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (file != INVALID_HANDLE_VALUE) {
        CloseHandle(file);
        field<std::uint8_t>(storage, 0x20) = 1;
    }
}

void pc_storage_file_write_time_00beb0c0(const void* storage, std::uint32_t* out, const NativeString& slot,
                                         const char* file, const char* empty_0109db8c) {
    char path[0x100];
    const char* const slot_text = slot.data() != nullptr ? slot.data() : empty_0109db8c;
    std::snprintf(path, sizeof path, "%s\\%s\\%s", pc_storage_root_path_00beab30(storage, empty_0109db8c), slot_text, file);
    const HANDLE handle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (handle != INVALID_HANDLE_VALUE) {
        FILETIME creation, access, write;
        const BOOL ok = GetFileTime(handle, &creation, &access, &write);
        CloseHandle(handle);
        if (ok) {
            out[0] = write.dwLowDateTime;
            out[1] = write.dwHighDateTime;
            return;
        }
    }
    out[0] = 0;
    out[1] = 0;
}

std::uint32_t pc_storage_can_create_slot_00beb190(void* storage, std::uint32_t slot, const char* valid_name,
                                                  EngineServiceCalls& calls) {
    using Ready = std::uint32_t(__thiscall*)(void*);
    const std::uint32_t ready = vtable_slot<Ready>(storage, 0x18)(storage);
    if (static_cast<std::uint8_t>(ready) == 0) return ready;
    return calls.pc_storage_file_exists_00beb040(storage, slot, valid_name) ? 0u : 1u;
}

void pc_storage_destroy_00beb220(void* storage, NativeStringStorage& strings, EngineServiceCalls& calls) {
    store_vtable(storage, 0x00D68C10u);
    release_string_at(storage, 0x538, strings);
    release_string_at(storage, 0x530, strings);
    calls.destroy_lua_state_owner_00bd4270(storage);
}

void* pc_storage_scalar_delete_00beb610(void* storage, std::uint32_t flags, NativeStringStorage& strings,
                                        EngineServiceCalls& calls) {
    pc_storage_destroy_00beb220(storage, strings, calls);
    if ((flags & 1u) != 0) singleton_lifetime_free(storage);
    return storage;
}

namespace {
// The shared FindFirstFileA scan of 00BEB810 / 00BEBD90: each name becomes a pooled string passed to the
// storage's vtable +20h; the string of the last iteration is released after the loop.
template<class OnName> void scan_storage_pattern(void* storage, const char* empty_0109db8c, NativeStringStorage& strings,
                                                 OnName on_name) {
    const char* pattern = field<const char*>(storage, 0x53C);
    if (pattern == nullptr) pattern = empty_0109db8c;
    WIN32_FIND_DATAA found;
    const HANDLE find = FindFirstFileA(pattern, &found);
    if (find == INVALID_HANDLE_VALUE) return;
    for (;;) {
        TemporaryString name(strings);
        name.value.resize_0041dd40(strings, static_cast<std::uint32_t>(std::strlen(found.cFileName)), true);
        if (name.value.data() != nullptr) std::memcpy(name.value.data(), found.cFileName, name.value.length() + 1u);
        using Accept = bool(__thiscall*)(void*, NativeString*);
        const bool accepted = vtable_slot<Accept>(storage, 0x20)(storage, &name.value);
        on_name(accepted, name.value);
        if (!FindNextFileA(find, &found)) break;
    }
    // The listing never calls FindClose on this handle.
}
} // namespace

std::uint32_t pc_storage_has_free_space_00beb810(void* storage, const char* empty_0109db8c,
                                                 NativeStringStorage& strings) {
    scan_storage_pattern(storage, empty_0109db8c, strings, [](bool, NativeString&) {});
    const char* root = field<const char*>(storage, 0x534);
    if (root == nullptr) root = empty_0109db8c;
    ULARGE_INTEGER available, total, free_bytes;
    if (!GetDiskFreeSpaceExA(root, &available, &total, &free_bytes)) return 0;
    if (available.HighPart == 0 && available.LowPart < 0x100000u) return 0;
    return 1;
}

void pc_storage_enumerate_files_00bebd90(void* storage, void* vector, const char* empty_0109db8c,
                                         NativeStringStorage& strings, EngineServiceCalls& calls) {
    scan_storage_pattern(storage, empty_0109db8c, strings, [&](bool accepted, NativeString& name) {
        if (accepted) calls.append_checked_string_vector_00450540(vector, name);
    });
}

// ---------------------------------------------------------------------------------------------------
// Error reporter map.

ErrorContextNode* error_context_max_00bf1400(ErrorContextNode* node) noexcept { return tree_max(node); }
ErrorContextNode* error_context_min_00bf1420(ErrorContextNode* node) noexcept { return tree_min(node); }

void error_context_rotate_left_00bf15e0(NativeContainerHeader& map, ErrorContextNode* where) noexcept {
    rotate_left(map, where);
}

void error_context_rotate_right_00bf1630(NativeContainerHeader& map, ErrorContextNode* where) noexcept {
    rotate_right(map, where);
}

void error_context_advance_00bf1450(NativeIterator& it, InvalidParameterSink& sink) {
    if (it.container == nullptr) sink.invalid_parameter_00bf6713();
    auto* const node = static_cast<ErrorContextNode*>(it.node);
    if (node->isnil_1d) {
        sink.invalid_parameter_00bf6713();
        return;
    }
    ErrorContextNode* const right = node->right;
    if (right->isnil_1d) {
        ErrorContextNode* p = node->parent;
        while (!p->isnil_1d && it.node == p->right) {
            it.node = p;
            p = p->parent;
        }
        it.node = p;
        return;
    }
    it.node = tree_min(right);
}

void error_context_retreat_00bf16b0(NativeIterator& it, InvalidParameterSink& sink) {
    if (it.container == nullptr) sink.invalid_parameter_00bf6713();
    auto* const node = static_cast<ErrorContextNode*>(it.node);
    if (node->isnil_1d) {
        it.node = node->right;
        if (node->right->isnil_1d) sink.invalid_parameter_00bf6713();
        return;
    }
    if (!node->left->isnil_1d) {
        it.node = tree_max(node->left);
        return;
    }
    ErrorContextNode* p = node->parent;
    while (!p->isnil_1d && it.node == p->left) {
        it.node = p;
        p = p->parent;
    }
    if (static_cast<ErrorContextNode*>(it.node)->isnil_1d) {
        sink.invalid_parameter_00bf6713();
        return;
    }
    it.node = p;
}

NativeIterator& error_context_increment(NativeIterator& it, InvalidParameterSink& sink) {
    error_context_advance_00bf1450(it, sink);
    return it;
}

NativeIterator& error_context_decrement(NativeIterator& it, InvalidParameterSink& sink) {
    error_context_retreat_00bf16b0(it, sink);
    return it;
}

ErrorContextNode* error_context_allocate_head_00bf18e0() {
    auto* const node = static_cast<ErrorContextNode*>(allocate(sizeof(ErrorContextNode)));
    if (node != nullptr) node->left = nullptr;
    if (node != nullptr) node->parent = nullptr;
    if (node != nullptr) node->right = nullptr;
    node->color_1c = 1;
    node->isnil_1d = 0;
    return node;
}

NativeContainerHeader& error_context_construct_00bf1a50(NativeContainerHeader& map) {
    link_tree_head(map, error_context_allocate_head_00bf18e0(), 0x1D);
    return map;
}

void error_context_find_00bf19e0(NativeContainerHeader& map, NativeIterator& result, const std::uint32_t* thread) {
    auto* const head = tree_head<ErrorContextNode>(map);
    ErrorContextNode* where = head;
    for (ErrorContextNode* node = head->parent; !node->isnil_1d;) {
        if (node->thread_0c < *thread) node = node->right;
        else {
            where = node;
            node = node->left;
        }
    }
    result.container = &map;
    result.node = (where == head || *thread < where->thread_0c) ? head : where;
}

ErrorContextNode* error_context_construct_node_00bf2120(ErrorContextNode* node, ErrorContextNode* left,
    ErrorContextNode* parent, ErrorContextNode* right, const ErrorContextValue& value, std::uint8_t color,
    NativeStringStorage& strings) {
    node->left = left;
    node->parent = parent;
    node->right = right;
    node->thread_0c = value.thread;
    node->contexts_10 = NativeStringVectorStorage{nullptr, 0, 0};
    copy_native_string_vector_00543e50(node->contexts_10, value.contexts, strings);
    node->color_1c = color;
    node->isnil_1d = 0;
    return node;
}

ErrorContextNode* error_context_buy_node_00bf21a0(ErrorContextNode* left, ErrorContextNode* parent,
    ErrorContextNode* right, const ErrorContextValue& value, std::uint8_t color, NativeStringStorage& strings) {
    auto* const node = static_cast<ErrorContextNode*>(allocate(sizeof(ErrorContextNode)));
    if (node != nullptr) error_context_construct_node_00bf2120(node, left, parent, right, value, color, strings);
    return node;
}

void* error_context_node_scalar_delete_00bf2230(void* node, std::uint32_t flags, NativeStringStorage& strings) {
    auto& contexts = field<NativeStringVectorStorage>(node, 0x10);
    resize_native_string_vector_00427110(contexts, 0, strings);
    singleton_lifetime_free(contexts.data_00);
    if ((flags & 1u) != 0) singleton_lifetime_free(node);
    return node;
}

void error_context_value_destroy(ErrorContextValue& value, NativeStringStorage& strings) {
    resize_native_string_vector_00427110(value.contexts, 0, strings);
    singleton_lifetime_free(value.contexts.data_00);
}

void error_context_insert_at_00bf2270(NativeContainerHeader& map, NativeIterator& result, bool add_left,
                                      ErrorContextNode* where, const ErrorContextValue& value,
                                      NativeStringStorage& strings) {
    check_tree_size(map);
    auto* const head = tree_head<ErrorContextNode>(map);
    ErrorContextNode* const node = error_context_buy_node_00bf21a0(head, where, head, value, 0, strings);
    link_and_rebalance(map, result, add_left, where, node);
}

void error_context_insert_unique_00bf2480(NativeContainerHeader& map, InsertResult& result,
                                          const ErrorContextValue& value, NativeStringStorage& strings,
                                          InvalidParameterSink& sink) {
    ErrorContextNode* where = tree_head<ErrorContextNode>(map);
    bool add_left = true;
    for (ErrorContextNode* node = where->parent; !node->isnil_1d;) {
        where = node;
        add_left = value.thread < node->thread_0c;
        node = add_left ? node->left : node->right;
    }
    NativeIterator it{&map, where};
    if (add_left) {
        if (where == tree_head<ErrorContextNode>(map)->left) {
            error_context_insert_at_00bf2270(map, result.it, true, where, value, strings);
            result.inserted = 1;
            return;
        }
        error_context_retreat_00bf16b0(it, sink);
    }
    if (value.thread <= static_cast<ErrorContextNode*>(it.node)->thread_0c) {
        result.it = it;
        result.inserted = 0;
        return;
    }
    error_context_insert_at_00bf2270(map, result.it, add_left, where, value, strings);
    result.inserted = 1;
}

void error_context_erase_00bf26c0(NativeContainerHeader& map, NativeIterator& result, NativeIterator where,
                                  NativeStringStorage& strings, InvalidParameterSink& sink) {
    auto* const erased = static_cast<ErrorContextNode*>(where.node);
    if (erased->isnil_1d) throw std::out_of_range("invalid map/set<T> iterator");
    error_context_advance_00bf1450(where, sink);
    auto* const head = tree_head<ErrorContextNode>(map);
    ErrorContextNode* pnode = erased;
    ErrorContextNode* fixnode;
    ErrorContextNode* fixparent;
    if (pnode->left->isnil_1d) fixnode = pnode->right;
    else if (pnode->right->isnil_1d) fixnode = pnode->left;
    else {
        pnode = static_cast<ErrorContextNode*>(where.node); // the successor
        fixnode = pnode->right;
    }
    if (pnode == erased) {
        fixparent = erased->parent;
        if (!fixnode->isnil_1d) fixnode->parent = fixparent;
        if (head->parent == erased) head->parent = fixnode;
        else if (fixparent->left == erased) fixparent->left = fixnode;
        else fixparent->right = fixnode;
        if (head->left == erased) head->left = fixnode->isnil_1d ? fixparent : tree_min(fixnode);
        if (head->right == erased) head->right = fixnode->isnil_1d ? fixparent : tree_max(fixnode);
    } else {
        erased->left->parent = pnode;
        pnode->left = erased->left;
        if (pnode == erased->right) fixparent = pnode;
        else {
            fixparent = pnode->parent;
            if (!fixnode->isnil_1d) fixnode->parent = fixparent;
            fixparent->left = fixnode;
            pnode->right = erased->right;
            erased->right->parent = pnode;
        }
        if (head->parent == erased) head->parent = pnode;
        else if (erased->parent->left == erased) erased->parent->left = pnode;
        else erased->parent->right = pnode;
        pnode->parent = erased->parent;
        const std::uint8_t color = pnode->color_1c;
        pnode->color_1c = erased->color_1c;
        erased->color_1c = color;
    }
    if (erased->color_1c == 1) {
        for (; fixnode != head->parent && fixnode->color_1c == 1; fixparent = fixnode->parent) {
            if (fixnode == fixparent->left) {
                ErrorContextNode* sibling = fixparent->right;
                if (sibling->color_1c == 0) {
                    sibling->color_1c = 1;
                    fixparent->color_1c = 0;
                    rotate_left(map, fixparent);
                    sibling = fixparent->right;
                }
                if (sibling->isnil_1d) fixnode = fixparent;
                else if (sibling->left->color_1c == 1 && sibling->right->color_1c == 1) {
                    sibling->color_1c = 0;
                    fixnode = fixparent;
                } else {
                    if (sibling->right->color_1c == 1) {
                        sibling->left->color_1c = 1;
                        sibling->color_1c = 0;
                        rotate_right(map, sibling);
                        sibling = fixparent->right;
                    }
                    sibling->color_1c = fixparent->color_1c;
                    fixparent->color_1c = 1;
                    sibling->right->color_1c = 1;
                    rotate_left(map, fixparent);
                    break;
                }
            } else {
                ErrorContextNode* sibling = fixparent->left;
                if (sibling->color_1c == 0) {
                    sibling->color_1c = 1;
                    fixparent->color_1c = 0;
                    rotate_right(map, fixparent);
                    sibling = fixparent->left;
                }
                if (sibling->isnil_1d) fixnode = fixparent;
                else if (sibling->right->color_1c == 1 && sibling->left->color_1c == 1) {
                    sibling->color_1c = 0;
                    fixnode = fixparent;
                } else {
                    if (sibling->left->color_1c == 1) {
                        sibling->right->color_1c = 1;
                        sibling->color_1c = 0;
                        rotate_left(map, sibling);
                        sibling = fixparent->left;
                    }
                    sibling->color_1c = fixparent->color_1c;
                    fixparent->color_1c = 1;
                    sibling->left->color_1c = 1;
                    rotate_right(map, fixparent);
                    break;
                }
            }
        }
        fixnode->color_1c = 1;
    }
    resize_native_string_vector_00427110(erased->contexts_10, 0, strings);
    singleton_lifetime_free(erased->contexts_10.data_00);
    singleton_lifetime_free(erased);
    if (map.size != 0) --map.size;
    result = where;
}

void error_context_erase_subtree_00bf2980(ErrorContextNode* node, NativeStringStorage& strings) {
    while (!node->isnil_1d) {
        error_context_erase_subtree_00bf2980(node->right, strings);
        ErrorContextNode* const left = node->left;
        resize_native_string_vector_00427110(node->contexts_10, 0, strings);
        singleton_lifetime_free(node->contexts_10.data_00);
        singleton_lifetime_free(node);
        node = left;
    }
}

void error_context_clear_00bf29d0(NativeContainerHeader& map, NativeStringStorage& strings) {
    auto* const head = tree_head<ErrorContextNode>(map);
    error_context_erase_subtree_00bf2980(head->parent, strings);
    head->parent = head;
    map.size = 0;
    head->left = head;
    head->right = head;
}

void error_context_erase_range_00bf2a00(NativeContainerHeader& map, NativeIterator& result, NativeIterator first,
                                        NativeIterator last, NativeStringStorage& strings,
                                        InvalidParameterSink& sink) {
    if (first.container == nullptr || first.container != &map) sink.invalid_parameter_00bf6713();
    if (first.node == tree_head<ErrorContextNode>(map)->left) {
        if (last.container == nullptr || last.container != &map) sink.invalid_parameter_00bf6713();
        if (last.node == map.head) {
            error_context_clear_00bf29d0(map, strings);
            result.container = &map;
            result.node = tree_head<ErrorContextNode>(map)->left;
            return;
        }
    }
    for (;;) {
        if (first.container == nullptr || first.container != last.container) sink.invalid_parameter_00bf6713();
        if (first.node == last.node) break;
        const NativeIterator victim = first;
        error_context_advance_00bf1450(first, sink);
        NativeIterator ignored{};
        error_context_erase_00bf26c0(map, ignored, victim, strings, sink);
    }
    result = first;
}

void error_context_destroy(NativeContainerHeader& map, NativeStringStorage& strings, InvalidParameterSink& sink) {
    NativeIterator result{};
    error_context_erase_range_00bf2a00(map, result, {&map, tree_head<ErrorContextNode>(map)->left}, {&map, map.head},
                                       strings, sink);
    singleton_lifetime_free(map.head);
    map.head = nullptr;
    map.size = 0;
}

void* error_reporter_construct_00bf2c40(void* reporter, EngineServiceCalls& calls) {
    store_vtable(reporter, 0x00D68E60u);
    field<std::uint8_t>(reporter, 8) = 1;
    error_context_construct_00bf1a50(field<NativeContainerHeader>(reporter, 0x0C));
    field<void*>(reporter, 4) = calls.create_tracked_section_00bd1860();
    return reporter;
}

void error_reporter_destroy_00bf2b90(void* reporter, ErrorReporterContext& context) {
    store_vtable(reporter, 0x00D68E60u);
    context.calls.destroy_tracked_section_0041cc80(bytes(reporter) + 4);
    error_context_destroy(field<NativeContainerHeader>(reporter, 0x0C), context.strings, context.calls);
    reset_singleton_base(reporter, context.published_0109dbdc);
}

void* error_reporter_scalar_delete_00bf2cc0(void* reporter, std::uint32_t flags, ErrorReporterContext& context) {
    error_reporter_destroy_00bf2b90(reporter, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(reporter);
    return reporter;
}

void error_reporter_push_context_00bf2560(void* reporter, const NativeString& text, ErrorReporterContext& context) {
    TrackedLock lock(field<NativeTrackedSection*>(reporter, 4));
    const std::uint32_t thread = GetCurrentThreadId();
    // The listing builds an empty vector, copies it into the {thread, vector} value, inserts, then
    // destroys the inserted value copy, the intermediate and the empty vector in that order.
    NativeStringVectorStorage empty{nullptr, 0, 0};
    ErrorContextValue staged{thread, {nullptr, 0, 0}};
    copy_native_string_vector_00543e50(staged.contexts, empty, context.strings);
    ErrorContextValue value{staged.thread, {nullptr, 0, 0}};
    copy_native_string_vector_00543e50(value.contexts, staged.contexts, context.strings);
    auto& map = field<NativeContainerHeader>(reporter, 0x0C);
    InsertResult inserted{};
    error_context_insert_unique_00bf2480(map, inserted, value, context.strings, context.calls);
    error_context_value_destroy(value, context.strings);
    error_context_value_destroy(staged, context.strings);
    resize_native_string_vector_00427110(empty, 0, context.strings);
    singleton_lifetime_free(empty.data_00);
    if (inserted.it.container == nullptr) context.calls.invalid_parameter_00bf6713();
    if (inserted.it.node == container_head(inserted.it)) context.calls.invalid_parameter_00bf6713();
    append_native_string_vector_004cdc20(static_cast<ErrorContextNode*>(inserted.it.node)->contexts_10, text,
                                         context.strings);
}

void error_reporter_pop_context_00bf1a80(void* reporter, ErrorReporterContext& context) {
    TrackedLock lock(field<NativeTrackedSection*>(reporter, 4));
    const std::uint32_t thread = GetCurrentThreadId();
    NativeIterator found{};
    error_context_find_00bf19e0(field<NativeContainerHeader>(reporter, 0x0C), found, &thread);
    if (found.container == nullptr) context.calls.invalid_parameter_00bf6713();
    if (found.node == container_head(found)) context.calls.invalid_parameter_00bf6713();
    auto& contexts = static_cast<ErrorContextNode*>(found.node)->contexts_10;
    if (contexts.count_04 != 0) {
        destroy_native_string_header_0041dd20(&contexts.data_00[contexts.count_04 - 1], context.strings);
        contexts.count_04 = add32(contexts.count_04, -1);
    }
}

void error_reporter_fail_00bf1f50(void* reporter, const NativeString& text, ErrorReporterContext& context) {
    TemporaryString message(context.strings);
    context.calls.compose_error_message_00bf1b50(reporter, message.value, text);
    if (field<std::uint8_t>(reporter, 8) != 0) {
        const char* const shown = message.value.data() != nullptr ? message.value.data() : context.empty_0109dbe0;
        MessageBoxA(nullptr, shown, "Failure", 0x10);
    }
}

void error_reporter_fail_unknown_00bf1fb0(void* reporter, ErrorReporterContext& context) {
    static const char kText[] = "Unknown assertion failed";
    TemporaryString text(context.strings);
    text.value.resize_0041dd40(context.strings, 0x18, true);
    if (text.value.data() != nullptr) std::memcpy(text.value.data(), kText, text.value.length() + 1u);
    error_reporter_fail_00bf1f50(reporter, text.value, context);
}

} // namespace bsp::engine_services_cc13
