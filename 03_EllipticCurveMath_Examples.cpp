#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>


using namespace bc;

void example1() {
  //Generate 256 bit entropy.
  data_chunk myEntropy(ec_secret_size); //256bits
  pseudo_random_fill(myEntropy);

  //Instantiate private key with Entropy
  ec_secret mySecret; //256bit byte_array
  mySecret = to_array<ec_secret_size>(myEntropy);

  //Not all possible 256bits are member of Fp
  std::cout << verify(mySecret);
}

void example2() {
  ec_secret myScalar1 = base16_literal("f3c8f9a6198cca98f481edde13bcc031b1470a81e367b838fe9e0a9db0f5993d");
  ec_secret myScalar2 = base16_literal("04c294ab836b61955e762547c561a45e4be88984dca06da959d47bf880fd92f4");

  //Commutative addition:
  ec_secret myScalar1_add(myScalar1);
  ec_secret myScalar2_add(myScalar2);
  ec_add(myScalar1_add, myScalar2);  //myScalar1_add += myScalar2 % p
  ec_add(myScalar2_add, myScalar1);  //myScalar2_add += myScalar1 % p
  std::cout << (myScalar1_add == myScalar2_add);

  //Commutative multiplication:
  ec_secret myScalar1_mul(myScalar1);
  ec_secret myScalar2_mul(myScalar2);
  ec_add(myScalar1_mul, myScalar2);  //myScalar1_mul *= myScalar2 % p
  ec_add(myScalar2_mul, myScalar1);  //myScalar2_mul *= myScalar1 % p
  std::cout << (myScalar1_mul == myScalar2_mul);
}

void examplePoints() {
  ec_compressed point = base16_literal("0228026f91e1c97db3f6453262484ef5f69f71d89474f10926aae24d3c3eeb5f00");
  ec_uncompressed pointDecompressed;
  decompress(pointDecompressed, point);
  std::cout << encode_base16(pointDecompressed);
}


void example3() {
  //Private Key
  ec_secret mySecret = base16_literal("f3c8f9a6198cca98f481edde13bcc031b1470a81e367b838fe9e0a9db0f5993d");
  //The secp256k1 Generator Point
  ec_compressed pointGen = base16_literal("0279BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798");

  //Manually generating the public key
  ec_compressed myPubKey_compressed(pointGen);
  ec_multiply(myPubKey_compressed,mySecret);

  //Better: Using helper fct to generate the public key
  ec_compressed myPubKey2;
  secret_to_public(myPubKey2, mySecret);

  std::cout << (myPubKey_compressed == myPubKey2);
}

void example4() {
  //********** Part 1 **********

  //Hash of an arbitrary msg
  byte_array<32u> msg = base16_literal("04c294ab836b61955e762547c561a45e4be88984dca06da959d47bf880fd92f4");
  hash_digest myHash = bitcoin_hash(msg);

  //My private & public keys
  ec_secret mySecret = base16_literal("f3c8f9a6198cca98f481edde13bcc031b1470a81e367b838fe9e0a9db0f5993d");
  ec_compressed myPubKey = base16_literal("02b974a3e9fe9ce1ca7f9bb86c114567a51cd8deb7157aeabcce46eb6138c3a1b3");

  //Sign hash of TX with my private key
  ec_signature mySignature; //r,s
  sign(mySignature, mySecret, myHash);
  std::cout << encode_base16(mySignature) << "\n";

  //Verify Signature
  std::cout << verify_signature(myPubKey, myHash, mySignature) << "\n";

  //********** Part 2 **********

  //Format mySignature (r,s) as a DER signature sequence
  der_signature myDerSignature;
  encode_signature(myDerSignature, mySignature);

  //DER Signature Format (Strict DER Signature)
  std::cout << encode_base16(myDerSignature) << "\n";

  //********** Part 3 **********

  //Parse r,s values from myDerSignature
  //Strict enforcement of DER = true
  parse_signature(mySignature, myDerSignature, true);

  //********** Part 4 **********

  recoverable_signature myRecoverableSig;
  sign_recoverable(myRecoverableSig, mySecret, myHash);
  std::cout << unsigned(myRecoverableSig.recovery_id) << "\n";

  //recover public key from recoverable signature
  ec_compressed recoveredSig;
  recover_public(recoveredSig, myRecoverableSig, myHash);

  std::cout << (recoveredSig == myPubKey);

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
