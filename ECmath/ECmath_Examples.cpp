#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>


using namespace bc;

void create_verify_private_key() {
    // We derive a new Bitcoin private key.

    // Generate 256 bits of entropy.
    data_chunk my_entropy(ec_secret_size); //256bits
    pseudo_random_fill(my_entropy);

    // Instantiate private key with 256 bits of entropy.
    auto my_secret = to_array<ec_secret_size>(my_entropy);

    // Not all possible 256bits are valid secret keys.
    // Verify that my_secret is member of finite field Fp.
    std::cout << verify(my_secret) << std::endl;
}

void ec_operations() {

    auto my_scalar1 = base16_literal(
          "f3c8f9a6198cca98f481edde13bcc031b1470a81e367b838fe9e0a9db0f5993d");
    auto my_scalar2 = base16_literal(
          "04c294ab836b61955e762547c561a45e4be88984dca06da959d47bf880fd92f4");

    // Commutative ec addition:
    ec_secret my_scalar_added1(my_scalar1);
    ec_secret my_scalar_added2(my_scalar2);
    // my_scalar1_added1 += my_scalar2 % p
    ec_add(my_scalar_added1, my_scalar2);
    // my_scalar2_added2 += my_scalar1 % p
    ec_add(my_scalar_added2, my_scalar1);
    std::cout << (my_scalar_added1 == my_scalar_added2) << std::endl;

    // Commutative ec multiplication:
    ec_secret my_scalar_multiplied1(my_scalar1);
    ec_secret my_scalar_multiplied2(my_scalar2);
    // my_scalar_multiplied1 *= my_scalar2 % p
    ec_add(my_scalar_multiplied1, my_scalar2);
    // my_scalar_multiplied2 *= my_scalar1 % p
    ec_add(my_scalar_multiplied2, my_scalar1);
    std::cout << (my_scalar_multiplied1 == my_scalar_multiplied2) << std::endl;

}

void decompress_point() {
    auto point = base16_literal(
        "0228026f91e1c97db3f6453262484ef5f69f71d89474f10926aae24d3c3eeb5f00");
    ec_uncompressed point_decompressed;
    decompress(point_decompressed, point);
    std::cout << encode_base16(point_decompressed) <<std::endl;
}


void create_public_key() {
    // Private Key.
    auto my_secret = base16_literal(
        "f3c8f9a6198cca98f481edde13bcc031b1470a81e367b838fe9e0a9db0f5993d");

    // The secp256k1 Generator Point.
    auto gen_point = base16_literal(
        "0279BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798");

    // Manually generating the public key.
    ec_compressed my_pubkey_compressed(gen_point);
    ec_multiply(my_pubkey_compressed,my_secret);

    // Better: Using helper fct to generate the public key.
    ec_compressed my_alternative_pubkey_compressed;
    secret_to_public(my_alternative_pubkey_compressed, my_secret);

    std::cout << (my_pubkey_compressed == my_alternative_pubkey_compressed)
              << std::endl;
}


int main() {

  std::cout << "Private key is valid: " << std::endl;
  create_verify_private_key();

  std::cout << "Commutative EC Operations: " << std::endl;
  ec_operations();

  std::cout << "(Un)compressed points: " << std::endl;
  decompress_point();

  std::cout << "Public Key from Generator Point: " << std::endl;
  create_public_key();

  return 0;

}
