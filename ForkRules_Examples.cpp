#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>

using namespace bc;
using namespace wallet;
using namespace chain;
using namespace machine;

// Testnet wallets.
auto my_secret0 = base16_literal(
    "b7423c94ab99d3295c1af7e7bbea47c75d298f7190ca2077b53bae61299b70a5");
ec_private my_private0(my_secret0, ec_private::testnet, true);
auto pubkey0 = my_private0.to_public().point();
auto my_address0 = my_private0.to_payment_address();

auto my_secret1 = base16_literal(
    "d977e2ce0f744dc3432cde9813a99360a3f79f7c8035ef82310d54c57332b2cc");
ec_private my_private1(my_secret1, ec_private::testnet, true);
auto pubkey1 = my_private1.to_public().point();

// Witness aware testnet wallets.
// ("Witness aware" is an arbitrary assignment for illustration purposes)
auto my_secret_witness_aware = base16_literal(
    "0a44957babaa5fd46c0d921b236c50b1369519c7032df7906a18a31bb905cfdf");
ec_private my_private_witness_aware(my_secret_witness_aware,
    ec_private::testnet, true);
auto pubkey_witness_aware = my_private_witness_aware
    .to_public().point();


transaction create_example_transaction() {

  // Function creates single input tx template for all subsequent examples.
  //---------------------------------------------------------------------------

  // Destination output, a p2pkh script for example.
  auto btc_amount = "0.998";
  uint64_t output_amount;
  decode_base10(output_amount, btc_amount, btc_decimal_places);
  auto p2pkh_script = script::to_pay_key_hash_pattern(
      bitcoin_short_hash(pubkey1));
  output p2pkh_output(output_amount, p2pkh_script);

  // Build example input.
  input example_input;
  auto prev_tx =
      "44101b50393d01de1e113b17eb07e8a09fbf6334e2012575bc97da227958a7a5";
  hash_digest prev_tx_hash;
  decode_hash(prev_tx_hash,prev_tx);
  uint32_t index = 0;
  output_point uxto_to_spend(prev_tx_hash, index);
  example_input.set_previous_output(uxto_to_spend);
  example_input.set_sequence(max_input_sequence);

  // Build transaction.
  transaction tx;
  tx.set_version(1u);
  tx.inputs().push_back(example_input);
  tx.outputs().push_back(p2pkh_output);
  tx.set_locktime(0u);

  return tx;

}


void create_and_verify_p2sh(const transaction& example_transaction) {

  // Create local example_transaction copy.
  auto p2sh_transaction = example_transaction;

  // Previous output script / Previous output amount.
  //---------------------------------------------------------------------------

  // Previous output script = P2SH(2-of-2 Multisig).

  // 2-of-2 Multisig.
  uint8_t signatures(2u);
  point_list points;
  points.push_back(pubkey0);
  points.push_back(pubkey1);
  script multisig_script = script::to_pay_multisig_pattern(signatures, points);

  // P2SH(2-of-2 Multisig) script.
  auto multisig_script_hash = bitcoin_short_hash(
      multisig_script.to_data(false));
  auto p2sh_multisig_script = script::to_pay_script_hash_pattern(
      multisig_script_hash);

  // Previous output amount.
  auto previous_btc_amount = "1.0";
  uint64_t previous_output_amount;
  decode_base10(previous_output_amount, previous_btc_amount, btc_decimal_places);

  // Input Scripts.
  //---------------------------------------------------------------------------

  // Create Endorsements.
  endorsement sig0;
  endorsement sig1;
  uint8_t input0_index(0u);
  script::create_endorsement(sig0, my_secret0, multisig_script,
      p2sh_transaction, input0_index, sighash_algorithm::all);
  script::create_endorsement(sig1, my_secret1, multisig_script,
      p2sh_transaction, input0_index, sighash_algorithm::all);

  // Create input script w/o signatures (no BIP16).
  operation::list unsigned_input_ops;
  unsigned_input_ops.push_back(operation(multisig_script.to_data(false)));
  script unsigned_input_script(unsigned_input_ops);

  // Create input script /w signatures (BIP16).
  operation::list signed_input_ops {
      operation(opcode::push_size_0),
      operation(sig0),
      operation(sig1),
      operation(multisig_script.to_data(false))

  };
  script signed_input_script(signed_input_ops);

  // Script Verification w/o BIP16 activation.
  //---------------------------------------------------------------------------

  // Add input script to transaction.
  p2sh_transaction.inputs()[input0_index].set_script(unsigned_input_script);

  // Turn off BIP16 soft fork.
  // Note: all rules includes testnet and regtest.
  auto my_fork_rules = rule_fork::all_rules ^ rule_fork::bip16_rule;

  // Verify w/o signatures.
  witness empty_witness;
  auto ec = script::verify(p2sh_transaction, input0_index, my_fork_rules,
      unsigned_input_script, empty_witness, p2sh_multisig_script,
      previous_output_amount);

  // Prints success
  std::cout << ec.message() << std::endl;

  // Script verification with BIP16 activation.
  //---------------------------------------------------------------------------

  // Add input script to transaction.
  p2sh_transaction.inputs()[input0_index].set_script(signed_input_script);

  // BIP16 is active.
  // Note: all rules includes testnet and regtest.
  my_fork_rules = rule_fork::all_rules;

  // Input script also works without BIP16 activation.
  // my_fork_rules = rule_fork::all_rules ^ rule_fork::bip16_rule;

  // Verify with signatures.
  ec = script::verify(p2sh_transaction, input0_index, my_fork_rules,
      signed_input_script, empty_witness, p2sh_multisig_script,
      previous_output_amount);

  // Prints success
  std::cout << ec.message() << std::endl;

}


void create_and_verify_p2wpkh(const transaction& example_transaction) {

  // Create local example_transaction copy.
  transaction p2wpkh_transaction = example_transaction;

  // Previous output script / Previous output amount.
  //---------------------------------------------------------------------------

  // Previous P2WPKH.
  // 0 [20-byte hash160(public key)]
  operation::list p2wpkh_operations {
      operation(opcode::push_size_0),
      to_chunk(bitcoin_short_hash(pubkey_witness_aware))
  };

  // Previous output amount.
  uint8_t input_index(0u);
  auto previous_btc_amount = "1.0";
  uint64_t previous_output_amount;
  decode_base10(previous_output_amount, previous_btc_amount,btc_decimal_places);

  // Script Verification w/o BIP141/143 activation.
  //---------------------------------------------------------------------------

  // Deactivate BIP141/143.
  // Note: all rules includes testnet and regtest.
  auto my_fork_rules = rule_fork::all_rules ^ rule_fork::bip141_rule
      ^ rule_fork::bip143_rule;

  witness empty_witness;
  script empty_input_script;

  auto ec = script::verify(p2wpkh_transaction, input0_index, my_fork_rules,
      empty_input_script, empty_witness, p2wpkh_operations,
      previous_output_amount);

  // Prints success
  std::cout << ec.message() << std::endl;

  // Create Witness.
  //---------------------------------------------------------------------------

  // Create signature and witness.
  // Script code.
  script p2wpkh_script_code = script::to_pay_key_hash_pattern(
      bitcoin_short_hash(pubkey_witness_aware));

  // Pass script_version::zero and previous output amount.
  endorsement sig0;
  script::create_endorsement(sig0, my_secret_witness_aware, p2wpkh_script_code,
      p2wpkh_transaction, input_index, sighash_algorithm::all,
      script_version::zero, previous_output_amount);

  // Create witness.
  // 02 [signature] [public key]
  data_stack witness_stack {
      sig0,
      to_chunk(pubkey_witness_aware)
  };
  witness p2wpkh_witness(witness_stack);

  // Script Verification with BIP141/143 activation.
  //---------------------------------------------------------------------------

  my_fork_rules = rule_fork::all_rules;
  // Without bip141: error code 77, unexpected witness.
  // Without bip143: error code 65, stack false...
  //    ...since BIP143 signature serialisation not activated.

  // Add witness to transaction.
  p2wpkh_transaction.inputs()[input0_index].set_witness(p2wpkh_witness);

  ec = script::verify(p2wpkh_transaction, input0_index, my_fork_rules,
      empty_input_script, p2wpkh_witness, p2wpkh_operations,
      previous_output_amount);

  // Prints success
  std::cout << ec.message() << std::endl;

}

int main() {

  auto tx = create_example_transaction();

  create_and_verify_p2sh(tx);

  create_and_verify_p2wpkh(tx);

  return 0;

}
