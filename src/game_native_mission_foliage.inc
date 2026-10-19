// A scoped genuine D5D220 base for B46A70's unconditional AD5700/AD5740
// getters. AD6950/AD69F0 are complete singleton base operations; AD6D00 is
// their flags1 terminal. This never advertises the D5D270 derived foliage
// profile or admits a landscape, Lua type table, bush/tree draw or update.
// Original ABI: ECX receiver, no public arguments, RET; scalar delete also
// takes stacked byte flags/RET4 and returns the original receiver in EAX.
// Source member/context interfaces add bindings. Original FH3 stack maps,
// hardware-fault cleanup, arbitrary provider changes and binary ABI are unproved.
struct MissionShipFoliageGraph final {
    struct RawGuard { U profile; CRITICAL_SECTION* section; };
    struct BaseCleanup {
        void* receiver;
        bool armed{true};
        ~BaseCleanup() noexcept {
            if (armed) destroy_native_generic_singleton_base_00412430(receiver);
        }
    };
    struct GuardCleanup {
        RawGuard raw;
        bool armed{};
        explicit GuardCleanup(CRITICAL_SECTION* section) : raw{0x00ce37fc, section} {
            if (section) {
                EnterCriticalSection(section);
                depth(section) = depth(section) + 1u;
            }
            armed = true; // original state1, including the null-section arm
        }
        static volatile U& depth(CRITICAL_SECTION* section) noexcept {
            return *reinterpret_cast<volatile U*>(reinterpret_cast<unsigned char*>(section) + 0x18);
        }
        ~GuardCleanup() noexcept {
            if (armed) destroy_native_singleton_guard_00411ee0(&raw);
        }
        void leave() {
            if (raw.section) {
                depth(raw.section) = depth(raw.section) - 1u;
                LeaveCriticalSection(raw.section);
            }
            armed = false;
        }
    };
    static_assert(sizeof(RawGuard) == 8);
    GameHostLog& log;
    void* volatile& manager;
    void* volatile publication_00f8c210{};
    void* allocation{};
    bool entered{}, retired{};

    MissionShipFoliageGraph(GameHostLog& l, void* volatile& m) : log(l), manager(m) {}
    ~MissionShipFoliageGraph() {
        if (entered && !retired) std::terminate();
    }
    void* construct_base_00ad6950(void* receiver) {
        BaseCleanup base{receiver};
        *static_cast<volatile U*>(receiver) = 0x00d5d220;
        void* first_manager = get_native_singleton_manager_00415350(manager);
        auto* section = get<CRITICAL_SECTION*>(first_manager, 0x10);
        GuardCleanup guard(section);
        publication_00f8c210 = receiver;
        void* current_manager = get_native_singleton_manager_00415350(manager);
        void* current = publication_00f8c210;
        register_native_singleton_object_00bd0c30(current_manager, nullptr, current);
        guard.leave();
        base.armed = false;
        return receiver;
    }
    void destroy_base_00ad69f0(void* receiver) {
        *static_cast<volatile U*>(receiver) = 0x00d5d220;
        BaseCleanup base{receiver};
        void* first_manager = get_native_singleton_manager_00415350(manager);
        auto* section = get<CRITICAL_SECTION*>(first_manager, 0x10);
        GuardCleanup guard(section);
        void* current_manager = get_native_singleton_manager_00415350(manager);
        void* current = publication_00f8c210;
        unregister_native_singleton_object_00bcfca0(current_manager, nullptr, current);
        publication_00f8c210 = nullptr;
        guard.leave();
        destroy_native_generic_singleton_base_00412430(receiver);
        base.armed = false;
    }
    void create(U diagnostic_scalar_08_bits, std::uint8_t diagnostic_bush_normal) {
        check(!entered && !publication_00f8c210,
            "mission foliage base construction cannot be replayed");
        entered = true;
        allocation = raw_allocate(0xa8);
        check(allocation != nullptr, "mission foliage base allocation failed");
        (void)construct_base_00ad6950(allocation);
        // These are the existing complete native scalar-store operations,
        // with explicit diagnostic inputs. No uninitialized field is consumed.
        check(cc13_terrain::apply_field_setter(0x00ad56f0, allocation,
            diagnostic_scalar_08_bits), "missing native foliage scalar setter");
        check(cc13_terrain::apply_field_setter(0x00ad5730, allocation,
            diagnostic_bush_normal), "missing native foliage normal setter");
        log.notef("native mission ship foliage base: owner=%p profile=00D5D220 "
            "scalar08_bits=%08x bush_normal=%u (00AD6950/00AD56F0/00AD5730)",
            allocation, diagnostic_scalar_08_bits, diagnostic_bush_normal);
    }
    void retire_before_native_drain() {
        check(entered && !retired && publication_00f8c210 == allocation &&
            get<U>(allocation) == 0x00d5d220,
            "mission foliage base retirement requires its original base owner");
        // AD6D00's flags1 behavior: base destroy first, then free the captured
        // receiver. Explicitly unregister before the ordinary singleton drain;
        // no unknown derived profile is added to that finite dispatcher.
        void* captured = allocation;
        destroy_base_00ad69f0(captured);
        raw_free(captured);
        allocation = nullptr;
        retired = true;
        log.note("native mission ship foliage base: retired via 00AD69F0/00AD6D00 before singleton drain");
    }
};
