#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>

using namespace bc;
using namespace wallet;
using namespace chain;
using namespace machine;


void example_1() {
    //SETUP (IDENTICAL TO EXAMPLE_1)
    //Begin with a private key
    auto my_secret0 = base16_literal("3eec08386d08321cd7143859e9bf4d6f65a71d24f37536d76b4224fdea48009f");
    auto my_secret1 = base16_literal("86faa240ae2b0f28b125a42961bd3adf9d5f5dc6a1deaa5feda04e7be8c872f6");
    auto my_secret2 = base16_literal("b7423c94ab99d3295c1af7e7bbea47c75d298f7190ca2077b53bae61299b70a5");
    auto my_secret3 = base16_literal("d977e2ce0f744dc3432cde9813a99360a3f79f7c8035ef82310d54c57332b2cc");
    ec_private my_private0(my_secret0, ec_private::testnet, true); //compressed
    ec_private my_private1(my_secret1, ec_private::testnet, true); //compressed
    ec_private my_private2(my_secret2, ec_private::testnet, true); //compressed
    ec_private my_private3(my_secret3, ec_private::testnet, true); //compressed
    payment_address my_address0 = my_private0.to_payment_address();
    payment_address my_address1 = my_private1.to_payment_address();
    payment_address my_address2 = my_private2.to_payment_address();
    payment_address my_address3 = my_private3.to_payment_address();
    ec_compressed pubkey0= my_private0.to_public().point();
    ec_compressed pubkey1= my_private1.to_public().point();
    std::cout << my_address0.encoded() << std::endl;
    std::cout << my_address1.encoded() << std::endl;
    std::cout << my_address2.encoded() << std::endl;
    std::cout << my_address3.encoded() << std::endl;

    //version
    uint32_t version = 1u;
    transaction tx;
    tx.set_version(version);

    //build input 0
    std::string prev_tx_string_0 = "7ea970031b28fcc1cef517dfa7d812cb61c409aec37a0463e951a05700d61b73";
    hash_digest prev_tx_hash_0;
    decode_hash(prev_tx_hash_0,prev_tx_string_0);
    //prev uxto index
    uint32_t index0 = 0;
    output_point uxto_tospend_0(prev_tx_hash_0, index0);
    //input object
    input input_0;
    input_0.set_previous_output(uxto_tospend_0);   //sequence is set in input then...
    input_0.set_sequence(0xffffffff); //standard, no locktime
    //build input 1
    std::string prev_tx_string_1 = "32d070ed7d387b9db02bf35f3ba1c0ee61837c2226fd5cbf0c913525a9be869d";
    hash_digest prev_tx_hash_1;
    decode_hash(prev_tx_hash_1,prev_tx_string_1);
    //prev uxto index
    uint32_t index1 = 0;
    output_point uxto_tospend_1(prev_tx_hash_1, index1);
    //input object
    input input_1;
    input_1.set_previous_output(uxto_tospend_1);   //sequence is set in input then...
    input_1.set_sequence(0xffffffff); //standard, no locktime

    //build output 0
    operation::list locking_script_0=script::to_pay_key_hash_pattern(my_address2.hash());
    std::string btc_amount_string_0 = "1";
    uint64_t satoshi_amount_0;
    decode_base10(satoshi_amount_0, btc_amount_string_0, btc_decimal_places); //8 decimals = btc_decimal_places
    output output_0(satoshi_amount_0, locking_script_0); //1st arg satoshis, 2nd arg output script
    //build output 1
    operation::list locking_script_1=script::to_pay_key_hash_pattern(my_address3.hash());
    std::string btc_amount_string_1 = "0.945";
    uint64_t satoshi_amount_1;
    decode_base10(satoshi_amount_1, btc_amount_string_1, btc_decimal_places); //8 decimals = btc_decimal_places
    output output_1(satoshi_amount_1, locking_script_1); //1st arg satoshis, 2nd arg output script
    //build locktime


    //****************************************//

    //Not shown:
    //Build inputs (w/o scriptSig)
    //Build outputs

    //Finalise TX - can't be modified later
    tx.inputs().push_back(input_0);   //first input
    tx.inputs().push_back(input_1);   //second input
                                      //...nth input
    tx.outputs().push_back(output_0); //first output
    tx.outputs().push_back(output_1); //second output
                                      //...nth output

    //Construct previous locking script of input_0 & input_1
    script prev_script_0 = script::to_pay_key_hash_pattern(my_address0.hash());
    script prev_script_1 = script::to_pay_key_hash_pattern(my_address1.hash());

    //TX signature for input_0
    endorsement sig_0;
    uint8_t input0_index(0u);
    script::create_endorsement(sig_0, my_secret0, prev_script_0, tx, input0_index, 0x01);

    //TX signature for input_1
    endorsement sig_1;
    uint8_t input1_index(1u);
    script::create_endorsement(sig_1, my_secret1, prev_script_1, tx, input1_index, 0x01);

    //Construct unlocking script_0
    operation::list sig_script_0;
    sig_script_0.push_back(operation(sig_0));
    sig_script_0.push_back(operation(to_chunk(pubkey0)));
    script my_unlocking_script_0(sig_script_0);

    //Construct unlocking script_1
    operation::list sig_script_1;
    sig_script_1.push_back(operation(sig_1));
    sig_script_1.push_back(operation(to_chunk(pubkey1)));
    script my_unlocking_script_1(sig_script_1);

    //Add unlockingscript to TX
    tx.inputs()[0].set_script(my_unlocking_script_0);
    tx.inputs()[1].set_script(my_unlocking_script_1);

    //SINGLE: We cannot modify TX after signing

    //Print out
    std::cout << encode_base16(tx.to_data()) << std::endl;

}

void example_2() {
  //SETUP (IDENTICAL TO EXAMPLE_1)
  //Begin with a private key
  auto my_secret0 = base16_literal("3eec08386d08321cd7143859e9bf4d6f65a71d24f37536d76b4224fdea48009f");
  auto my_secret1 = base16_literal("86faa240ae2b0f28b125a42961bd3adf9d5f5dc6a1deaa5feda04e7be8c872f6");
  auto my_secret2 = base16_literal("b7423c94ab99d3295c1af7e7bbea47c75d298f7190ca2077b53bae61299b70a5");
  auto my_secret3 = base16_literal("d977e2ce0f744dc3432cde9813a99360a3f79f7c8035ef82310d54c57332b2cc");
  ec_private my_private0(my_secret0, ec_private::testnet, true); //compressed
  ec_private my_private1(my_secret1, ec_private::testnet, true); //compressed
  ec_private my_private2(my_secret2, ec_private::testnet, true); //compressed
  ec_private my_private3(my_secret3, ec_private::testnet, true); //compressed
  payment_address my_address0 = my_private0.to_payment_address();
  payment_address my_address1 = my_private1.to_payment_address();
  payment_address my_address2 = my_private2.to_payment_address();
  payment_address my_address3 = my_private3.to_payment_address();
  ec_compressed pubkey0= my_private0.to_public().point();
  ec_compressed pubkey1= my_private1.to_public().point();
  std::cout << my_address0.encoded() << std::endl;
  std::cout << my_address1.encoded() << std::endl;
  std::cout << my_address2.encoded() << std::endl;
  std::cout << my_address3.encoded() << std::endl;

  //version
  uint32_t version = 1u;
  transaction tx;
  tx.set_version(version);

  //build input 0
  std::string prev_tx_string_0 = "e964ed0883933ae8f3f53139efef149b0cedb7895a040cab3b64e792acd11412";
  hash_digest prev_tx_hash_0;
  decode_hash(prev_tx_hash_0,prev_tx_string_0);
  //prev uxto index
  uint32_t index0 = 0;
  output_point uxto_tospend_0(prev_tx_hash_0, index0);
  //input object
  input input_0;
  input_0.set_previous_output(uxto_tospend_0);   //sequence is set in input then...
  input_0.set_sequence(0xffffffff); //standard, no locktime
  //build input 1
  std::string prev_tx_string_1 = "0aea180c6a3b0233574e9e51f065cd1996f1db6f8a10c72fc480151be323d956";
  hash_digest prev_tx_hash_1;
  decode_hash(prev_tx_hash_1,prev_tx_string_1);
  //prev uxto index
  uint32_t index1 = 0;
  output_point uxto_tospend_1(prev_tx_hash_1, index1);
  //input object
  input input_1;
  input_1.set_previous_output(uxto_tospend_1);   //sequence is set in input then...
  input_1.set_sequence(0xffffffff); //standard, no locktime

  //build output 0
  operation::list locking_script_0=script::to_pay_key_hash_pattern(my_address2.hash());
  std::string btc_amount_string_0 = "0.2";
  uint64_t satoshi_amount_0;
  decode_base10(satoshi_amount_0, btc_amount_string_0, btc_decimal_places); //8 decimals = btc_decimal_places
  output output_0(satoshi_amount_0, locking_script_0); //1st arg satoshis, 2nd arg output script
  //build output 1
  operation::list locking_script_1=script::to_pay_key_hash_pattern(my_address3.hash());
  std::string btc_amount_string_1 = "0.287";
  uint64_t satoshi_amount_1;
  decode_base10(satoshi_amount_1, btc_amount_string_1, btc_decimal_places); //8 decimals = btc_decimal_places
  output output_1(satoshi_amount_1, locking_script_1); //1st arg satoshis, 2nd arg output script
  //build locktime


  //****************************************//
  //SIGHASH NONE example

  //Not shown:
  //Build inputs (w/o scriptSig)
  //Build outputs

  //We only need to finalise inputs. Outputs can be modified after signing.
  tx.inputs().push_back(input_0);   //first input
  tx.inputs().push_back(input_1);   //second input
                                    //...nth input

  //Construct previous locking script of input_0 & input_1
  script prev_script_0 = script::to_pay_key_hash_pattern(my_address0.hash());
  script prev_script_1 = script::to_pay_key_hash_pattern(my_address1.hash());

  //TX signature for input_0
  endorsement sig_0;
  uint8_t input0_index(0u);
  script::create_endorsement(sig_0, my_secret0, prev_script_0, tx, input0_index, 0x02);

  //TX signature for input_1
  endorsement sig_1;
  uint8_t input1_index(1u);
  script::create_endorsement(sig_1, my_secret1, prev_script_1, tx, input1_index, 0x02);

  //Construct unlocking script_0
  operation::list sig_script_0;
  sig_script_0.push_back(operation(sig_0));
  sig_script_0.push_back(operation(to_chunk(pubkey0)));
  script my_unlocking_script_0(sig_script_0);

  //Construct unlocking script_1
  operation::list sig_script_1;
  sig_script_1.push_back(operation(sig_1));
  sig_script_1.push_back(operation(to_chunk(pubkey1)));
  script my_unlocking_script_1(sig_script_1);

  //Add unlockingscript to TX
  tx.inputs()[0].set_script(my_unlocking_script_0);
  tx.inputs()[1].set_script(my_unlocking_script_1);

  //NONE: We can modify all outputs after signing
  tx.outputs().push_back(output_0); //first output
  tx.outputs().push_back(output_1); //second output
                                    //...nth output

  //Print out
  std::cout << encode_base16(tx.to_data()) << std::endl;

}

void example_3() {
  //SETUP (IDENTICAL TO EXAMPLE_1)
  //Begin with a private key
  auto my_secret0 = base16_literal("3eec08386d08321cd7143859e9bf4d6f65a71d24f37536d76b4224fdea48009f");
  auto my_secret1 = base16_literal("86faa240ae2b0f28b125a42961bd3adf9d5f5dc6a1deaa5feda04e7be8c872f6");
  auto my_secret2 = base16_literal("b7423c94ab99d3295c1af7e7bbea47c75d298f7190ca2077b53bae61299b70a5");
  auto my_secret3 = base16_literal("d977e2ce0f744dc3432cde9813a99360a3f79f7c8035ef82310d54c57332b2cc");
  ec_private my_private0(my_secret0, ec_private::testnet, true); //compressed
  ec_private my_private1(my_secret1, ec_private::testnet, true); //compressed
  ec_private my_private2(my_secret2, ec_private::testnet, true); //compressed
  ec_private my_private3(my_secret3, ec_private::testnet, true); //compressed
  payment_address my_address0 = my_private0.to_payment_address();
  payment_address my_address1 = my_private1.to_payment_address();
  payment_address my_address2 = my_private2.to_payment_address();
  payment_address my_address3 = my_private3.to_payment_address();
  ec_compressed pubkey0= my_private0.to_public().point();
  ec_compressed pubkey1= my_private1.to_public().point();
  std::cout << my_address0.encoded() << std::endl;
  std::cout << my_address1.encoded() << std::endl;
  std::cout << my_address2.encoded() << std::endl;
  std::cout << my_address3.encoded() << std::endl;

  //version
  uint32_t version = 1u;
  transaction tx;
  tx.set_version(version);

  //build input 0
  std::string prev_tx_string_0 = "3af74abff61f5a8486da92a8fc5e31dc10b899862bf3468d0505a5c1ca550b52";
  hash_digest prev_tx_hash_0;
  decode_hash(prev_tx_hash_0,prev_tx_string_0);
  //prev uxto index
  uint32_t index0 = 0;
  output_point uxto_tospend_0(prev_tx_hash_0, index0);
  //input object
  input input_0;
  input_0.set_previous_output(uxto_tospend_0);   //sequence is set in input then...
  input_0.set_sequence(0xffffffff); //standard, no locktime
  //build input 1
  std::string prev_tx_string_1 = "de065530166c16ab1c820b2f4e8e70d8b4259de263d18e47efde41f30ec79970";
  hash_digest prev_tx_hash_1;
  decode_hash(prev_tx_hash_1,prev_tx_string_1);
  //prev uxto index
  uint32_t index1 = 0;
  output_point uxto_tospend_1(prev_tx_hash_1, index1);
  //input object
  input input_1;
  input_1.set_previous_output(uxto_tospend_1);   //sequence is set in input then...
  input_1.set_sequence(0xffffffff); //standard, no locktime

  //build output 0
  operation::list locking_script_0=script::to_pay_key_hash_pattern(my_address2.hash());
  std::string btc_amount_string_0 = "0.041";
  uint64_t satoshi_amount_0;
  decode_base10(satoshi_amount_0, btc_amount_string_0, btc_decimal_places); //8 decimals = btc_decimal_places
  output output_0(satoshi_amount_0, locking_script_0); //1st arg satoshis, 2nd arg output script
  //build output 1
  operation::list locking_script_1=script::to_pay_key_hash_pattern(my_address3.hash());
  std::string btc_amount_string_1 = "0.03";
  uint64_t satoshi_amount_1;
  decode_base10(satoshi_amount_1, btc_amount_string_1, btc_decimal_places); //8 decimals = btc_decimal_places
  output output_1(satoshi_amount_1, locking_script_1); //1st arg satoshis, 2nd arg output script
  //build output 2
  operation::list locking_script_2=script::to_pay_key_hash_pattern(my_address3.hash());
  std::string btc_amount_string_2 = "0.03";
  uint64_t satoshi_amount_2;
  decode_base10(satoshi_amount_2, btc_amount_string_2, btc_decimal_places); //8 decimals = btc_decimal_places
  output output_2(satoshi_amount_2, locking_script_2); //1st arg satoshis, 2nd arg output script
  //build locktime


  //****************************************//
  //SIGHASH SINGLE example

  //Not shown:
  //Build inputs (w/o scriptSig)
  //Build outputs

  //We sign all inputs and single output with same index
  tx.inputs().push_back(input_0);   //first input
  tx.outputs().push_back(output_0); //first output
  tx.inputs().push_back(input_1);   //second input
  tx.outputs().push_back(output_1); //second output
                                    //...nth input
                                    //...nth output

  //Construct previous locking script of input_0 & input_1
  script prev_script_0 = script::to_pay_key_hash_pattern(my_address0.hash());
  script prev_script_1 = script::to_pay_key_hash_pattern(my_address1.hash());

  //TX signature for input_0
  endorsement sig_0;
  uint8_t input0_index(0u);
  script::create_endorsement(sig_0, my_secret0, prev_script_0, tx, input0_index, 0x03);

  //TX signature for input_1
  endorsement sig_1;
  uint8_t input1_index(1u);
  script::create_endorsement(sig_1, my_secret1, prev_script_1, tx, input1_index, 0x03);

  //Construct unlocking script_0
  operation::list sig_script_0;
  sig_script_0.push_back(operation(sig_0));
  sig_script_0.push_back(operation(to_chunk(pubkey0)));
  script my_unlocking_script_0(sig_script_0);

  //Construct unlocking script_1
  operation::list sig_script_1;
  sig_script_1.push_back(operation(sig_1));
  sig_script_1.push_back(operation(to_chunk(pubkey1)));
  script my_unlocking_script_1(sig_script_1);

  //Add unlockingscript to TX
  tx.inputs()[0].set_script(my_unlocking_script_0);
  tx.inputs()[1].set_script(my_unlocking_script_1);

  //SINGLE: We can add additional outputs after signing
  tx.outputs().push_back(output_2); //third output
                                    //...nth output

  //Print out
  std::cout << encode_base16(tx.to_data()) << std::endl;
}

void example_4() {
  //SETUP (IDENTICAL TO EXAMPLE_1)
  //Begin with a private key
  auto my_secret0 = base16_literal("3eec08386d08321cd7143859e9bf4d6f65a71d24f37536d76b4224fdea48009f");
  auto my_secret1 = base16_literal("86faa240ae2b0f28b125a42961bd3adf9d5f5dc6a1deaa5feda04e7be8c872f6");
  auto my_secret2 = base16_literal("b7423c94ab99d3295c1af7e7bbea47c75d298f7190ca2077b53bae61299b70a5");
  auto my_secret3 = base16_literal("d977e2ce0f744dc3432cde9813a99360a3f79f7c8035ef82310d54c57332b2cc");
  ec_private my_private0(my_secret0, ec_private::testnet, true); //compressed
  ec_private my_private1(my_secret1, ec_private::testnet, true); //compressed
  ec_private my_private2(my_secret2, ec_private::testnet, true); //compressed
  ec_private my_private3(my_secret3, ec_private::testnet, true); //compressed
  payment_address my_address0 = my_private0.to_payment_address();
  payment_address my_address1 = my_private1.to_payment_address();
  payment_address my_address2 = my_private2.to_payment_address();
  payment_address my_address3 = my_private3.to_payment_address();
  ec_compressed pubkey0= my_private0.to_public().point();
  ec_compressed pubkey1= my_private1.to_public().point();
  std::cout << my_address0.encoded() << std::endl;
  std::cout << my_address1.encoded() << std::endl;
  std::cout << my_address2.encoded() << std::endl;
  std::cout << my_address3.encoded() << std::endl;

  //version
  uint32_t version = 1u;
  transaction tx;
  tx.set_version(version);

  //build input 0
  std::string prev_tx_string_0 = "48828a16d0b93111272ec1721fceb29518efbd663c183e156873724ded5fe15d";
  hash_digest prev_tx_hash_0;
  decode_hash(prev_tx_hash_0,prev_tx_string_0);
  //prev uxto index
  uint32_t index0 = 0;
  output_point uxto_tospend_0(prev_tx_hash_0, index0);
  //input object
  input input_0;
  input_0.set_previous_output(uxto_tospend_0);   //sequence is set in input then...
  input_0.set_sequence(0xffffffff); //standard, no locktime
  //build input 1
  std::string prev_tx_string_1 = "e97db8ffd52711bbe012b7b1875e59106424dbfacd6938c688ca9535655c89ec";
  hash_digest prev_tx_hash_1;
  decode_hash(prev_tx_hash_1,prev_tx_string_1);
  //prev uxto index
  uint32_t index1 = 0;
  output_point uxto_tospend_1(prev_tx_hash_1, index1);
  //input object
  input input_1;
  input_1.set_previous_output(uxto_tospend_1);   //sequence is set in input then...
  input_1.set_sequence(0xffffffff); //standard, no locktime

  //build output 0
  operation::list locking_script_0=script::to_pay_key_hash_pattern(my_address2.hash());
  std::string btc_amount_string_0 = "0.03";
  uint64_t satoshi_amount_0;
  decode_base10(satoshi_amount_0, btc_amount_string_0, btc_decimal_places); //8 decimals = btc_decimal_places
  output output_0(satoshi_amount_0, locking_script_0); //1st arg satoshis, 2nd arg output script
  //build output 1
  operation::list locking_script_1=script::to_pay_key_hash_pattern(my_address3.hash());
  std::string btc_amount_string_1 = "0.02";
  uint64_t satoshi_amount_1;
  decode_base10(satoshi_amount_1, btc_amount_string_1, btc_decimal_places); //8 decimals = btc_decimal_places
  output output_1(satoshi_amount_1, locking_script_1); //1st arg satoshis, 2nd arg output script
  //build locktime

  //****************************************//

  //We create a TX2 to just for input_1 signing
  //TX remains empty for input_0 signing
  transaction tx2 = tx;
  //Create previous script
  script prev_script_1 = script::to_pay_key_hash_pattern(my_address1.hash());
  //Push input into TX2
  input empty_input;
  tx2.inputs().push_back(empty_input); //we need a placeholder, since signing at index 1
  tx2.inputs().push_back(input_1);
  //TX signature for input_1
  endorsement sig_1;
  uint8_t input1_index(1u);
  script::create_endorsement(sig_1, my_secret1, prev_script_1, tx2, input1_index, 0x82);
  //Construct unlocking script 1
  operation::list sig_script_1;
  sig_script_1.push_back(operation(sig_1));
  sig_script_1.push_back(operation(to_chunk(pubkey1)));
  script my_unlocking_script_1(sig_script_1);
  input_1.set_script(my_unlocking_script_1);


  //******************NONE|ANYONECANPAY**********************//

  //Not shown:
  //Build input (NONE,ANYONECANPAY) signed at index 1
  //Build outputs

  //We only sign a single input
  tx.inputs().push_back(input_0);

  //Construct previous locking script of input_0
  script prev_script_0 = script::to_pay_key_hash_pattern(my_address0.hash());

  //TX signature for input_0
  endorsement sig_0;
  uint8_t input0_index(0u);
  script::create_endorsement(sig_0, my_secret0, prev_script_0, tx, input0_index, 0x82);

  //Construct unlocking script_0
  operation::list sig_script_0;
  sig_script_0.push_back(operation(sig_0));
  sig_script_0.push_back(operation(to_chunk(pubkey0)));
  script my_unlocking_script_0(sig_script_0);

  //Add unlockingscript to TX
  tx.inputs()[0].set_script(my_unlocking_script_0);

  //ANYONECANPAY: We can modify other inputs after signing
  //Important: input added here must include valid scriptSig!
  //...and previously be signed with tx index = 1
  tx.inputs().push_back(input_1);   //second input
                                    //...nth input

  //NONE: We can modify all outputs after signing
  tx.outputs().push_back(output_0); //first output
  tx.outputs().push_back(output_1); //second output
                                    //...nth output

  //Print out
  std::cout << encode_base16(tx.to_data()) << std::endl;

}


int main() {

  std::cout << "Example 1: " << "\n";
  example_1();
  std::cout << "\n";

  std::cout << "Example 2: " << "\n";
  example_2();
  std::cout << "\n";

  std::cout << "Example 3: " << "\n";
  example_3();
  std::cout << "\n";

  std::cout << "Example 4: " << "\n";
  example_4();
  std::cout << "\n";

  return 0;

}
