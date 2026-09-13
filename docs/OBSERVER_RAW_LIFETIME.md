# Observer lifetime in the application's raw manager

Addresses: 00694200, 00694280, 00694EA0, 00BD0400.

The observer lifetime now accepts the existing borrowed manager access used by
the application. It can use the actual 01090AA0 publication instead of requiring
a separate semantic SingletonLifetimeDomain. The access and captured-section
types retain their historical SoundLifetimeAccess names; their manager behavior
is shared without creating a second publication or copying another guard.

At 006942A6 the native getter captures the first manager's section at +10. After
double-checking E198E0 it allocates and constructs the lock owner, publishes it,
looks up the manager again at 00694306 and registers the freshly read owner.
It releases the original captured section, then reloads E198E0. The adapter
preserves that sequence through the existing raw/semantic access boundary.
The observer constructor and deleting destructor now use the paired raw1Ch
malloc-backed critical-section helpers. They do not mix those allocations with
the older convenience new/delete interface.

Native table CF7E70 contains slot zero 00694EA0. The raw manager's existing
finite profile dispatcher now admits that profile with a borrowed observer
lifetime. It passes the actual popped owner and flags1, allowing the real
destructor to release its section, clear E198E0, rewrite CE3818 and free the
captured allocation. GameSingletonHost can bind that context before registration;
the caller must keep it alive through shutdown and borrow this same manager.

Win32 Release and both existing CTests passed. One focused source lifecycle
fixture creates a single raw manager and lock, registers a pair twice, preserves
the shared reference count through the first unregister, removes the pair on the
second, and drains the actual lock through the raw manager profile. Endpoint
arrays and the empty dispatch vector are explicit fixture inputs. This is not
an original-byte differential test or evidence of game observer execution.

The initial fixture expected the foreign-profile deletion callback for an
already-supported canonical edge. Inspection confirmed that the existing code
dispatches CF7E64 directly. Only that fixture expectation was corrected; its
failed source and executable are retained. Exact source, fixture and build
library hashes are recorded in reports/observer_raw_lifetime.json. No permanent
test cases were added.

## Follow-up packets

The actual dispatch static initializer publishes an embedded vector alias;
its owner and destruction order are a separate Q packet. Complete neighbour
node construction and unit endpoint/flag ownership must also be integrated
before candidate admission is enabled. These changes add the real manager
access and deleting path; they do not establish those missing owners or original
FH3, hardware-fault, concurrency, ABI or gameplay compatibility.
