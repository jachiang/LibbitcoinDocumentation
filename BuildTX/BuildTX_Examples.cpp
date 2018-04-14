#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>

using namespace bc;
using namespace wallet;
using namespace chain;
using namespace machine;


void example_1() {

  //******* part 1 *******

  //Instantiate tx object
  transaction tx;

  //******* part 2 *******

  //version 1
  uint32_t version = 1u;
  tx.set_version(version);

  //print version in serialised format
  auto serialised_version = to_little_endian(tx.version());
  std::cout << encode_base16(to_chunk(serialised_version));

  //******* part 3 *******

  //Previous TX hash
  std::string prev_tx_string_0 = "ca05e6c14fe816c93b91dd4c8f00e60e4a205da85741f26326d6f21f9a5ac5e9";
  hash_digest prev_tx_hash_0;
  decode_hash(prev_tx_hash_0,prev_tx_string_0);

  //Previous UXTO index:
  uint32_t index0 = 0;
  output_point uxto_tospend_0(prev_tx_hash_0, index0);

  //Build input_0 object
  input input_0;
  input_0.set_previous_output(uxto_tospend_0);
  input_0.set_sequence(0xffffffff);

  //Additional input objects can be created for additional inputs

  //All input objects can then be added to transaction
  tx.inputs().push_back(input_0);       //first input
  //tx.inputs().push_back(input_1);     //second input
                                        //...nth input

  //Unlocking script/scriptSig will be added later.


  //******* part 4 *******

  //Destination Address
  auto my_address_raw = "mmbmNXo7QZWU2WgWwvrtnyQrwffngWScFe";
  payment_address my_address1(my_address_raw);

  //Create Output locking script/scriptPubKey from template:
  operation::list locking_script_0=script::to_pay_key_hash_pattern(my_address1.hash());

  //Define Output amount
  std::string btc_amount_string_0 = "1.295";
  uint64_t satoshi_amount_0;
  decode_base10(satoshi_amount_0, btc_amount_string_0, btc_decimal_places); // btc_decimal_places = 8

  //Create output_0 object
  output output_0(satoshi_amount_0, locking_script_0);

  //Above can be repeated for other outputs

  //Add outputs to TX
  tx.outputs().push_back(output_0);     //first output
  //tx.outputs().push_back(output_1);   //second output
  //tx.outputs().push_back(output_n);   //...nth output

  //******* part 5 *******

  //We rebuild our P2PKH script manually:
  operation::list my_own_p2pkh;
  my_own_p2pkh.push_back(operation(opcode::dup));
  my_own_p2pkh.push_back(operation(opcode::hash160));
  operation op_pubkey = operation(to_chunk(my_address1.hash()));
  my_own_p2pkh.push_back(op_pubkey); //includes hash length prefix
  my_own_p2pkh.push_back(operation(opcode::equalverify));
  my_own_p2pkh.push_back(operation(opcode::checksig));

  //The two operation lists are equivalent
  std::cout << (my_own_p2pkh == locking_script_0) << std::endl;

  //******* part 6 *******

  //Signer: Secret > Pubkey > Address
  auto my_secret0 = base16_literal("3eec08386d08321cd7143859e9bf4d6f65a71d24f37536d76b4224fdea48009f");
  ec_private my_private0(my_secret0, ec_private::testnet, true);
  ec_compressed pubkey0= my_private0.to_public().point();
  payment_address my_address0 = my_private0.to_payment_address();

  //Signature
  endorsement sig_0;
  script prev_script_0 = script::to_pay_key_hash_pattern(my_address0.hash());
  uint8_t input0_index(0u);
  script::create_endorsement(sig_0, my_secret0, prev_script_0, tx, input0_index, 0x01);

  //Create sigScript
  operation::list sig_script_0;
  sig_script_0.push_back(operation(sig_0));
  sig_script_0.push_back(operation(to_chunk(pubkey0)));
  script my_unlocking_script_0(sig_script_0);

  //Add sigScript to first input in transaction
  tx.inputs()[0].set_script(my_unlocking_script_0);

  //Print serialised transaction
  std::cout << encode_base16(tx.to_data()) << std::endl;

}


int main() {

  std::cout << "Example 1: " << "\n";
  example_1();
  std::cout << "\n";

  return 0;

}
