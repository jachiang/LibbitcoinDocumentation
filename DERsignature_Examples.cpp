#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>


using namespace bc;

void example() {

  //********** Part 1 **********

  // Hash of an arbitrary msg
  auto msg = base16_literal(
	  "04c294ab836b61955e762547c561a45e4be88984dca06da959d47bf880fd92f4");
  hash_digest my_hash = bitcoin_hash(msg);

  // My private & public keys
  auto my_secret = base16_literal(
      "f3c8f9a6198cca98f481edde13bcc031b1470a81e367b838fe9e0a9db0f5993d");
  ec_compressed my_pubkey;
  secret_to_public(my_pubkey, my_secret);
  
  // Sign hash of TX with my private key
  ec_signature my_signature; //r,s
  sign(my_signature, my_secret, my_hash);

  // Verify Signature
  std::cout << verify_signature(my_pubkey, my_hash, my_signature) << std::endl;

  //********** Part 2 **********

  // Format signature (r,s) as DER
  der_signature my_der_signature;
  encode_signature(my_der_signature, my_signature);

  // DER serialisation
  std::cout << encode_base16(my_der_signature) << std::endl;

  //********** Part 3 **********

  // Parse r,s values from DER formatted signature
  // Strict enforcement of DER = true
  std::cout << parse_signature(my_signature, my_der_signature, true)
      << std::endl;

}


int main() {

  example();

  return 0;

}
