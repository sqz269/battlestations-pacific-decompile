# CC13 sound pass 1: segment 74 leaves (cc13-sound1)

Worker cc13-gp7a on packet cc13-sound1, 2026-10-12, branch `agent/cc13-sound1`. This was a reconstruction pass over
segment 74, `00A7A440`-`00A9AC40`. The segment holds sound, streaming, FMOD wrappers, shadow targets and input. The
targets were the functions with a ledger name but no reconstruction record and no tag in the index. Nothing was run in
the game, and nothing is wired into a host.

Status: **reconstructed, build-tested** only. That means `./scripts/build.ps1` passed and ctest passed 2 of 2. Nothing
here is ABI-compatible, fixture-tested or game-validated. No test was added.

## Counts

| | count |
|---|---|
| named, no reconstruction record, no tag (index at the start) | 120 |
| **reconstructed and recorded** (`ledger add-function`, `src/sound_cc13.cpp`) | **63** |
| left (below) | 57 |

## Files and model

- `include/bsp/sound_cc13.hpp` and `src/sound_cc13.cpp` are new and registered in `bsp_core` by one line appended to
  `cmake/startup.cmake`. Each declaration carries the address, the original ABI and the listing evidence.
- The routines work on the actual native storage, as this segment's existing files do (`src/sound_sample.cpp`,
  `src/sound_file_callbacks.cpp`, `src/native_input_binding_storage.cpp`):
  - pointer-array headers `{data, count, capacity}`;
  - intrusive reference counts at `+4h`, released through the object's own vtable slot 0;
  - native red-black tree nodes (`left +0h`, `parent +4h`, `right +8h`, colour `+18h`, isnil `+19h`, size `1Ch`);
  - singleton bases.
- The image's operator new (`00BF55BE`, `00BF681B`) and free (`00BF6989`, `00BF65AC`, `00BF9DC8`) map to the project's
  `singleton_lifetime_allocate`/`singleton_lifetime_free` boundary, the same mapping `src/sound_sample.cpp` uses.
- Image globals are never invented. The published singleton words (`00F8BBDC`, `00E18AD4`, `00F8BBF8`) and the empty
  buffer `00F8BC02` are passed in by the caller.
- Every body was read from the PE bytes with capstone (`local/s1_dis.py`). Byte-identical copies were grouped
  (`local/s1_twins.py`) and share one C++ body; each copy has its own record.

## What was reconstructed

- **Pointer arrays.** The minimum or growth policy is read from each body.
  - `00A7A850` reserve with minimum 8 and `00A7A8B0` reserve with minimum 16.
  - `00A7A930` resize.
  - Appends: `00A7B2D0` grows by +4 with a floor of 8, `00A7B310` by +16 with a floor of 16, and `00A7B390` doubles
    with a floor of 1 through `00A7A9E0`.
  - Destroys: `00A7B7A0`, and `00A7B830` (which calls `00A7A980`, the same resize body over `00A7A8B0`; that address
    keeps its fragment record in `src/sound_configuration_shutdown.cpp`).
  - `00A7BA80`, `00A7BB20` and `00A7BF20`: three copies over `00A7B3D0`.
  - Assigns `00A7B7C0` and `00A7BAA0`.
- **Reference slots.**
  - Reset-and-return-self: seven byte-identical copies (`00A7B4B0`, `00A7B4E0`, `00A82880`, `00A8DE20`, `00A8DE50`,
    `00A90AF0`, `00A929F0`).
  - Release: six copies (`00A7B960`, `00A7B990`, `00A7BB40`, `00A7BB70`, `00A92B40`, `00A92FA0`).
  - `00A92B00` assigns **without adding a reference** to the new object.
  - `00A92C00` resets the slot at `owner+2Ch`.
- **Owned pointers.** Deleted through vtable `+4h` (`00A81C20`, `00A81CF0`) or slot 0 (`00A87CB0`), with flag 1.
- **Sound trees.**
  - Min and max: `00A81980` and `00A81960`.
  - Rotations: `00A822E0` left and `00A819C0` right.
  - The black-node allocation `00A88270`. It writes the node fields even for a null result, so a non-null result is a
    stated precondition.
  - The three head initialisers `00A88370`, `00A883D0` and `00A88620`. They differ only in their RET and EAX.
- **Singleton bases.** `00A894B0`, `00A8DD60`, `00A92180` and `00A92230`.
- **FMOD.** The free callback `00A87BD0` (`__stdcall`, RET 8). Its debug print `004254B0` is an empty body.
- **Input.**
  - `00A90180`: any of eight devices answers vtable `+2Ch`.
  - `00A93EC0`: clear the state bytes.
  - `00A96450`: key-character translation.
  - `00A90840`: `fill_n`.
  - Binding assign `00A931A0` (its float copy goes through the x87, which quiets a signalling NaN; the C++ copy keeps
    the bits).
  - Swap-remove `00A934B0`.
  - Vector destroys `00A938E0` and `00A93D80`.
  - The action map's vector clear `00A93D60`.
- **Trivial bodies.**
  - `00A7BE70` (RET 8) and `00A7BE80` (RET 18h).
  - `00A89C50`, `00A8A9B0` and `00A8A9C0`.
  - `00A90EC0` and `00A97220` (false).
  - Accessors: `00A97B80` (byte `+F4h`), `00A92290` (context level) and `00A818D0` (sample resource word).

## Left, with the reason

- **Records that hold native strings.** Their copy constructors, reserves, appends and pops need the pooled-string
  copy and destroy services of the segment's existing files with matching contexts. They were not done in this pass:
  - `00A7D880`, `00A7F110`, `00A7F160`, `00A7FAF0`, `00A7FB50`, `00A7FBD0`, `00A7FEB0`;
  - `00A82FF0`, `00A83040`, `00A83090`;
  - `00A843A0`, `00A84450`, `00A84380`, `00A84D20`, `00A86FB0`, `00A93900`.
- **Destroys over resize routines that exist only as fragments or class methods.** These are `00A7D9B0`, `00A7F2D0`,
  `00A7F0D0`, `00A7FAD0`, `00A7FF30` and `00A81460`. They call `00A7C2C0`, a `SoundClassOwnership` method, and the
  fragments `00A7F240` and `00A7FCC0`. Also left are `00A7C240`, `00A7D990` and `00A7D9A0`.
- **Tree code that frees string payloads, and the checked-iterator wrappers.**
  - `00A81D10`, `00A88400`, `00A88460`, `00A88560`, `00A885A0`, `00A885E0`.
  - `00A82360`, `00A823C0`, `00A82810`, `00A82870`. Their callee is modelled over a host `std::map` in
    `src/sound_sample_cache.cpp`, not over native nodes.
- **String unwinds and the lock destructor.** `00A82E20` and `00A82E40` are string unwinds. `00A89B10` needs the
  critical-section seam `0041CC80`.
- **Shadow targets.** `00A8A820`, `00A8A8C0`, `00A8A980`, `00A8F3B0` (255 lines), `00A8FF30` and `00A8FFF0` (Direct3D
  resources).
- **Larger input and text routines.** `00A90530`, `00A92A20`, `00A93690`, `00A943D0`, `00A96750`, `00A97140`,
  `00A97260`, `00A97B30`, `00A99180`, `00A9A0B0`. The debug dump `00A7EA30` and `00A7FE60` are also left.

## Also in this branch

- `docs/CC13_ROCKET_TASK.md`: the base bot-task slot `+50h` (`007B4140`) answers 1. Its bytes are `MOV EAX,1; RET`,
  as docs/CC13_GP_007A_007F.md finding 2 records.
- The lead's split item (`007A7DF0` in `007A7DE0`, and `007F64EC` in `007F6190`) was already repaired on
  `agent/cc13-gp7a` (`82ea96c59`). I re-checked both heads per cc13-gp85's caution. Both parents start right after
  INT3 padding. Neither has a RET followed by INT3 before its split address, so neither ends at its own RET.
