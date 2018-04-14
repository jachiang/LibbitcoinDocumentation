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


transaction create_example_transaction() {

  // Function creates single input tx template for all subsequent examples.
  //---------------------------------------------------------------------------

  // Destination output, a p2pkh script for example.
  std::string btc_amount = "0.998";
  uint64_t output_amount;
  decode_base10(output_amount, btc_amount, btc_decimal_places);
  auto p2pkh_script = script::to_pay_key_hash_pattern(
      bitcoin_short_hash(pubkey1));
  output p2pkh_output(output_amount, p2pkh_script);

  // Build example input.
  input example_input;
  std::string prev_tx =
      "44101b50393d01de1e113b17eb07e8a09fbf6334e2012575bc97da227958a7a5";
  hash_digest prev_tx_hash;
  decode_hash(prev_tx_hash,prev_tx);
  uint32_t index = 0;
  output_point uxto_to_spend(prev_tx_hash, index);
  example_input.set_previous_output(uxto_to_spend);
  example_input.set_sequence(max_input_sequence);

  // Build Transaction
  transaction tx;
  tx.set_version(1u);
  tx.inputs().push_back(example_input);
  tx.outputs().push_back(p2pkh_output);
  tx.set_locktime(0u);

  return tx;

}


void create_and_verify_p2pkh(const transaction& example_transaction)
{

  // Assumes relevant input script located at index 0.

  // Create local example_transaction copy.
  auto p2pkh_transaction = example_transaction;

  // Previous output script / Previous output amount.
  //---------------------------------------------------------------------------

  // Previous output script: P2PKH.
  auto p2pkh_output_script = script::to_pay_key_hash_pattern(
        bitcoin_short_hash(pubkey0));

  // Previous output amount.
  std::string previous_btc_amount = "1.0";
  uint64_t previous_output_amount;
  decode_base10(previous_output_amount, previous_btc_amount, btc_decimal_places);

  // Input script.
  //---------------------------------------------------------------------------

  // Signature.
  endorsement sig_0;
  uint8_t input0_index(0u);
  script::create_endorsement(sig_0, my_secret0, p2pkh_output_script,
      p2pkh_transaction, input0_index, sighash_algorithm::all);

  // Input script operations.
  operation::list input_operations {
      operation(sig_0),
      operation(to_chunk(pubkey0))
  };
  script p2pkh_input_script(input_operations);

  // Add input script to transaction.
  p2pkh_transaction.inputs()[input0_index].set_script(p2pkh_input_script);

  // Verify input script, output script.
  //---------------------------------------------------------------------------

  // With all fork rules.
  // Note: all rules includes testnet and regtest.
  witness empty_witness;
  auto ec = script::verify(p2pkh_transaction, input0_index,rule_fork::all_rules,
      p2pkh_input_script, empty_witness, p2pkh_output_script,
      previous_output_amount);

  // Libbitcoin  version4: Changes to script::verify().
  // Input script and witness parameters are moved into tx.
  // auto ec1 = script::verify(p2pkh_transaction, input0_index,
  //     rule_fork::all_rules, p2pkh_output_script, previous_output_amount);

  // Libbitcoin version4: Alternative script::verify() signature.
  // Prevout script and amount can be moved into tx metadata.
  // p2pkh_transaction.inputs()[input0_index]
  //     .previous_output().metadata.cache.set_script(p2pkh_output_script);
  // p2pkh_transaction.inputs()[input0_index]
  //     .previous_output().metadata.cache.set_value(previous_output_amount);
  // auto ec2 = script::verify(p2pkh_transaction, input0_index,
  //     rule_fork::all_rules);

  // Prints success
  std::cout << ec.message() << std::endl;

}

// TO DO: Other Examples

int main() {

  auto tx = create_example_transaction();

  create_and_verify_p2pkh(tx);

  return 0;

}
