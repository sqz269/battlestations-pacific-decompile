# Raw camera first-viewport admission

Addresses: `00B71A80`, `00B1F850`, `00CC1AD0`, `00CC1AD8`, `00CC1AE3`.
Host composition over the existing reconstructed constructor; no new native body.

The raw-name camera constructor now accepts an optional explicit
`NativeViewportRegistry::Admission` through a new public overload. Both raw
signatures call one file-private body. The semantic `NativeString` overload
retains its separate body and behavior.

The shared body preserves prepared-owner, raw-name-domain and exact D7A24C-cell
checks. For an admitted call, it then validates the token's installed registry
and requires the camera environment's viewport resolver to be that exact
registry. Only after these checks does it move the token into a local guard,
before changing phase or running B6F5A0. Rejected preflight leaves the caller's
token and prepared owner unchanged. Native callbacks see an empty caller token;
the selected path is held in a local Boolean, with no later token-pointer read.

At the existing first viewport allocation expression, the admitted path calls
the already implemented admitted factory. That factory registers its successful
B1F850 owner without allocation before returning. Assignment to actual camera
+180 follows the return. It transfers the existing count1 without adding a
retain. Ordinary raw calls keep the ordinary allocation path. The continuation's
native stores, volatile loads, x87 FLDZ/FLD1 materialization and cleanup remain
literal and shared between raw overloads.

An unused local token cancels its record if node construction, native allocation
or viewport initialization fails. Existing node/raw-allocation cleanup and
caller-owned camera-slot return are unchanged. A successful factory consumes the
token; later camera failure leaves its published viewport and live view record
intact. The actual native EH map cleans +438/node state but does not release
camera+180. The persistent record must outlive the failed camera, and its owner
must be tracked independently of ended camera storage. Quiescent record
forgetting and cleanup of any surviving native owner remain explicit.

The complete live B71A80 listing was reread through the target-verified BSP CLI:
B71A80..B71CDB inclusive,604bytes, ECX camera, stacked name, EAX same, RET4.
B71AA7 calls B6F5A0; B71ABA allocates34h through BF681B and B71ABF removes its
four-byte argument; B71AD1 calls B1F850; B71ADD publishes its returned pointer.
The complete three-state EH map at DFAA20 has actions CC1AD0(next-1),
CC1AD8(next0), CC1AE3(next0). Their listings show node cleanup, saved raw
viewport free, and +438 release respectively. Registration is additional host
bookkeeping; no new call is attributed to the original binary.

Independent design: `docs/NATIVE_CAMERA_VIEWPORT_ADMISSION_BI.md`. Successful
native/source comparison and focused source-side early/late failure verification
are pending against the exact combined source. No repository test is added.
Original FH3/SEH exception identity, replacement ABI and game execution remain
unproved. The installed-registry, serialized-mutation, stable-record and live
native-borrow contracts remain in force. Independent prepared constructions may
nest during callbacks; same-owner construction or registry-mutation reentry is
not supported.
