# DER Signatures

Signing a transaction in Bitcoin means endorsing a serialised representation of the transaction called [sighash](https://github.com/libbitcoin/libbitcoin/wiki/Sighash-&-TX-Signing) with an [ECSDA](https://en.wikipedia.org/wiki/Elliptic_Curve_Digital_Signature_Algorithm) signature.

The signature itself is composed of two scalar values `r,s`.

The Libbitcoin type signature  is a 64-byte, byte array container which holds the two 32-byte values. We will sign an arbitrary message hash in the following example.

<!-- Example (Part 1) -->

```c++
// Hash of an arbitrary msg.
auto msg = base16_literal(
  "04c294ab836b61955e762547c561a45e4be88984dca06da959d47bf880fd92f4");
hash_digest my_hash = bitcoin_hash(msg);

// My private & public keys.
auto my_secret = base16_literal(
    "f3c8f9a6198cca98f481edde13bcc031b1470a81e367b838fe9e0a9db0f5993d");
ec_compressed my_pubkey;
secret_to_public(my_pubkey, my_secret);

// Sign hash of TX with my private key.
ec_signature my_signature; //r,s
sign(my_signature, my_secret, my_hash);

// Verify Signature.
std::cout << verify_signature(my_pubkey, my_hash, my_signature) << std::endl;
```
**Deterministic ECSDA Signatures**  

Signing in Libbitcoin results in deterministic ECSDA signatures, which means that the "random" `k` value used to derive the signature is not actually random, but derived deterministically from both the message and private key.

This means that a signature generated in Libbitcoin will always be the same for a given message and private key.

## Serialised DER Signature Sequence

Bitcoin signatures are serialised in the DER format over the wire. The serialisation follows the form below.

* `30` - DER prefix
* `45` - Length of rest of Signature
* `02` - Marker for r value
* `21` - Length of r value
* `00ed...8f` - r value, Big Endian
* `02` - Marker for s value
* `21` - Length of s value
* `7a98...ed` - s value, Big Endian

In Libbitcoin, the serialised DER signature is represented by the `der_signature` type. We will format our signature from the previous example this way.

<!-- Example (Part 2) -->
```c++
// Format signature (r,s) as DER
der_signature my_der_signature;
encode_signature(my_der_signature, my_signature);

// DER serialisation
std::cout << encode_base16(my_der_signature) << std::endl;
```
**Strict DER Signatures (BIP66)**  
Bitcoin implementations have relied on OpenSSL for signing and verification of DER signatures. However, OpenSSL releases have deviated from the strict DER standard in the past.

For example, the DER standard does not allow `r,s` values with first byte values of `>0x7F`, whilst this is ignored by the OpenSSL implementation. [BIP66](https://github.com/bitcoin/bips/blob/master/bip-0066.mediawiki) was proposed to enforce strict DER adherence and is implemented in Libbitcoin.

We use a simple parser function to ensure a DER signature adheres to strict DER encoding rules.

<!-- Example (Part 3) -->
```c++
// Parse r,s values from DER formatted signature
// Strict enforcement of DER = true
std::cout << parse_signature(my_signature, my_der_signature, true)
    << std::endl;
```
