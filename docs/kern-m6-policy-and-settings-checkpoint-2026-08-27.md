# Kern M6 policy and settings checkpoint

Date: 2026-08-27

## Agreed initial policy

- Anti-exfil signing is experimental, persistent, and disabled by default.
- The initial physical integration is testnet-only.
- The device follows the existing SeedSigner ceremony: review and explicitly
  approve stage 1 before creating nonce openings, then review/confirm and
  explicitly approve stage 3 before creating protected signatures.
- Every locally controlled eligible signing slot must be protected atomically.
  A request with a missing, extra, duplicate, reordered, unsupported, or
  already-signed controlled slot produces no opening and no signature.
- Foreign inputs remain governed independently by Kern's existing Partial
  signing policy. Complete local slot coverage does not require Kern to own
  every transaction input.
- Failures do not trigger automatic retries or session substitution. Restarting
  requires a new ceremony.
- Stage and network must match exactly and protected requests never fall back to
  ordinary PSBT signing.
- Stage-2 and stage-4 responses use animated `x-btc-anti-exfil` URs.
- Heap and task-stack measurements remain enabled throughout the experimental
  integration and must not contain seed or transaction data.

## Settings implementation

Wallet Settings now contains an **Anti-exfil signing** toggle immediately below
**Expected-owned signing**. The existing vertically scrollable settings
container is unchanged; no rows or touch targets were reduced. The help dialog
identifies the feature as experimental and testnet-only and explains the two QR
exchanges.

The NVS-backed `ae_sign` value defaults to false. A simulator smoke test proves:

- missing state reads as disabled;
- enabled state is immediately visible and survives settings close/reopen;
- disabled state is immediately visible; and
- settings reset restores the default-off state.

The toggle is intentionally inert at this checkpoint: it does not yet authorize
scanner dispatch or signing.

## Physical result

The signed ESP-IDF 6.0.2 `wave_7b` firmware was flashed to the physical Kern on
COM6. The user completed all checks:

1. Seed load and Wallet Settings navigation were smooth.
2. Scrolling below Expected-owned signing was smooth.
3. Anti-exfil signing appeared in the correct location and defaulted off.
4. The help dialog fit and was readable.
5. The enabled visual state was circle-right with a blue background.
6. Enabled state survived leaving and reopening settings.
7. Enabled state survived a device reboot and seed reload.
8. Disabled state survived leaving and reopening settings.
9. The device was left with anti-exfil signing disabled.

No clipping, awkward scrolling, overlap, or difficult touch targets were
observed.

## Verification

- `kern_sim_settings_smoke`: passed.
- Existing `storage_smoke`: passed.
- `kern_simulator` for `wave_7b`: built and linked successfully.
- Signed `wave_7b` firmware: built and flashed successfully.
- `kern.bin`: 1,904,640 bytes (`0x1d1000`), leaving 70% of the smallest
  application partition free.

The next slice may use this setting only at a dedicated approval-gated workflow
boundary. Merely recognizing or retaining an anti-exfil QR remains insufficient
authority to call the signer.
