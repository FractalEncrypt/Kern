# Kern M5 headless response checkpoint

Date: 2026-08-27

This checkpoint connects the retained scanner request to the completed M4
signer and canonical M5 response transport without selecting M6 policy.

## Implemented boundary

`anti_exfil_response_create()` accepts only an already owned stage-1 or stage-3
request. It borrows the exact retained AEXT PSBT bytes, invokes the stateless M4
prepare or complete operation, constructs a PSBT-free stage-2 or stage-4 AEXT
package and canonical CBOR, and creates an owned `x-btc-anti-exfil` cUR encoder.
The input request remains valid until its separate owner destroys it.

The API performs signing and says so explicitly. It is not called by scanner or
UI code. A later approved workflow must apply the anti-exfil setting and obtain
explicit user approval before invoking it.

This layer intentionally does not choose:

- enabled/disabled or network-family policy;
- session continuity, retry, or cancellation behavior;
- complete-slot versus subset coverage;
- fee or transaction review policy; or
- coordinator merge and downgrade behavior.

## Exact conformance tests

The host test uses the pinned realistic four-input, five-slot fixture and proves:

- stage 1 produces the byte-exact canonical stage-2 CBOR;
- stage 3 produces the byte-exact canonical stage-4 CBOR;
- the first full 150-byte-fragment cUR window reconstructs the exact semantic
  response with no PSBT;
- response creation does not consume or invalidate the retained request;
- message 2, undersized fragment configuration, and the wrong seed fail without
  returning a response; and
- destroying either owner clears its caller-visible pointer.

Result: 21 passed, 0 failed. The full host suite also passes.

## Build and resource evidence

- The simulator build includes the response bridge.
- The native ESP-IDF 6.0.2 `wave_7b` signed firmware build succeeds.
- `kern.bin` is 1,904,640 bytes (`0x1d1000`), leaving 70% of the smallest app
  partition free.
- Stage 2 is 819 canonical CBOR bytes and 6 source parts at a 150-byte maximum.
- Stage 4 is 1,139 canonical CBOR bytes and 8 source parts at a 150-byte maximum.
- Host record sizes are 30,296 bytes for a semantic message, 17,456 bytes for
  signer slot scratch, and 30,312 bytes for AEXT validation scratch.
- The bridge wipes and frees the 47,752 bytes of large signer work records before
  allocating the AEXT encoder scratch, so those record sets do not overlap.

The record sizes and lifetime separation are reproducible host evidence, not a
physical peak-heap claim. Physical signer-output heap measurements remain gated
on the first policy-approved UI invocation.

## Milestone boundary

This completes the headless engine-to-transport seam and composes with the
existing QR generator: the live bridge output is byte-identical to the pinned
stage-2/stage-4 data that `measure_anti_exfil_transport --emit` renders as cUR
parts. It does not yet satisfy the interactive M5 exit gate by itself because
Kern cannot present an approval screen and then display the live response.

That final UI connection belongs with the M6 decisions it would enforce. Until
those decisions are explicit, recognized anti-exfil scans remain consumed and
retained for measurement only, never dispatched into signing.
