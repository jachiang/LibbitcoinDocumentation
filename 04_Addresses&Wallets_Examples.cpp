#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>

using namespace bc;
using namespace wallet;

void example1() {
  //******* part 1 *******

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
  std::cout << encode_base58(prefix_pubkey_checksum) << std::endl;

  //******* part 2 *******

  //WIF encoded private key
  //Additional Information: Mainnet, PubKey compression
  one_byte secret_prefix = { { 0x80 } }; //Testnet Prefix: 0xEF
  one_byte secret_compressed = { { 0x01} }; //Omitted if uncompressed

  //Apply prefix, suffix & append checksum
  auto prefix_secret_comp_checksum = to_chunk(secret_prefix);
  extend_data(prefix_secret_comp_checksum, my_secret);
  extend_data(prefix_secret_comp_checksum, secret_compressed);
  append_checksum(prefix_secret_comp_checksum);

  //WIF (mainnet/compressed)
  std::cout << encode_base58(prefix_secret_comp_checksum) << std::endl;

  //******* part 3 *******

  //ec_private::mainnet = 0x8000 (Mainnet Prefixes 0x80,0x00)
  //ec_private::testnet = 0xEF6F (Testnet Prefixes 0xEF,0x6F)
  ec_private my_private(my_secret, ec_private::mainnet, true);
  std::cout << my_private.to_payment_address() << std::endl;
  std::cout << my_private.encoded() << std::endl; //WIF private key

  //******* part 4 *******

  //ec_public from ec_private
  //(network/compression implied from ec_private input)
  ec_public my_public(my_private);

  //ec_public from point
  //(network/compression not implied, supplied as arguments)
  ec_public my_public2(my_pubkey, true); //compression = true

  //Create payment_address from ec_public
  //(network argument supplied here)
  payment_address my_addr = my_public.to_payment_address(0x00); //0x00, 0x6f
  std::cout << my_addr.encoded() << std::endl;

}



void example2() {
  //******* part 1 *******

  //128, 160, 192, 224, 256 bits of Entropy are valid
  //Generate 128 bits of Entropy
  data_chunk my_entropy_128(16); //16 bytes
  pseudo_random_fill(my_entropy_128);

  //Instantiate mnemonic word_list
  word_list my_word_list = create_mnemonic(my_entropy_128);
  std::cout << join(my_word_list) << std::endl;

  //******* part 2 *******







  //Create 512bit seed
  auto hd_seed = decode_mnemonic(my_word_list); //no passphrase string?

  //prefix is concat of mainnet prefixes of both private and public
  uint64_t main_prefix = uint64_t(76066276) << 32 | uint64_t(76067358);
  // = hd_private::mainnet;

  //Create hd_private wallet
  auto my_hd_private = hd_private(to_chunk(hd_seed),main_prefix);

  //Mainnet: 0x0488B21E/76067358 public, 0x0488ADE4/76066276 private; testnet: 0x043587CF public, 0x04358394 private)
  std::cout << my_hd_private.encoded() << "\n"; //WIF -> xprv
  std::cout << encode_base16(my_hd_private.to_hd_key()) << "\n"; //82bytes = 656 bits

  //[ prefix ][ depth ][ parent fingerprint ][ key index ][ chain code ][ key ]
  // auto prefix = base16_literal("0488ADE4");
  // auto depth = base16_literal("00");
  // auto par_fingerprint = base16_literal("00000000"); //4 bytes of hash160 of parent
  // auto key_index = base16_literal("00000000"); //max FFFFFFFF

  //child private key
  uint32_t hd_first_hardened_key = 1 << 31;
  auto m0h = my_hd_private.derive_private(hd_first_hardened_key); //hd_first_hardened_key is

  //child public key key
  //check out the tests

}


void example3() {
  //******* part 1 *******

  //Load mnemonic sentence into word list
  std::string my_sentence = "market parent marriage drive umbrella custom leisure fury recipe steak have enable";
  auto my_word_list = split(my_sentence, " ", true);

  //Create an optional secret passphrase
  std::string my_passphrase = "my secret passphrase";

  //Create 512bit seed
  auto hd_seed = decode_mnemonic(my_word_list); //no passphrase string?

  //******* part 2 *******
  
  //We reuse 512 bit hd_seed from the previous example
  //To derive master private key m
  data_chunk seed_chunk(to_chunk(hd_seed));
  hd_private m(seed_chunk, hd_private::mainnet);

  //We now derive master public key M
  hd_public M = m.to_public();

  //EXPORT WIF FORMATS

}


int main() {

  std::cout << "Example 1: " << "\n";
  example1();
  std::cout << "\n";

  std::cout << "Example 2: " << "\n";
  example2();
  std::cout << "\n";

return  0;
}
