# Examples: Addresses, Wallets and HD Wallets

All examples from the Bitcoin addresses and HD wallets documentation chapter are shown here in full. The specific examples referenced in the subsections are wrapped in the functions listed below.

**Addresses, WIF format, Wallets**
* create_address_wif_wallet()

**Mnemonic Word Lists**
* create_mnemonic_from_entropy()

**HD Children**
* create_hd_children()

**Extended Keys, Hardened Children**
* create_extended_hardened_keys()

**Libbitcoin API:** Version 3. Libbitcoin must be compiled with [ICU (International Components for Unicode)](https://github.com/libbitcoin/libbitcoin/blob/master/README.md) to work with the optional mnemonic secret passphrase.

Script below is ready-to-compile: `g++ -std=c++11 -o addresses_hd_wallets addresses_hd_wallets_examples.cpp $(pkg-config --cflags libbitcoin --libs libbitcoin)`

```c++
#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>

using namespace bc;
using namespace wallet;

void create_address_wif_wallet() {
    // ******* part 1 *******
    // Begin with a private key
    auto my_secret = base16_literal(
        "f3c8f9a6198cca98f481edde13bcc031b1470a81e367b838fe9e0a9db0f5993d");

    // Derive pubkey point
    ec_compressed my_pubkey;
    secret_to_public(my_pubkey, my_secret);

    // Pubkeyhash: sha256 + hash160
    auto my_pubkeyhash = bitcoin_short_hash(my_pubkey);

    // Prefix for mainnet = 0x00
    one_byte addr_prefix = { { 0x00 } }; //Testnet 0x6f

    // Byte sequence = prefix + pubkey + checksum(4-bytes)
    data_chunk prefix_pubkey_checksum(to_chunk(addr_prefix));
    extend_data(prefix_pubkey_checksum, my_pubkeyhash);
    append_checksum(prefix_pubkey_checksum);

    // Base58 encode byte sequence -> Bitcoin Address
    std::cout << encode_base58(prefix_pubkey_checksum) << std::endl;

    // You can directly generate Bitcoin addresses
    // with Libbitcoin wallet types: ec_private/ec_public
    // described in the following section


    //******* part 2 *******

    // WIF encoded private key
    // Additional Information: Mainnet, PubKey compression
    one_byte secret_prefix = { { 0x80 } }; //Testnet Prefix: 0xEF
    one_byte secret_compressed = { { 0x01} }; //Omitted if uncompressed

    // Apply prefix, suffix & append checksum
    auto prefix_secret_comp_checksum = to_chunk(secret_prefix);
    extend_data(prefix_secret_comp_checksum, my_secret);
    extend_data(prefix_secret_comp_checksum, secret_compressed);
    append_checksum(prefix_secret_comp_checksum);

    // WIF (mainnet/compressed)
    std::cout << encode_base58(prefix_secret_comp_checksum) << std::endl;

    // ******* part 3 *******
    // Instantiate ec_private object
    // ec_private::mainnet = 0x8000 (Mainnet Prefixes 0x80,0x00)
    // ec_private::testnet = 0xEF6F (Testnet Prefixes 0xEF,0x6F)
    ec_private my_private(my_secret, ec_private::mainnet, true);
    std::cout << my_private.to_payment_address() << std::endl;
    std::cout << my_private.encoded() << std::endl; //WIF private key

    // ******* part 4 *******

    // ec_public from ec_private
    // (compression implied from ec_private input)
    ec_public my_public(my_private);

    // ec_public from point
    // (compression not implied, supplied as arguments)
    ec_public my_public2(my_pubkey, true); //compression = true

    // Payment addresses:
    // Will always default to mainnet if no argument supplied
    // regardless of version in ec_private constructor argument
    payment_address my_addr = my_public.to_payment_address();
    payment_address my_addr2 = my_public2.to_payment_address(); //0x00, 0x6f
    std::cout << (my_addr.encoded() == my_addr2.encoded()) << std::endl;

}


void create_mnemonic_from_entropy() {
    // ******* part 1 *******

    // 128, 160, 192, 224, 256 bits of Entropy are valid
    // We generate 128 bits of Entropy
    data_chunk my_entropy_128(16); //16 bytes = 128 bits
    pseudo_random_fill(my_entropy_128);

    // Instantiate mnemonic word_list
    word_list my_word_list = create_mnemonic(my_entropy_128);
    std::cout << join(my_word_list) << std::endl; //join to a single string with spaces
}


void create_hd_children() {

    // ******* part 1 *******

    // Load mnemonic sentence into word list
    std::string my_sentence = "market parent marriage drive umbrella custom leisure fury recipe steak have enable";
    auto my_word_list = split(my_sentence, " ", true);

    // Create an optional secret passphrase
    std::string my_passphrase = "my secret passphrase";

    // Create 512bit seed (without optional secret passphrase)
    auto hd_seed = decode_mnemonic(my_word_list);

    // Create 512bit seed (with optional secret passphrase)
    // Requires: Libbitcoin compiled with ICU.
    // auto hd_seed = decode_mnemonic(my_word_list, my_passphrase);

    // ******* part 2 *******

    // We reuse 512 bit hd_seed from the previous example
    // Derivation of master private key m
    data_chunk seed_chunk(to_chunk(hd_seed));
    hd_private m(seed_chunk, hd_private::mainnet);

    // Derivation of master public key M
    hd_public M = m.to_public();


    // ******* part 3 *******

    // Derive children of master key m
    auto m0 = m.derive_private(0);
    auto m1 = m.derive_private(1);
    auto m2 = m.derive_private(2);

    // Derive grandchild private keys
    auto m10 = m1.derive_private(0); //Depth 2, Index 0
    auto m11 = m1.derive_private(1); //Depth 2, Index 1
    auto m12 = m1.derive_private(2); //Depth 2, Index 2
    auto m100 = m10.derive_private(0); //Depth 3, Index 0
    auto m101 = m10.derive_private(1); //Depth 3, Index 1
    auto m102 = m10.derive_private(2); //Depth 3, Index 1

    // Derive grandchild public keys
    auto M00 = m0.derive_public(0); //Depth 2, Index 0
    auto M01 = m0.derive_public(1); //Depth 2, Index 1
    auto M02 = m0.derive_public(2); //Depth 2, Index 2
    // ...

    // Derive hd_public of any hd_private object
    // of same depth & index
    auto M102 = m102.to_public();


    // ******* part 4 *******

    // Derive public children of master key M
    auto M0 = M.derive_public(0); //Depth 1, Index 0
    auto M1 = M.derive_public(1); //Depth 1, Index 1
    auto M2 = M.derive_public(2); //Depth 1, Index 2

    // Derive further public children
    auto M10 = M1.derive_public(0); //Depth 2, Index 0
    auto M11 = M1.derive_public(1); //Depth 2, Index 1
    auto M12 = M1.derive_public(2); //Depth 2, Index 2
    auto M100 = M10.derive_public(0); //Depth 3, Index 0
    auto M101 = M10.derive_public(1); //Depth 3, Index 1
    // ...

    // No private children can be derived
    // from child public keys!


}

void create_extended_hardened_keys() {

    // Load mnemonic sentence into word list
    std::string my_sentence = "market parent marriage drive umbrella custom leisure fury recipe steak have enable";
    auto my_word_list = split(my_sentence, " ", true);
    // Create an optional secret passphrase
    std::string my_passphrase = "my secret passphrase";
    // Create 512bit seed
    auto hd_seed = decode_mnemonic(my_word_list); //2nd argument: my_passphrase
    data_chunk seed_chunk(to_chunk(hd_seed));


    // ******* part 1 *******
    // Generate master private key
    // hd_private::mainnet = 0x0488ADE40488B21E
    // Versions for both private and public keys
    hd_private m(seed_chunk, hd_private::mainnet);
    auto m1 = m.derive_private(1);

    // Public key mainnet prefix 0488B21E
    // is implicitly passed on to M
    hd_public M = m1.to_public();

    // Extended Private Key:
    // m1 serialised in extended private key format
    auto m1_xprv = m.to_hd_key();

    // 4 Bytes: Version in hex
    auto m1_xprv_ver = slice<0,4>(m1_xprv);
    std::cout << encode_base16(m1_xprv_ver) << std::endl;

    // 1-Byte: Depth
    auto m1_xprv_depth = slice<4,5>(m1_xprv);
    std::cout << encode_base16(m1_xprv_depth) << std::endl;

    // 4-Bytes: Parent Fingerprint
    auto m1_xprv_parent = slice<5,9>(m1_xprv);
    std::cout << encode_base16(m1_xprv_parent) << std::endl;

    // 4-Bytes: Index Number
    auto m1_xprv_index = slice<9,13>(m1_xprv);
    std::cout << encode_base16(m1_xprv_index) << std::endl;

    // 32-Bytes: Chain Code
    auto m1_xprv_chaincode = slice<13,45>(m1_xprv);
    std::cout << encode_base16(m1_xprv_chaincode) << std::endl;

    // 34-Bytes: Private Key (with 0x00 prefix)
    auto m1_xprv_private = slice<45,78>(m1_xprv);
    std::cout << encode_base16(m1_xprv_private) << std::endl;

    // 4-Bytes: double sha256 checksum
    auto m1_xprv_checksum = slice<78,82>(m1_xprv);
    std::cout << encode_base16(m1_xprv_checksum) << std::endl;

    // //checksum test
    // auto checksum_test = slice<0,78>(m1_xprv);
    // auto checksum_test_slice = to_chunk(checksum_test);
    // append_checksum(checksum_test_slice);
    // auto rest = slice<78,82>(to_array<82>(checksum_test_slice));
    // std::cout << encode_base16(rest) << std::endl;


    // ******* part 2 *******

    // Hardened private key derivation with index >= 1 << 31
    auto m0 = m.derive_private(0);
    auto m00h = m.derive_private(hd_first_hardened_key);
    auto m01h = m.derive_private(1 + hd_first_hardened_key);
    auto m02h = m.derive_private(2 + hd_first_hardened_key);

    // Hardened public key can only be derived from private key
    auto M00h = m00h.to_public();
    // or from parent private key
    auto M00h_ = m.derive_public(hd_first_hardened_key);
    // Above keys are equivalent
    std::cout << (M00h == M00h_) << std::endl;



}


int main() {

    std::cout << "Address, WIF, Wallets: " << "\n";
    create_address_wif_wallet();
    std::cout << "\n";

    std::cout << "Mnemonic Word Lists: " << "\n";
    create_mnemonic_from_entropy();
    std::cout << "\n";

    create_hd_children();

    std::cout << "Extended & Hardened Keys: " << "\n";
    create_extended_hardened_keys();
    std::cout << "\n";

return  0;

}

```
