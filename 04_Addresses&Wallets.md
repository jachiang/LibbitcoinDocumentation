# Addresses & HD wallets
In the previous chapter we illustrated how to create EC private/public key pairs in Libbitcoin, which were then used to sign/verify potential transactions. In this chapter, we will cover how to derive publicly shareable Bitcoin addresses, and how to deterministically derive new addresses from existing private keys, which makes the management and recovery of Bitcoin wallets much easier.

## Creating a Bitcoin Address
A publicly shareable Bitcoin address is derived from a compressed or uncompressed public key with the additional information of whether it is intended for the Bitcoin mainnet or testnet.

<!-- Image of Address derivation -->

As illustrated above, the public key in its compressed or uncompressed form is first hashed with sha256 and subequently  Ripemd160 algorithms. The resulting digest receives a leading version prefix `mainnet = 0x00, testnet = 0x6f` byte. A 4-byte checksum of this format is then appended, before finally encoding the entire byte sequence in Base58.

```c++
//Namespace
using namespace bc;
using namespace wallet;
```
<!-- Example 1 -->
```c++
auto my_secret = base16_literal("f3c8f9a6198cca98f481edde13bcc031b1470a81e367b838fe9e0a9db0f5993d");

//Derive pubkey point
ec_compressed my_pubkey;
secret_to_public(my_pubkey, my_secret);

//Pubkeyhash: sha256 + hash160
auto my_pubkeyhash = bitcoin_short_hash(my_pubkey);

//Prefix for mainnet = 0x00
one_byte addr_prefix = { { 0x00 } }; //Testnet 0x6f

//Byte sequence = prefix + pubkey + checksum(4-bytes)
data_chunk prefix_pubkey_checksum(to_chunk(addr_prefix));
extend_data(prefix_pubkey_checksum, my_pubkeyhash);
append_checksum(prefix_pubkey_checksum);

//Base58 encode byte sequence -> Bitcoin Address
std::cout << encode_base58(prefix_pubkey_checksum);
```
In the next section we will cover Libbitcoin wallet types `ec_private` & `ec_public` feature methods which directly format Bitcoin addresses from secrets and public key points.

## A Basic Bitcoin Wallet
Now that we have created a public Bitcoin address from a private key secret, let us consider how a private key can be universally imported & exported from a wallet, independent of implementation, whilst maintaining the ability to derive the publicly shared Bitcoin address.

From the previous example we conclude that following pieces of information are required to create and recreate a specific Bitcoin address:

* Private Key
* Public Key Point Format (compressed or uncompressed)
* Mainnet or Testnet usage

The Wallet Import Format (WIF) provides these three pieces of information in a single byte sequence.

**WIF Private Key Derivation**
<!-- Image of WIF derivation -->  


We can now manually create a WIF private key following the preceding schema and byte sequence format.  
<!-- Example 1 (part2) -->
```c++
//WIF encoded private key
//Additional Information: Mainnet, PubKey compression
one_byte secret_prefix = { { 0x80 } }; //Testnet: 0xEF
one_byte secret_compressed = { { 0x01} }; //Omitted if uncompressed

//Apply prefix, suffix & append checksum
auto prefix_secret_comp_checksum = to_chunk(secret_prefix);
extend_data(prefix_secret_comp_checksum, my_secret);
extend_data(prefix_secret_comp_checksum, secret_compressed);
append_checksum(prefix_secret_comp_checksum);

//WIF (mainnet/compressed)
std::cout << encode_base58(prefix_secret_comp_checksum);
```
The WIF can easily be imported into any wallet, and provides all the information for its unique Bitcoin address to be derived.  

**Libbitcoin Wallet Types `ec_private`, `ec_public`**

In Libbitcoin, we can use the class `ec_private` to store a private key and necessary information to derive a unique Bitcoin address or export the private key in WIF form.
<!-- Example 1 (part3) -->
```c++
//ec_private::mainnet = 0x8000 (Mainnet Prefixes 0x80,0x00)
//ec_private::testnet = 0xEF6F (Testnet Prefixes 0xEF,0x6F)
ec_private my_private(my_secret, ec_private::mainnet, true);
std::cout << my_private.to_payment_address() << "\n";
std::cout << my_private.encoded() << "\n"; //WIF private key
```

The `ec_public` type is complementary and holds only following information:
* Public Key Point
* Public Key Point Format (compressed or uncompressed)  

The network (mainnet/testnet) argument must be provided to produce a Bitcoin address.

```c++
//ec_public from ec_private
//(network/compression implied from ec_private input)
ec_public my_public(my_private);

//ec_public from point
//(network/compression not implied, supplied as arguments)
ec_public my_public2(my_pubkey, true); //compression = true

//Create payment_address from ec_public
//(network argument supplied here)
payment_address my_addr = my_public.to_payment_address(0x00); //0x00, 0x6f
std::cout << my_addr.encoded() << "\n";
```

## Mnemonic Code Words

The seed of a wallet can be
This secret can range from 128 to 256 bits in 32bit increments, and together with its checksum, can be expressed in a mnemonic word list, with each word representing 11-bits each.

| Secret  Bits  | Checksum Bits | Mnemonic Words |
| ------------- |---------------| ---------------|
| 128           | 4             | 12             |
| 160           | 5             | 15             |
| 192           | 6             | 18             |
| 224           | 7             | 21             |
| 256           | 8             | 24             |

We use the Libbitcoin type `wallet::word_list` to instantiate a mnemonic word list, which can now be safely recorded as backup for recreating the wallet if necessary.
<!-- Example 2 (part 1) -->
```c++
//128, 160, 192, 224, 256 bits of Entropy are valid
//Generate 128 bits of Entropy
data_chunk my_entropy_128(16); //16 bytes
pseudo_random_fill(my_entropy_128);

//Instantiate mnemonic word_list
word_list my_word_list = create_mnemonic(my_entropy_128);
std::cout << join(my_word_list) << std::endl;
```
The `wallet::word_list` type is simply type alias of `std::vector<std::string>`, with each element containing a single word of the mnemonic word list.

**Key Stretching**

This 128 - 256 bit secret encoded by the mnemonic word list can be stretched to 512 bits using the "Password-Based Key Derivation Function 2" (PBKDF2) key-stretching function, which accepts two parameters: The mnemonic & a salt consisting of `"mnemonic"` and an optional passphrase.
<!-- Example 3 (part 1) -->
```c++
//Load mnemonic sentence into word list
std::string my_sentence = "market parent marriage drive umbrella custom leisure fury recipe steak have enable";
auto my_word_list = split(my_sentence, " ", true);

//Create an optional secret passphrase
std::string my_passphrase = "my secret passphrase";

//Create 512bit seed
auto hd_seed = decode_mnemonic(my_word_list); //no passphrase string?
```
This is often used as a key-stretching process to generate a seed for the HD wallet.


## Hierarchical Deterministic Wallets

Hierarchical deterministic (HD) wallets allow for an arbitrary number of Bitcoin private keys and addresses can be deterministically created and recreated from a **single root seed** controlled by the owner.

This root seed can have a length of 128, 256 or 512 bits. The 512 bit lengthened secret encoded by the mnemonic in the previous section is frequently used as the HD root seed to create a HD wallet.

The HD root seed is then hashed by the `HMAC-SHA512` algorithm to create a
512 bit digest, which is used in two separate parts:

* First 256 bits: Master Private key
* Second 256 bits: Master Chain Code

<!-- Illustration of HD key generation -->

**Libbitcoin HD Wallet Types: `hd_private`, `hd_public`**

Libbitcoin provides us with the `hd_private` type to instantiate the master private key. We can call a `hd_private` method to derive a `hd_public` object which contains the master public key.
<!-- Example 3 (part 2) -->
```c++
//We reuse 512 bit hd_seed from the previous example
//To derive master private key m
data_chunk seed_chunk(to_chunk(hd_seed));
hd_private m(seed_chunk, hd_private::mainnet);

//We now derive master public key M
hd_public M = m.to_public();

//EXPORT WIF FORMATS
```
Both types have methods to derive child keys described in subsequent sections of this chapter.

### Child Private Keys

The `HMAC-SHA512` is used to create child private keys with the following inputs:  
* 256 bit Parent Private Key
* 256 bit Parent Chain Code
* 32 bit Index Number  

The Index is incremented to generate additional child private key from the same parent private key.  

*Note that  indices between 0 and 2^31-1 (0x and 0x7FFFFFFF) are used for normal child derivation as described above. Higher indices are reserved for hardened child derivation, which will be covered in a later section in this chapter.*

<!-- Illustration of Child Private Key Generation -->

<!-- Example 3 (part 3) -->
```c++
//Derive children of master key m
auto m0 = m.derive_private(0);
auto m1 = m.derive_private(1);
auto m2 = m.derive_private(2);

//Derive further children
auto m10 = m1.derive_private(0); //Depth 2, Index 0
auto m11 = m1.derive_private(1); //Depth 2, Index 1
auto m12 = m1.derive_private(2); //Depth 2, Index 2
auto m100 = m10.derive_private(0); //Depth 3, Index 0
auto m101 = m10.derive_private(1); //Depth 3, Index 1
auto m102 = m10.derive_private(2); //Depth 3, Index 1

//Derive child public key
auto M00 = m0.derive_public(0); //Depth 2, Index 0
auto M01 = m0.derive_public(1); //Depth 2, Index 1
auto M02 = m0.derive_public(2); //Depth 2, Index 2
//...

//Derive hd_public of any hd_private object
//of same depth & index
auto M102 = m102.to_public();
```
Note that we have ....


### Child Public Keys

Due of the associative properties of EC operations, the following equivalency can be exploited to derive child public keys without knowing or exposing the public key:   

```c++
//Derivation of child_private_key
point(parent_private_key + left_256)
point(parent_private_key) + point(left_256)
parent_public_key + point(left_256) //no private key required.
```
So we can now create child public keys with `HMAC-SHA512` with the following inputs:
* 256 bit **Parent Public Key**
* 256 bit **Parent Chain Code**
* 32 bit **Index Number**  

So now it is possible for child public keys to be generated in a runtime environment to which the respective child private keys are not exposed to.

<!-- Illustration of public key generation -->

<!-- Example 3 (part 4) -->
```c++
//Derive public children of master key M
auto M0 = M.derive_public(0); //Depth 1, Index 0
auto M1 = M.derive_public(1); //Depth 1, Index 1
auto M2 = M.derive_public(2); //Depth 1, Index 2

//Derive further public children
auto M10 = M1.derive_public(0); //Depth 2, Index 0
auto M11 = M1.derive_public(1); //Depth 2, Index 1
auto M12 = M1.derive_public(2); //Depth 2, Index 2
auto M100 = M10.derive_public(0); //Depth 3, Index 0
auto M101 = M10.derive_public(1); //Depth 3, Index 1
//...

//No private children can be derived
//from child public keys!
```

### Extended Private & Public Keys

All information required to generate child private and public keys can be serialised in the *extended key format*.

*From left to right:*

* 4-byte **Version**
 * Private Key: Mainnet(0x0488ADE4)/Testnet(0x04358394 )
 * Public Key: Mainnet(0x0488B21E)/Testnet(0x043587CF )
* 1-byte **Depth**
* 4-byte **Fingerprint of parent**  
 * Hash160 ...
* 4-byte **Index Number**
* 32-byte **Parent Chain Code**
* 34-byte **Key**
 * Private Key: 0x00 + 32 bytes private key
 * Public Key: 34 byte compressed public key
* 4-byte **Checksum**

More extended serialisation details can be found in  [BIP32](https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki).

We can another, closer look at how we instantiated `hd_private` and `hd_public` objects from our previous example and understand how the extended key information is handled in these types.

```c++
//Versions prefix for both private and public keys
//hd_private::mainnet = 0x0488ADE40488B21E
hd_private m(seed_chunk, hd_private::mainnet);
auto m1 = m.derive_private(1);

//Public key mainnet prefix 0488B21E
//is implicitly passed on to hd_public M
hd_public M = m1.to_public();

//XPRV: m serialised as extended private key
auto m1_xprv = m1.to_hd_key();

//XPRV: Version in hex
auto m1_xprv_ver = slice<0,4>(m1_xprv);
std::cout << encode_base16(m1_xprv_ver) << std::endl;

//XPRV: Depth
auto m1_xprv_depth = slice<4,5>(m1_xprv);
std::cout << encode_base16(m1_xprv_depth) << std::endl;

//XPRV: Parent Fingerprint
auto m1_xprv_parent = slice<5,9>(m1_xprv);
std::cout << encode_base16(m1_xprv_parent) << std::endl;

//XPRV: Index Number
auto m1_xprv_index = slice<9,13>(m1_xprv);
std::cout << encode_base16(m1_xprv_index) << std::endl;

//XPRV: Chain Code
auto m1_xprv_chaincode = slice<13,45>(m1_xprv);
std::cout << encode_base16(m1_xprv_chaincode) << std::endl;

//XPRV: Private Key (with 0x00 prefix)
auto m1_xprv_private = slice<45,78>(m1_xprv);
std::cout << encode_base16(m1_xprv_private) << std::endl;

//XPRV: Checksum
auto m1_xprv_checksum = slice<78,82>(m1_xprv);
std::cout << encode_base16(m1_xprv_checksum) << std::endl;
```
### Hardened Child Keys

<!-- Illustration of parent key derivation -->

```c++
//example code of hardened k
```
