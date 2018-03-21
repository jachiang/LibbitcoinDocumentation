# Segwit TX Examples

**P2WPKH**
* example_to_p2wpkh();
* example_from_p2wpkh();  

**P2SH(P2WPKH)**
* example_to_p2sh_p2wpkh();
* example_from_p2sh_p2wpkh();

**P2WSH**
* example_to_p2wsh();
* example_from_p2wsh();

**P2SH(P2WSH)**
* example_to_p2sh_p2wsh();
* example_from_p2sh_p2wsh();

```C++
#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>

using namespace bc;
using namespace wallet;
using namespace chain;
using namespace machine;

// "Normal" Wallets
auto my_secret0 = base16_literal("b7423c94ab99d3295c1af7e7bbea47c75d298f7190ca2077b53bae61299b70a5");
ec_private my_private0(my_secret0, ec_private::testnet, true);
ec_compressed pubkey0= my_private0.to_public().point();
payment_address my_address0 = my_private0.to_payment_address();

auto my_secret1 = base16_literal("d977e2ce0f744dc3432cde9813a99360a3f79f7c8035ef82310d54c57332b2cc");
ec_private my_private1(my_secret1, ec_private::testnet, true);
ec_compressed pubkey1= my_private1.to_public().point();

// "Segwit Aware" Wallets
auto my_secret_segwit = base16_literal("0a44957babaa5fd46c0d921b236c50b1369519c7032df7906a18a31bb905cfdf");
ec_private my_private_segwit(my_secret_segwit, ec_private::testnet, true);
ec_compressed pubkey_segwit= my_private_segwit.to_public().point();

auto my_secret_segwit1 = base16_literal("2361b894dab5c45c3c448eb4ab65f847cb3000e05969f18c94b8850233a95b74");
ec_private my_private_segwit1(my_secret_segwit1, ec_private::testnet, true);
ec_compressed pubkey_segwit1= my_private_segwit1.to_public().point();

auto my_secret_segwit2 = base16_literal("87493c67155f44a9a9a6abf621926a407121d6f4e1e94c75ced61208d7abe9db");
ec_private my_private_segwit2(my_secret_segwit2, ec_private::testnet, true);
ec_compressed pubkey_segwit2= my_private_segwit2.to_public().point();


void example_to_p2wpkh() {

  //**************************************************************
  // SEND TO P2WPKH
  //**************************************************************

  // ScriptPubKey:
  // 	  0 [20-byte hash160(pubKey)]
  // ScriptSig:
  //    According to previous output
  // scriptCode:
  // 	  According to previous scriptPubKey

  // P2PKH Input
  // Previous TX hash
  std::string prev_tx_string = "44101b50393d01de1e113b17eb07e8a09fbf6334e2012575bc97da227958a7a5";
  hash_digest prev_tx_hash;
  decode_hash(prev_tx_hash,prev_tx_string);
  // Previous UXTO index:
  uint32_t index = 0;
  output_point uxto_tospend(prev_tx_hash, index);
  // Build input_0 object
  input input_0;
  input_0.set_previous_output(uxto_tospend);
  input_0.set_sequence(0xffffffff);
  // Previous scriptPubKey Script:
  script prev_p2pkh_scriptpubkey = script::to_pay_key_hash_pattern(bitcoin_short_hash(pubkey0));

  // P2WPKH ScriptPubKey
  // 0 [20-byte hash160(pubKey)]
  operation::list p2wpkh_oplist;
  p2wpkh_oplist.push_back(operation(opcode::push_size_0));
  p2wpkh_oplist.push_back(operation(to_chunk(bitcoin_short_hash(pubkey_segwit))));
  // Build P2WPKH Output
  std::string btc_amount_string = "0.995";
  uint64_t satoshi_amount;
  decode_base10(satoshi_amount, btc_amount_string, btc_decimal_places);
  output p2wpkh_output(satoshi_amount, p2wpkh_oplist);

  // Build & Sign TX
  transaction tx;
  tx.set_version(1u);
  tx.inputs().push_back(input_0);
  tx.outputs().push_back(p2wpkh_output);

  // Signature of input 0
  endorsement sig_0;
  uint8_t input0_index(0u);
  script::create_endorsement(sig_0, my_secret0, prev_p2pkh_scriptpubkey, tx, input0_index, 0x01);

  // ScriptSig
  operation::list p2pkh_oplist;
  p2pkh_oplist.push_back(operation(sig_0));
  p2pkh_oplist.push_back(operation(to_chunk(pubkey0)));
  script p2pkh_sig_script(p2pkh_oplist);

  // Complete TX
  tx.inputs()[0].set_script(p2pkh_sig_script);
  std::cout << encode_base16(tx.to_data()) << std::endl;

}


void example_from_p2wpkh() {

  //**************************************************************
  // SEND FROM P2WPKH
  //**************************************************************

  // ScriptPubKey:
  //    According to destination address
  // ScriptSig:
  //    []
  // ScriptCode:
  //    P2PKH(publicKeyHash)
  // Witness:
  //    [signature] [publicKey]

  // P2WPKH Output
  // 0 [20-byte hash160(pubKey)]
  operation::list p2wpkh_oplist;
  p2wpkh_oplist.push_back(operation(opcode::push_size_0));
  p2wpkh_oplist.push_back(operation(to_chunk(bitcoin_short_hash(pubkey1))));
  // Build Output
  std::string btc_amount_string = "0.993";
  uint64_t satoshi_amount;
  decode_base10(satoshi_amount, btc_amount_string, btc_decimal_places);
  output p2wpkh_output(satoshi_amount, p2wpkh_oplist);

  // P2WPKH Input
  // Previous TX hash
  std::string prev_tx_string = "26c9768cdbb00332ff1052f27e71eb7e82b578bf02fb6d7eecfd0b43412e9d10";
  hash_digest prev_tx_hash;
  decode_hash(prev_tx_hash,prev_tx_string);
  // Previous UXTO index:
  uint32_t index = 0;
  output_point uxto_tospend(prev_tx_hash, index);
  // Build input_0 object
  input p2wpkh_input;
  p2wpkh_input.set_previous_output(uxto_tospend);
  p2wpkh_input.set_sequence(0xffffffff);

  // Build TX
  transaction tx;
  tx.set_version(1u);
  tx.inputs().push_back(p2wpkh_input);
  tx.outputs().push_back(p2wpkh_output);

  // Create Segwit Signature
  // ScriptCode:
  script p2wpkh_script_code = script::to_pay_key_hash_pattern(bitcoin_short_hash(pubkey_segwit));
  // Previous Input Amount:
  uint8_t input_index(0u);
  std::string prev_btc_amount_string = "0.995";
  uint64_t prev_satoshi_amount;
  decode_base10(prev_satoshi_amount, prev_btc_amount_string, btc_decimal_places);
  // Segwit Endorsement: Pass script_version::zero & prev input amount
  endorsement sig_1;
  script::create_endorsement(sig_1, my_secret_segwit, p2wpkh_script_code, tx, input_index, 0x01, script_version::zero, prev_satoshi_amount);

  // Create Witness
  // 02 [signature] [publicKey]
  data_stack witness_stack;
  witness_stack.push_back(sig_1);
  witness_stack.push_back(to_chunk(pubkey_segwit));
  witness p2wpkh_witness(witness_stack);
  tx.inputs()[0].set_witness(p2wpkh_witness);

  // Serialize TX
  std::cout << encode_base16(tx.to_data(true,true)) << std::endl;

}


void example_to_p2sh_p2wpkh() {

  //**************************************************************
  // SEND to P2SH(P2WPKH)
  //**************************************************************

  // ScriptPubKey:
  //    0 [20-byte hash160(redeemscript)]
  // ScriptSig:
  //    According to previous output
  // ScriptCode:
  //    According to previous scriptPubKey

  // P2PKH Input
  // Previous TX hash
  std::string prev_tx_string = "e93e1e7aafb38269512592fa2225c5783f0e589ec5359a4602eac11033d5e09d";
  hash_digest prev_tx_hash;
  decode_hash(prev_tx_hash,prev_tx_string);
  // Previous UXTO index:
  uint32_t index = 0;
  output_point uxto_tospend(prev_tx_hash, index);
  // Build input_0 object
  input input_0;
  input_0.set_previous_output(uxto_tospend);
  input_0.set_sequence(0xffffffff);
  // Previous Locking Script:
  script prev_script_pubkey = script::to_pay_key_hash_pattern(bitcoin_short_hash(pubkey0));

  // P2SH(P2WPKH) Output
  // P2SH Redeemscript = P2WPKH(PubKeyHash)
  //    0 [20-byte publicKeyHash]
  short_hash keyhash_dest = bitcoin_short_hash(pubkey_segwit);
  operation::list p2wpkh_oplist;
  p2wpkh_oplist.push_back(operation(opcode::push_size_0));
  p2wpkh_oplist.push_back(operation(to_chunk(keyhash_dest)));
  script p2wpkh_redeemscript(p2wpkh_oplist);
  // P2SH ScriptPubKey:
  // hash160 [20-byte hash160(redeemscript)] equal
  short_hash redeemscript_hash = bitcoin_short_hash(p2wpkh_redeemscript.to_data(false));
  script script_pubkey = script::to_pay_script_hash_pattern(redeemscript_hash);
  // Build output:
  std::string btc_amount_string = "1.298";
  uint64_t satoshi_amount;
  decode_base10(satoshi_amount, btc_amount_string, btc_decimal_places);
  output p2sh_p2wpkh_output(satoshi_amount, script_pubkey);

  // Build TX
  transaction tx;
  tx.inputs().push_back(input_0);
  tx.outputs().push_back(p2sh_p2wpkh_output);
  tx.set_version(1u);

  // Build endorsement
  endorsement sig_0;
  uint8_t input_index(0u);
  script::create_endorsement(sig_0, my_secret0, prev_script_pubkey, tx, input_index, 0x01);

  // Build ScriptSig
  operation::list script_sig_oplist;
  script_sig_oplist.push_back(sig_0);
  script_sig_oplist.push_back(operation(to_chunk(pubkey0)));
  script script_sig(script_sig_oplist);
  tx.inputs()[0].set_script(script_sig);

  // Serialize TX
  std::cout << encode_base16(tx.to_data()) << std::endl;
}


void example_from_p2sh_p2wpkh() {

  //**************************************************************
  // SEND from P2SH(P2WPKH)
  //**************************************************************

  // ScriptPubKey:
  //    According to destination address
  // ScriptSig: [p2sh Redeemscript]
  //    [0 <20-byte publicKeyHash>]
  // ScriptCode:
  //    P2PKH(hash160(pubkey))
  // Witness:
  //    [signature] [publicKey]

  // P2SH(P2WPKH) Input
  // Previous TX hash
  std::string prev_tx_string = "8231a9027eca6f2bd7bdf712cd2368f0b6e8dd6005b6b348078938042178ffed";
  hash_digest prev_tx_hash;
  decode_hash(prev_tx_hash,prev_tx_string);
  // Previous UXTO index:
  uint32_t index = 0;
  output_point uxto_tospend(prev_tx_hash, index);
  // Build P2SH(P2WPKH) input object
  input p2sh_p2wpkh_input;
  p2sh_p2wpkh_input.set_previous_output(uxto_tospend);
  p2sh_p2wpkh_input.set_sequence(0xffffffff);

  // Build output:
  std::string btc_amount_string = "1.295";
  uint64_t satoshi_amount;
  decode_base10(satoshi_amount, btc_amount_string, btc_decimal_places);
  script script_pubkey = script::to_pay_key_hash_pattern(bitcoin_short_hash(pubkey1));
  output p2pkh_output(satoshi_amount, script_pubkey);

  // Build TX
  transaction tx;
  tx.set_version(1u);
  tx.inputs().push_back(p2sh_p2wpkh_input);
  tx.outputs().push_back(p2pkh_output);

  // Create Segwit Signature
  // ScriptCode:
  script script_code = script::to_pay_key_hash_pattern(bitcoin_short_hash(pubkey_segwit));
  // Previous Input Amount
  uint8_t input_index(0u);
  std::string btc_amount_string_in = "1.298";
  uint64_t satoshi_amount_in;
  decode_base10(satoshi_amount_in, btc_amount_string_in, btc_decimal_places); // btc_decimal_places = 8
  // Segwit Endorsement: Pass script_version::zero & prev input amount
  endorsement sig;
  script::create_endorsement(sig, my_secret_segwit, script_code, tx, input_index, 0x01, script_version::zero, satoshi_amount_in); //turn on witness

  // ScriptSig:
  // Redeemscript = P2WPKH script
  short_hash keyhash_dest = bitcoin_short_hash(pubkey_segwit);
  operation::list p2wpkh_oplist;
  p2wpkh_oplist.push_back(operation(opcode::push_size_0));
  p2wpkh_oplist.push_back(operation(to_chunk(keyhash_dest)));
  script p2wpkh_redeemscript(p2wpkh_oplist);

  // Wrap (P2SH) redeemscript in single single data push
  data_chunk p2sh_redeemscript_chunk = to_chunk(p2wpkh_redeemscript.to_data(true)); // true: include size
  script p2sh_redeemscript_wrapper(p2sh_redeemscript_chunk, false); // false: interpret as single data push
  tx.inputs()[0].set_script(p2sh_redeemscript_wrapper);

  // Witness:
  // 02 [signature] [publicKey]
  data_stack witness_stack;
  witness_stack.push_back(sig);
  witness_stack.push_back(to_chunk(pubkey_segwit));
  witness p2wpkh_witness(witness_stack);
  tx.inputs()[0].set_witness(p2wpkh_witness);

  // Serialize TX
  std::cout << encode_base16(tx.to_data(true,true)) << std::endl;

}


void example_to_p2wsh() {

  //**************************************************************
  // SEND TO P2WSH
  //**************************************************************

  // ScriptPubKey:
  //    0 [32-byte sha256(WitnessScript)]
  // ScriptSig:
  //    According to previous output
  // ScriptCode:
  //    According to previous scriptPubKey

  // P2PKH Input
  // Previous TX hash
  std::string prev_tx_string = "77ac219bd5ae60e9ca48dd0bbc73f49ca886982749bb0368ed1011e07ae87dac";
  hash_digest prev_tx_hash;
  decode_hash(prev_tx_hash,prev_tx_string);
  // Previous UXTO:
  uint32_t index = 0;
  output_point uxto_tospend(prev_tx_hash, index);
  // Build input object
  input input_0;
  input_0.set_previous_output(uxto_tospend);
  input_0.set_sequence(0xffffffff);
  // Previous P2PKH scriptPubKey
  script prev_script_pubkey = script::to_pay_key_hash_pattern(
      bitcoin_short_hash(pubkey0));

  // WitnessScript: MultiSig
  uint8_t signatures(2u); //2 of 3
  point_list points;
  points.push_back(pubkey_segwit);
  points.push_back(pubkey_segwit1);
  points.push_back(pubkey_segwit2);
  script witness_script = script::to_pay_multisig_pattern(signatures, points);

  // P2WSH scriptPubKey:
  //    0 [32-byte sha256(WitnessScript)]
  hash_digest redeemscript_hash = sha256_hash(witness_script.to_data(false));
  operation::list p2sh_oplist;
  p2sh_oplist.push_back(operation(opcode::push_size_0));
  p2sh_oplist.push_back(operation(to_chunk(redeemscript_hash)));

  // Build Output
  std::string btc_amount_string = "1.295";
  uint64_t satoshi_amount;
  decode_base10(satoshi_amount, btc_amount_string, btc_decimal_places);
  output p2wsh_output(satoshi_amount, p2sh_oplist);

  // Build tx
  transaction tx;
  tx.set_version(1u);
  tx.inputs().push_back(input_0);
  tx.outputs().push_back(p2wsh_output);
  tx.set_locktime(0u); //not necessary

  // Sign Endorsement
  endorsement sig;
  uint8_t input0_index(0u);
  script::create_endorsement(sig, my_secret0, prev_script_pubkey, tx,
      input0_index, 0x01);

  // Create ScriptSig
  operation::list sig_script_0;
  sig_script_0.push_back(operation(sig));
  sig_script_0.push_back(operation(to_chunk(pubkey0)));
  tx.inputs()[0].set_script(sig_script_0);

  // Complete TX
  std::cout << encode_base16(tx.to_data()) << std::endl;

}


void example_from_p2wsh() {

  //**************************************************************
  // SEND FROM P2WSH
  //**************************************************************

  // ScriptPubKey:
  //    According to destination address
  // ScriptSig:
  //    ""
  // ScriptCode:
  //    Witness Script
  // Witness:
  //    [Signature0] [Signature1] [...] [WitnessScript]

  // Build p2pkh output:
  std::string btc_amount_string = "1.292";
  uint64_t satoshi_amount;
  decode_base10(satoshi_amount, btc_amount_string, btc_decimal_places);
  script script_pubkey = script::to_pay_key_hash_pattern(bitcoin_short_hash(pubkey1));
  output p2pkh_output(satoshi_amount, script_pubkey);

  // P2WSH Input
  // Previous TX hash
  std::string prev_tx_string = "98f85a8b774242979c9f0be37faba32d01c1e93fb69b4e6fa4b241b837dba293";
  hash_digest prev_tx_hash;
  decode_hash(prev_tx_hash,prev_tx_string);
  // Previous UXTO:
  uint32_t index = 0;
  output_point uxto_tospend(prev_tx_hash, index);
  // Build input object
  input p2wsh_input;
  p2wsh_input.set_previous_output(uxto_tospend);
  p2wsh_input.set_sequence(0xffffffff);

  // Build tx
  transaction tx;
  tx.set_version(1u);
  tx.inputs().push_back(p2wsh_input);
  tx.outputs().push_back(p2pkh_output);
  tx.set_locktime(0u);

  // ScriptCode: WitnessScript = e.g. MultiSig
  uint8_t signatures(2u); //2 of 3
  point_list points;
  points.push_back(pubkey_segwit);
  points.push_back(pubkey_segwit1);
  points.push_back(pubkey_segwit2);
  script witness_script = script::to_pay_multisig_pattern(signatures, points);

  // Sign
  endorsement sig0;
  endorsement sig1;
  uint8_t input0_index(0u);
  uint64_t satoshi_amount_in;
  std::string btc_amount_string_in = "1.295";
  decode_base10(satoshi_amount_in, btc_amount_string_in, btc_decimal_places);
  script::create_endorsement(sig0, my_secret_segwit, witness_script, tx,
      input0_index, 0x01, script_version::zero, satoshi_amount_in);
  script::create_endorsement(sig1, my_secret_segwit1, witness_script, tx,
      input0_index, 0x01, script_version::zero, satoshi_amount_in);

  // Create Witness
  data_stack witness_stack;
  data_chunk empty_chunk;
  witness_stack.push_back(empty_chunk);
  witness_stack.push_back(sig0);
  witness_stack.push_back(sig1);
  witness_stack.push_back(witness_script.to_data(false));
  witness p2wsh_witness(witness_stack);
  tx.inputs()[0].set_witness(p2wsh_witness);

  // Complete TX
  std::cout << encode_base16(tx.to_data(true,true)) << std::endl;

}


void example_to_p2sh_p2wsh() {

//**************************************************************
//SEND TO P2SH(P2WSH)
//**************************************************************

//ScriptPubKey:
//    hash160 [20-byte hash160(Redeemscript)] equal
//ScriptSig:
//    According to input
//ScriptCode:
//    According to prev ScriptPubKey

//P2PKH Input
//Previous TX hash
std::string prev_tx_string = "8e71c68c1e7388542bab54d3d0ded1f118fa8f49b240a8696488dce8d47593e5";
hash_digest prev_tx_hash;
decode_hash(prev_tx_hash,prev_tx_string);
//Previous UXTO index:
uint32_t index = 0;
output_point uxto_tospend(prev_tx_hash, index);
//Build input_0 object
input p2pkh_input;
p2pkh_input.set_previous_output(uxto_tospend);
p2pkh_input.set_sequence(0xffffffff);
//Previous Locking Script:
script prev_script_pubkey = script::to_pay_key_hash_pattern(bitcoin_short_hash(pubkey0));

// P2SH(P2WSH(MultSig)) Output
// 2-of-3 MultiSig WitnessScript & ScriptCode:
uint8_t signatures(2u); //2 of 3
point_list points;
points.push_back(pubkey_segwit);
points.push_back(pubkey_segwit1);
points.push_back(pubkey_segwit2);
script witness_script = script::to_pay_multisig_pattern(signatures, points);
// P2WSH(MultiSig) Redeemscript:
//    0 [34-byte sha256(WitnessScript)]
hash_digest witness_script_hash = sha256_hash(witness_script.to_data(false));
operation::list p2wsh_oplist;
p2wsh_oplist.push_back(operation(opcode::push_size_0));
p2wsh_oplist.push_back(operation(to_chunk(witness_script_hash)));
script p2wsh_script(p2wsh_oplist);
// P2SH(P2WSH) scriptPubKey:
//    hash160 [sha256(Redeemscript)] equal
short_hash redeemscript_hash = bitcoin_short_hash(p2wsh_script.to_data(false));
script script_pubkey = script::to_pay_script_hash_pattern(redeemscript_hash);
// Build output:
std::string btc_amount_string = "0.648";
uint64_t satoshi_amount;
decode_base10(satoshi_amount, btc_amount_string, btc_decimal_places);
output p2sh_p2wpkh_output(satoshi_amount, script_pubkey);

//Build TX
transaction tx;
tx.set_version(1u);
tx.inputs().push_back(p2pkh_input);
tx.outputs().push_back(p2sh_p2wpkh_output);
tx.set_locktime(0u);

//Sign
endorsement sig;
uint8_t input0_index(0u);
script::create_endorsement(sig, my_secret0, prev_script_pubkey, tx, input0_index, 0x01);

//Create ScriptSig
operation::list sig_script_0;
sig_script_0.push_back(operation(sig));
sig_script_0.push_back(operation(to_chunk(pubkey0)));

//Complete TX
tx.inputs()[0].set_script(sig_script_0);
std::cout << encode_base16(tx.to_data()) << std::endl;

}


void example_from_p2sh_p2wsh() {

  //**************************************************************
  //SEND FROM P2SH(P2WSH)
  //**************************************************************

  //ScriptPubKey:
  //    According to destination address
  //ScriptSig:
  //    [0 [34-byte sha256(WitnessScript)]]
  //ScriptCode:
  //    Witness Script
  //Witness:
  //    [Signature0] [Signature1] [...] [WitnessScript]

  //P2WPKH Output
  //0 [20-byte hash160(pubKey)]
  operation::list p2wpkh_oplist;
  p2wpkh_oplist.push_back(operation(opcode::push_size_0));
  p2wpkh_oplist.push_back(operation(to_chunk(bitcoin_short_hash(pubkey1))));
  //Build Output
  std::string btc_amount_string = "0.647";
  uint64_t satoshi_amount;
  decode_base10(satoshi_amount, btc_amount_string, btc_decimal_places); // btc_decimal_places = 8
  output p2wpkh_output(satoshi_amount, p2wpkh_oplist);

  // P2SH(P2WSH(MultiSig)) Input
  // Previous TX hash
  std::string prev_tx_string = "40d5ecc46fb5f99971cda1fcd1bc0d465b0ce3f80176f30ef0b36f89c5568270";
  hash_digest prev_tx_hash;
  decode_hash(prev_tx_hash,prev_tx_string);
  // Previous UXTO:
  uint32_t index = 0;
  output_point uxto_tospend(prev_tx_hash, index);
  // Build input object
  input p2sh_p2wsh_input;
  p2sh_p2wsh_input.set_previous_output(uxto_tospend);
  p2sh_p2wsh_input.set_sequence(0xffffffff);

  //Build TX
  transaction tx;
  tx.set_version(1u);
  tx.inputs().push_back(p2sh_p2wsh_input);
  tx.outputs().push_back(p2wpkh_output);
  tx.set_locktime(0u);
  // //P2SH(P2WSH) scriptPubKey:
  // //    hash160 [sha256(Redeemscript)] equal
  // short_hash redeemscript_hash = bitcoin_short_hash(p2wsh_script.to_data(false)); //script has to_data method
  // script script_pubkey = script::to_pay_script_hash_pattern(redeemscript_hash);

  //WitnessScript: MultiSig = ScriptCode
  uint8_t signatures(2u); //2 of 3
  point_list points;
  points.push_back(pubkey_segwit);
  points.push_back(pubkey_segwit1);
  points.push_back(pubkey_segwit2);
  script witness_script = script::to_pay_multisig_pattern(signatures, points);

  // Create Signatures
  endorsement sig0;
  endorsement sig1;
  uint8_t input0_index(0u);
  std::string btc_amount_string_in = "0.648";
  uint64_t satoshi_amount_in;
  decode_base10(satoshi_amount_in, btc_amount_string_in, btc_decimal_places);
  script::create_endorsement(sig0, my_secret_segwit, witness_script, tx,
        input0_index, 0x01, script_version::zero, satoshi_amount_in);
  script::create_endorsement(sig1, my_secret_segwit1, witness_script, tx,
        input0_index, 0x01, script_version::zero, satoshi_amount_in);

  //P2SH scriptPubKey:
  //    0 <34-byte sha256(WitnessScript)>
  hash_digest witness_script_hash = sha256_hash(witness_script.to_data(false));
  operation::list p2wsh_oplist;
  p2wsh_oplist.push_back(operation(opcode::push_size_0));
  p2wsh_oplist.push_back(operation(to_chunk(witness_script_hash)));
  script p2wsh_script(p2wsh_oplist);

  //ScriptSig
  //Wrap redeemscript as single data push (P2SH)
  data_chunk p2sh_redeemscript_chunk = to_chunk(p2wsh_script.to_data(true)); //true: include size prefix
  script p2sh_redeemscript_wrapper(p2sh_redeemscript_chunk, false); //false: interpret as single data push
  tx.inputs()[0].set_script(p2sh_redeemscript_wrapper);

  //Create Witness
  data_stack witness_stack;
  data_chunk empty_chunk;
  witness_stack.push_back(empty_chunk); //works
  witness_stack.push_back(sig0);
  witness_stack.push_back(sig1);
  witness_stack.push_back(witness_script.to_data(false));
  witness p2wsh_witness(witness_stack);
  tx.inputs()[0].set_witness(p2wsh_witness);

  //Complete TX
  std::cout << encode_base16(tx.to_data(true,true)) << std::endl;

}


int main() {

  example_to_p2wpkh();
  example_from_p2wpkh();

  example_to_p2sh_p2wpkh();
  example_from_p2sh_p2wpkh();

  example_to_p2wsh();
  example_from_p2wsh();

  example_to_p2sh_p2wsh();
  example_from_p2sh_p2wsh();

  return 0;

}
```
