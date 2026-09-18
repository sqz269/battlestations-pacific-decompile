#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
struct NativeStringPoolStorage;
struct NativeGameClassCleanupContext;
struct NativeGameClassCleanupProgress;
struct NativeGameClassCleanupCalls {
    virtual ~NativeGameClassCleanupCalls()=default;
    virtual void* class_allocate_00bf681b(std::uint32_t);
    virtual void class_free_00bf65ac(void*);
    virtual void class_invalid_parameter_00bf6713();
    virtual void class_memmove_00bf67a7(void* destination,std::uint32_t bytes,const void* source);
    virtual void* class_manager_00415350(NativeStringRawPoolContext&);
    virtual void class_register_00bd0c30(void* manager,void* object);
    virtual void class_enter_section(void*);
    virtual void class_leave_section(void*);
    virtual NativeStringPoolStorage* class_pool_00419cc0(NativeStringRawPoolContext&);
    virtual void class_return_00bd1510(NativeStringPoolStorage*,void*,std::uint32_t,NativeStringRawPoolContext&);
    virtual void* class_registry_004a9890(NativeGameClassCleanupContext&,NativeGameClassCleanupProgress&);
    virtual void class_increment_004a73a0(void* iterator,NativeGameClassCleanupProgress&);
    virtual void class_subtree_004a8ac0(void* tree,void* node,NativeGameClassCleanupContext&,NativeGameClassCleanupProgress&);
    virtual void class_clear_words_004a8fd0(void*,NativeGameClassCleanupProgress&);
    virtual void class_clear_float_004a8f10(void*,std::uint32_t fill_bits,NativeGameClassCleanupProgress&);
    virtual void class_clear_dwords_00492210(void*);
    virtual void virtual_scalar(void* captured,std::uint32_t slot,std::uint32_t flags)=0;
};
struct NativeGameClassCleanupContext {
    NativeStringRawPoolContext& strings;
    void* volatile& registry_00e187bc;
    // Stable borrowed60h block: six actual10h checked vector headers at
    // E1875C/6C/7C/8C/9C/AC. The global clear retains every backing/capacity.
    void* actual_vectors_00e1875c;
    NativeGameClassCleanupCalls& calls;
};
struct NativeGameClassCleanupProgress {
    std::uint32_t native_site{};
    void* registry{};
    void* tree{};
    void* cursor{};
    void* captured_section{};
    std::uint32_t iterator[2]{};
    std::uint32_t output[2]{};
};
struct NativeGameClassCleanupOperation final:NativeGameClassCleanupProgress {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    NativeGameClassCleanupContext* context{};
    NativeGameClassCleanupOperation()=default;
    ~NativeGameClassCleanupOperation();
    NativeGameClassCleanupOperation(const NativeGameClassCleanupOperation&)=delete;
    NativeGameClassCleanupOperation& operator=(const NativeGameClassCleanupOperation&)=delete;
    // Caller resolves partial ownership first. No free, rollback or retry.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Actual1Ch nodes: links0/4/8,owned keyC/10,mapped payload14,color18,nil19.
// Allocation55B; iterator99B; subtree82B. Subtree owns key/node ONLY.
void* allocate_native_game_class_tree_node_004a7800(NativeGameClassCleanupCalls&,NativeGameClassCleanupProgress&);
void increment_native_game_class_iterator_004a73a0(void*,NativeGameClassCleanupCalls&,NativeGameClassCleanupProgress&);
void destroy_native_game_class_subtree_004a8ac0(void*,void*,NativeGameClassCleanupContext&,NativeGameClassCleanupProgress&);
// Current full-range branch of201B4A9240,consumed by4A9800. General partial
// erase is not provided. Reject a non-current range before touching its graph.
void* clear_native_game_class_full_range_004a9240(void*,void*,void*,void*,void*,void*,NativeGameClassCleanupContext&,NativeGameClassCleanupProgress&);
// Complete17B base,104B constructor,189B getter,111B destructor,30B scalar.
// Native constructor/destructor ECXowner/RET; getter noinputs/EAX/RET;
// scalar ECXowner,stackflags,EAXcapturedowner,RET4. Explicit source contexts.
void destroy_native_game_class_registry_base_004a7140(void*,NativeGameClassCleanupContext&) noexcept;
void* construct_native_game_class_registry_004a9790(void*,NativeGameClassCleanupContext&,NativeGameClassCleanupProgress&);
void* get_native_game_class_registry_004a9890(NativeGameClassCleanupContext&,NativeGameClassCleanupProgress&);
void destroy_native_game_class_registry_004a9800(void*,NativeGameClassCleanupContext&,NativeGameClassCleanupProgress&);
void* delete_native_game_class_registry_004a9870(void*,std::uint32_t,NativeGameClassCleanupContext&,NativeGameClassCleanupProgress&);
// Same captured CE6C68 owner contract for canonical raw singleton drain.
void delete_native_game_class_registered_owner(void*,std::uint32_t,NativeGameClassCleanupContext&);
// Complete90B range erasures: ECXheader,five stackargs,EAXoutput,RET14.
// Both validate nonnull/equal iterator owners; owner need not equal receiver.
// Capture suffix byte count/end,memmove_s iff signed suffix count>0,publish
// end,then output position before owner. No element destruction is added.
void* erase_native_game_class_float_range_00488bf0(void*,void*,void*,void*,void*,void*,NativeGameClassCleanupCalls&,NativeGameClassCleanupProgress&);
void* erase_native_game_class_word_range_004a7a40(void*,void*,void*,void*,void*,void*,NativeGameClassCleanupCalls&,NativeGameClassCleanupProgress&);
// Only the resize-to-zero branches of179B4A8F10/4A8FD0. No general growth
// interface is supplied; these retain backing/opaque/capacity and stale words.
void clear_native_game_class_float_vector_004a8f10(void*,NativeGameClassCleanupCalls&,NativeGameClassCleanupProgress&);
void clear_native_game_class_word_vector_004a8fd0(void*,NativeGameClassCleanupCalls&,NativeGameClassCleanupProgress&);
// Complete268B4A9AC0: delete current nonnull mapped payloads in iterator order,
// clear keys/nodes of the registry from a fresh getter,then six vectors in
// native order. Three positive-zero arguments execute real FLDZ/FSTP32.
// Actual payload scalar bindings remain required. No original FH3/SEH,private
// stack aliases,concurrency,drop-in ABI or application admission is implied.
void clear_native_game_class_globals_004a9ac0(NativeGameClassCleanupContext&,NativeGameClassCleanupOperation&);
} // namespace bsp
