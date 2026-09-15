# Device class constructors

Eight complete ordinary constructor bodies now compose the damageable base at
`0087C640` through its reconstructed source interface. These are borrowed raw
storage entrypoints, with stable actual vtable word identities supplied by the
caller. They allocate no class object, fabricate no virtual methods, and leave
unspecified bytes untouched. Names below describe behavior; they are not recovered
source symbols.

| Entry | Inclusive end | Bytes | Final table identity | Extra writes after common base |
| --- | --- | ---: | --- | --- |
| `00442B90` | `00442BF7` | 104 | `00CE4534` | Common gun fields below |
| `00442C50` | `00442C6B` | 28 | `00CE4560` | DWORD `+80h = 10` |
| `00442CE0` | `00442CFB` | 28 | `00CE459C` | DWORD `+80h = 10` |
| `00442D80` | `00442D91` | 18 | `00CE45E0` | None |
| `00442E20` | `00442E31` | 18 | `00CE4614` | None |
| `00442EC0` | `00442ED1` | 18 | `00CE4648` | None |
| `00442F50` | `00442F61` | 18 | `00CE467C` | None |
| `00442FF0` | `0044300B` | 28 | `00CE46C0` | DWORD `+80h = 11` |

Original ABI for all eight: `ECX` is the actual class, no stacked arguments,
plain `RET`, `EAX` returns the same captured class. The new entrypoints use a
C++ interface and are not original ABI bridges.

## Common construction and failure

`00442B90` first calls the genuine damageable constructor. It then writes zero
to `+70h`, installs its table word at `+0`, and zeros `+74h,+78h,+7Ch,+9Ch,+A0h,
+A4h,+BCh,+C0h,+C4h`, in that order. All other bytes remain as the base left
them. In particular, these bodies neither set the factory's class id at `+6Ch`
nor initialize the whole allocation. Each leaf calls this complete common body,
then performs the writes in the table. Ordered DWORD stores in the source retain
the native schedule.

The common constructor's FH3 handler is `00C5FB41`, with FuncInfo `00D86790`
and a three-entry unwind map at `00D86778`. The entries refer to `00C5FB20`
(`004420D0` on the captured class), `00C5FB28` (`00442B70` on `+74h`), and
`00C5FB33` (`0041F5D0` on `+98h`). However, the complete ordinary body pushes
state `-1` and never arms any of those states, including during the base call.
Consequently the derived source adds no cleanup after a base failure; the real
base owns its own established failure cleanup. The leaf bodies have no FH3 frame.
Native FH3/SEH identity and hardware-fault recovery remain outside this interface.

## Factory relationship

The live common-constructor callers are its seven leaf bodies and factory
`00443090` at `004431C5` and `0044320C`. Those two factory paths inline the rapid
types' final table override. The standalone rapid constructors `00442D80` and
`00442E20` exist as complete bodies but have no current incoming references.
Other factory call sites are `0044324E -> 00442EC0`, `0044328A -> 00442C50`,
`004432C2 -> 00442CE0`, `004432F7 -> 00442F50`, and `00443330 -> 00442FF0`.

The table identities' first slots are real native targets: all begin with
`00BD30E0`; their next slots are the corresponding deleting-destructor entries.
Reader slot `+8` is `007327B0` for the common/rapid/single/depth-charge profiles,
`006E01C0` for bomb, `006E02C0` for multibomb, and `006EB6B0` for catapult.
Storing these identities does not reconstruct the missing destructor/reader
families or establish a working factory.

## Evidence and validation

All 260 original body bytes and the complete assembly schedules were inspected.
The report records per-body disk/live equality and checksums, direct constructor
transfers, the actual factory call sites, and previous names before annotation.
Project: `C:/Users/sqz269/bsp.gpr`; program: `/battlestationspacific.exe`.

The base's focused original/source construction comparison is documented in
`NATIVE_DAMAGEABLE_CLASS_CONSTRUCTION_ORCH4.md`. Derived validation uses the
complete listing, verified call sites, and strict MSVC Win32 build with existing
tests. No new broad test suite is introduced. These results establish ordinary
source coverage and build behavior, not original exception ABI or gameplay parity.
