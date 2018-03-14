#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>


using namespace bc;

void example1() {
  //Generate 256 bits of entropy.
  data_chunk my_entropy(ec_secret_size); //256bits
  pseudo_random_fill(my_entropy);

  //Instantiate private key with 256 bits of Entropy
  auto my_secret = to_array<ec_secret_size>(my_entropy);

  //Not all possible 256bits are member of Fp
  std::cout << encode_base16(my_secret) << std::endl;
  std::cout << verify(my_secret) << std::endl;
}

void example2() {
  auto my_scalar1 = base16_literal("f3c8f9a6198cca98f481edde13bcc031b1470a81e367b838fe9e0a9db0f5993d");
  auto my_scalar2 = base16_literal("04c294ab836b61955e762547c561a45e4be88984dca06da959d47bf880fd92f4");

  //Commutative addition:
  ec_secret my_scalar1_add(my_scalar1);
  ec_secret my_scalar2_add(my_scalar2);
  ec_add(my_scalar1_add, my_scalar2);  //my_scalar1_add += my_scalar2 % p
  ec_add(my_scalar2_add, my_scalar1);  //my_scalar2_add += my_scalar1 % p
  std::cout << (my_scalar1_add == my_scalar2_add);

  //Commutative multiplication:
  ec_secret my_scalar1_mul(my_scalar1);
  ec_secret my_scalar2_mul(my_scalar2);
  ec_add(my_scalar1_mul, my_scalar2);  //my_scalar1_mul *= my_scalar2 % p
  ec_add(my_scalar2_mul, my_scalar1);  //my_scalar2_mul *= my_scalar1 % p
  std::cout << (my_scalar1_mul == my_scalar2_mul);
}

void examplePoints() {
  auto point = base16_literal("0228026f91e1c97db3f6453262484ef5f69f71d89474f10926aae24d3c3eeb5f00");
  ec_uncompressed point_decompressed;
  decompress(point_decompressed, point);
  std::cout << encode_base16(point_decompressed);
}


void example3() {
  //Private Key
  auto my_secret = base16_literal("f3c8f9a6198cca98f481edde13bcc031b1470a81e367b838fe9e0a9db0f5993d");

  //The secp256k1 Generator Point
  auto gen_point = base16_literal("0279BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798");

  //Manually generating the public key
  ec_compressed my_pubkey_compressed(gen_point);
  ec_multiply(my_pubkey_compressed,my_secret);

  //Better: Using helper fct to generate the public key
  ec_compressed my_pubkey2;
  secret_to_public(my_pubkey2, my_secret);

  std::cout << (my_pubkey_compressed == my_pubkey2);
}

void example4() {
  //********** Part 1 **********

  //Hash of an arbitrary msg
  auto msg = base16_literal("04c294ab836b61955e762547c561a45e4be88984dca06da959d47bf880fd92f4");
  hash_digest my_hash = bitcoin_hash(msg);

  //My private & public keys
  auto my_secret = base16_literal("f3c8f9a6198cca98f481edde13bcc031b1470a81e367b838fe9e0a9db0f5993d");
  auto my_pubkey = base16_literal("02b974a3e9fe9ce1ca7f9bb86c114567a51cd8deb7157aeabcce46eb6138c3a1b3");

  //Sign hash of TX with my private key
  ec_signature my_signature; //r,s
  sign(my_signature, my_secret, my_hash);

  //Verify Signature
  std::cout << verify_signature(my_pubkey, my_hash, my_signature) << "\n";

  //********** Part 2 **********

  //Format my_signature (r,s) as a DER signature sequence
  der_signature my_der_signature;
  encode_signature(my_der_signature, my_signature);

  //DER Signature Format (Strict DER Signature)
  std::cout << encode_base16(my_der_signature) << "\n";

  //********** Part 3 **********

  //Parse r,s values from my_der_signature
  //Strict enforcement of DER = true
  parse_signature(my_signature, my_der_signature, true);

  //********** Part 4 **********

  recoverable_signature my_recoverable_sig;
  sign_recoverable(my_recoverable_sig, my_secret, my_hash);
  //Conversion: uint8_t->unsigned int for visible ASCII output
  std::cout << unsigned(my_recoverable_sig.recovery_id) << "\n";

  //recover public key from recoverable signature
  ec_compressed recovered_sig;
  recover_public(recovered_sig, my_recoverable_sig, my_hash);

  std::cout << (recovered_sig == my_pubkey);

}


int main() {

  std::cout << "Example 1: " << "\n";
  example1();
  std::cout << "\n";

  std::cout << "Example 2: " << "\n";
  example2();
  std::cout << "\n";

  std::cout << "(Un)compressed points: " << "\n";
  examplePoints();
  std::cout << "\n";

  std::cout << "Example 3: " << "\n";
  example3();
  std::cout << "\n";

  std::cout << "Example 4: " << "\n";
  example4();
  std::cout << "\n";

  return 0;

}
