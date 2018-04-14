#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>


using namespace bc;

void example() {
  // Hash of an arbitrary msg.
  auto msg = base16_literal(
	  "04c294ab836b61955e762547c561a45e4be88984dca06da959d47bf880fd92f4");
  hash_digest my_hash = bitcoin_hash(msg);

  // My private & public keys.
  auto my_secret = base16_literal(
      "f3c8f9a6198cca98f481edde13bcc031b1470a81e367b838fe9e0a9db0f5993d");
  auto my_pubkey = base16_literal(
      "02b974a3e9fe9ce1ca7f9bb86c114567a51cd8deb7157aeabcce46eb6138c3a1b3");

  recoverable_signature my_recoverable_sig;
  sign_recoverable(my_recoverable_sig, my_secret, my_hash);

  // Conversion: uint8_t -> unsigned int for visible ASCII output.
  std::cout << unsigned(my_recoverable_sig.recovery_id) << "\n";

  // Recover public key from recoverable signature.
  ec_compressed recovered_sig;
  recover_public(recovered_sig, my_recoverable_sig, my_hash);

  std::cout << (recovered_sig == my_pubkey) << std::endl;

}


int main() {

  example();

  return 0;

}
