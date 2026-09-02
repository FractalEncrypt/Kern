# M8-X01 preparation checkpoint

Date: 2026-09-02
Status: pristine native-P2WSH fixture frozen; Part E Kern signing may start
Scope: mixed Kern-then-SeedSigner Testnet3 ceremony

## Authorization

KimiK3's review of Kern `f259fda` accepted M8-C/P02, M8-D/P02, their
shared-fixture comparison, and the session-ID erratum. It authorized M8-X01
under the frozen playbook. The one deliberate broadcast remains separately
gated by Part G.

## Wallet registration

The new isolated Sparrow home is
`run/m8-x01-sparrow-home`. The exact native SegWit 2-of-3 wallet has descriptor
checksum `47y6998m` and account origin `m/48'/1'/0'/2'` for all three keys.

| Fingerprint | Sparrow model | Policy | M8-X01 role |
| --- | --- | --- | --- |
| `0fb882ff` | Kern | Optional | first protected signature |
| `b4899a09` | SeedSigner | Required | second protected signature |
| `2a0726f2` | SeedSigner | Required | reserve; must not sign |

The operator reported that both physical devices accepted the descriptor and
showed their expected registration screens. Screenshots were not captured, so
this is explicitly operator-reported evidence.

## Pristine fixture

The binary PSBT was saved and made read-only before either device acted:

- file: `run/m8-evidence/M8-X01-pristine-unsigned.psbt`;
- size: 1,607 bytes;
- SHA-256:
  `483294e990e74f412a31719b765147ea20c403bf8b2200c4722bb4c7fb0edec0`;
- unsigned txid:
  `6854c031978839983e6cb2bcf064a432f126fbbbdbec7c969e9202e554442496`;
- sole input: `61a05816...079da:2`, 36,369 sats;
- recipient: 11,111 sats to
  `tb1qkugmkmmlammdcu0wvx3zrvkeux6yxvtd7w9zpt2y8yr0en4yzces6ed3ta`;
- change: 25,055 sats to
  `tb1q64uw5vsynvgq3yklvdq6srwkep4pzjwl2qclq5mjqhmsv6twz8sq3287sv`;
- fee: 203 sats;
- input witness-script SHA-256:
  `c00eb6d8d95054018910670367fc9457a4d2ce46260be8cbcf9a3e8b800ea7fe`;
- partial signatures: zero; and
- finalized inputs: zero.

The remaining 11,111-sat P2WSH fixture at vout 0 is untouched.

## Independent inspection

The dependency-free strict checker
`run/m8-evidence/inspect_m8_x01_pristine.py` verified the PSBT maps, sole
outpoint, values, P2WSH-to-witness-script hash binding, sorted 2-of-3 script,
three pubkey/fingerprint/path mappings, Testnet bech32 outputs, fee, and
unsigned/unfinalized state. It exits with `validation=PASS`.

Pinned local records:

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| `M8-X01-pristine-unsigned.psbt` | 1,607 | `483294e990e74f412a31719b765147ea20c403bf8b2200c4722bb4c7fb0edec0` |
| `inspect_m8_x01_pristine.py` | 8,237 | `43a2efd397896e11b6221dd141b9161dd02b362cec6201f3acd986115c935203` |
| `M8-X01-pristine-unsigned-receipt.md` | 3,720 | `713397260fc461ae77c13e850262e81c3677cf684f3962a3b7f6f9aa48a0b283` |

All three files are read-only.

## Part E boundary

Kern may now sign first from this exact PSBT. Before Sparrow creates M1:

1. load `0fb882ff` and the accepted descriptor in Kern;
2. confirm Testnet and enable only Anti-exfil signing;
3. start both Sparrow and Kern monitors; and
4. reopen the read-only PSBT rather than recreating the transaction.

After Sparrow accepts M4, save the one-signature intermediate PSBT and stop.
It must remain incomplete, must contain only Kern's signature, and must not be
broadcast. Part F will start only from that frozen intermediate.
