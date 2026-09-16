# Diagnostic publication and shared-manager deletion

`GameSingletonHost` now owns one stable, initially null source cell modeling
`0109CF14`. Physical buffer Lock contexts can borrow this cell and the host's
existing `SoundLifetimeAccess` over `01090AA0`. Constructing the host does not
allocate a diagnostic owner or manager; the first diagnostic call remains lazy.

The host installs the cell's address in `NativeSingletonDeletionBindings`
before application callers can register an owner. The diagnostic field is
appended at byte 112, after the clock at 108 and resource support at 104. All
earlier field offsets are preserved; the Win32 source structure is now 116 bytes.

The existing raw `00BD0400` manager drain pops an owner before resolving its
current profile. For `CE752C`, the finite source dispatcher invokes the existing
`004BBCA0` scalar destructor on that popped owner with the current flags and the
same publication cell. The scalar clears the current cell, stamps the captured
owner `CE3818`, frees it for flags bit 0, and returns its original address bits.
It does not unregister itself. Missing diagnostic binding follows the existing
unsupported-profile source error; it cannot silently complete cleanup.

The publication, deletion table, and shared manager remain alive through the
host's shutdown drain. No second manager or diagnostic domain is introduced.

## Evidence and validation scope

The R37 audit and R38 access report establish the original getter, scalar,
profiles and loader-zero cells. This packet adds application-owned source
storage and a finite deletion admission; it reconstructs no new original body.
See `reports/native_diagnostic_host_admission_r38.json` for exact build, fixture
and artifact receipts.

The focused ignored fixture uses a real `GameSingletonHost`, genuine recovered
physical index/vertex constructors, cold and warm null-COM Lock/Unlock paths,
and the real raw manager. It then destroys both physical owners and drains the
diagnostic and resource-support registrations through the host. This tests the
source ownership closure without requiring a D3D device.

Original application shutdown explicitly invokes `007363B0` at step 19. That
wrapper's source reconstruction and application binding are separate work; this
packet's final-manager drain does not establish the earlier shutdown timing.
No native ABI/FH3/SEH, D3D Lock, renderer, full application or gameplay parity is
claimed. The changed host and deletion structures are C++ source interfaces and
require rebuilding their consumers.
