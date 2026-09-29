# Kern upstream-sync owner decisions

Date: 2026-09-29

These decisions resolve independent-review findings F-1 and F-2 for the Kern
upstream-sync candidate.

## F-1: BIP322 scope

Protected signing is currently a Bitcoin transaction-signing feature, not a
general message-signing protocol. While Anti-exfil signing is enabled, Kern
must reject both ordinary transaction PSBTs and BIP322 message-signing
requests. A user who explicitly needs an ordinary BIP322 signature must first
disable Anti-exfil signing. Protected BIP322 message signing may be designed as
a separate, versioned extension later.

This fail-closed policy prevents the setting from silently allowing an
unprotected signature through the BIP322 path.

## F-2: stateless session model

The stateless signer model is intentional and approved. Kern does not retain
stage-1 state and does not compare a stage-3 session ID with a locally stored
session. Durable session continuity, completion, replacement, and retry policy
belong to the coordinator.

Kern still validates the complete stage-3 request cryptographically, including
the exact PSBT digest, network, ordered locally controlled slot set, rederived
signer openings, and host reveal commitments. It repeats transaction review
and requires a fresh approval before creating protected signatures. The
session ID displayed by Kern is coordinator-supplied orientation and is echoed
unchanged; it is not a device-side assertion that Kern retained stage 1.

This model deliberately preserves continuation after reboot or seed reload
without persistent ceremony state on the device.
