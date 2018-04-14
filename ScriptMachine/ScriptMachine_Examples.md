# Examples: Script Machine

All examples from the script machine documentation chapter are shown here in full.

**Example Script Debugger**
* debug_program();

**Example Simple Script Evaluation**
* run_custom_script();

**Example P2SH(P2WPKH) Evaluation**
* run_p2sh_p2wpkh();

Compile with:
`g++ -std=c++11 -o script_machine script_machine_examples.cpp $(pkg-config --cflags libbitcoin --libs libbitcoin)`

```c++
#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>

using namespace bc;
using namespace wallet;
using namespace chain;
using namespace machine;

// "Normal" wallets.
auto my_secret0 = base16_literal(
    "b7423c94ab99d3295c1af7e7bbea47c75d298f7190ca2077b53bae61299b70a5");
ec_private my_private0(my_secret0, ec_private::testnet, true);
auto pubkey0 = my_private0.to_public().point();
auto my_address0 = my_private0.to_payment_address();

auto my_secret1 = base16_literal(
    "d977e2ce0f744dc3432cde9813a99360a3f79f7c8035ef82310d54c57332b2cc");
ec_private my_private1(my_secret1, ec_private::testnet, true);
auto pubkey1 = my_private1.to_public().point();

// "Witness Aware" wallet.
auto my_secret_witness_aware = base16_literal(
    "0a44957babaa5fd46c0d921b236c50b1369519c7032df7906a18a31bb905cfdf");
ec_private my_private_witness_aware(my_secret_witness_aware,
    ec_private::testnet, true);
auto pubkey_witness_aware = my_private_witness_aware
    .to_public().point();


transaction create_transaction_template() {

  // Function creates tx object as a tx template for all subsequent examples.
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


transaction create_p2sh_p2wpkh(transaction& transaction_template) {

    auto p2sh_p2wpkh_transaction = transaction_template;

    // Previous output script / Previous output amount.
    //--------------------------------------------------------------------------

    // P2SH(P2WPKH) output.

    //   P2WPKH(public key hash)
    //   0 [20-byte public key hash]
    short_hash keyhash_dest = bitcoin_short_hash(pubkey_witness_aware);
    operation::list p2wpkh_operations = {
        operation(opcode::push_size_0),
        operation(to_chunk(keyhash_dest))
    };
    script p2wpkh_script(p2wpkh_operations);

    //  P2SH(P2WPKH(public key hash))
    //  hash160 [20-byte hash160(redeem script)] equal
    short_hash embedded_script_hash = bitcoin_short_hash(
        p2wpkh_script.to_data(false));
    script p2sh_p2wpkh_output_script = script::to_pay_script_hash_pattern(
        embedded_script_hash);

    // Set prevout in transaction
    output_point outpoint;
    outpoint.metadata.cache.set_script(std::move(p2sh_p2wpkh_output_script));

    // Set prevout amount in transaction.
    uint8_t input_index(0u);
    std::string btc_amount_in = "1.298";
    uint64_t input_amount;
    decode_base10(input_amount, btc_amount_in, btc_decimal_places);
    outpoint.metadata.cache.set_value(input_amount);

    p2sh_p2wpkh_transaction.inputs()[0].set_previous_output(outpoint);

    // Create input script.
    //--------------------------------------------------------------------------

    // Wrap (P2SH) redeem script in single single data push.
    data_chunk p2sh_embedded_script_chunk =
        to_chunk(p2wpkh_script.to_data(true));
    script p2sh_p2wpkh_input_script(p2sh_embedded_script_chunk, false);
    p2sh_p2wpkh_transaction.inputs()[0].set_script(p2sh_p2wpkh_input_script);

    // Create Witness.
    //--------------------------------------------------------------------------

    // Create signature for witness.
    // Script code.
    script script_code = script::to_pay_key_hash_pattern(
          bitcoin_short_hash(pubkey_witness_aware));
    // Pass script_version::zero & prev input amount.
    endorsement sig;
    script::create_endorsement(sig, my_secret_witness_aware, script_code,
        p2sh_p2wpkh_transaction, input_index, sighash_algorithm::all,
        script_version::zero, input_amount);

    // 02 [signature] [public key]
    data_stack witness_stack = {
        sig,
        to_chunk(pubkey_witness_aware)
    };
    witness p2wpkh_witness(witness_stack);
    p2sh_p2wpkh_transaction.inputs()[0].set_witness(p2wpkh_witness);

    return p2sh_p2wpkh_transaction;
}



code debug_program(program& current_program, const script& current_script) {

    code ec;

    //Check if script is valid.
    //    Trailing invalid op due to push op size mismatch
    //    Script is unspendable (op_return / > max_script_size of 10kbytes)
    if (!current_program.is_valid()) {
        ec = error::invalid_script;
        return ec;
    }

    // Script is valid:
    else {

        // Run individual operations of script.
        int script_index(0);

        // Loop through all script operations.
        for (const auto& op: current_program) {

            // Max script element size (520 bytes)
            if (op.is_oversized()) {
                return error::invalid_push_data_size;
            }

            // Disallowed operations which can cause script vulnerabilities.
            if (op.is_disabled()) {
                return error::op_disabled;
            }

            // Increment operation count for (op >= op_97),
            //    Maximimum count of 201 permitted.
            if (!current_program.increment_operation_count(op)) {
                return error::invalid_operation_count;
            }

            // If operation is unconditional, conditional state must be positive
            //    for subsequent operation.
            if (current_program.if_(op))
            {
                // Check that stack < 1000 elements (overflow).
                if (!current_program.is_stack_overflow())
                {
                    // Execute operation. Changes state of stack.
                    if ((ec = current_program.evaluate(op))) // if error
                    {
                        return ec;
                    }

                    // Print out operator that has been executed.
                    std::cout << std::endl;
                    std::cout << std::string(script_index + 1, '>')
                              << " Operation: " << script_index << std::endl;
                    std::cout << op.to_string(rule_fork::all_rules) << std::endl;
                    std::cout << std::endl;

                    // Print stack state after execution of operation.
                    program program_copy(current_script, current_program);
                    std::cout << std::string(script_index + 1, '>')
                              << " Stack after operation: "
                              << script_index << std::endl;
                    while (!program_copy.empty())
                    {
                        std::cout << "[" << encode_base16(program_copy.pop())
                                  << "]" << std::endl;
                    }

                    // Increment script_index.
                    script_index += 1;
                }
            }
        }
    }

    // Checks for no outstanding flow control operations.
    // e.g. missing ENDIF
    current_program.closed() ?
        ec = error::success : ec = error::invalid_stack_scope;

    return ec;

}


code run_custom_script(const transaction& transaction, uint32_t input_index,
    uint32_t forks) {

    data_chunk my_data(32);
    pseudo_random_fill(my_data);
    auto my_hash = sha256_hash_chunk(my_data);

    operation::list my_operations = {
        operation(my_hash),
        operation(my_data),
        operation(opcode::push_size_0),
        operation(opcode::if_),
            operation(opcode::hash160),
            operation(opcode::equal),
        operation(opcode::else_),
            operation(opcode::sha256),
            operation(opcode::equal),
        operation(opcode::endif)
    };
    script my_script(my_operations);

    program my_program(my_script, transaction, input_index, forks);
    std::cout << "============= My script evaluation ============"
        << std::endl;
    code ec;
    if ((ec = debug_program(my_program, my_script)))
        return ec;
    return error::success;

}


code run_p2sh_p2wpkh(const transaction& transaction, uint32_t input_index,
    uint32_t forks) {

    // Assumes witness and previous output point set in transaction.
    auto p2sh_p2wpkh_transaction = transaction;
    auto input_script = p2sh_p2wpkh_transaction.inputs()[input_index].script();
    auto witness = p2sh_p2wpkh_transaction.inputs()[input_index].witness();
    auto prevout_script = p2sh_p2wpkh_transaction.inputs()[input_index]
        .previous_output().metadata.cache.script();
    auto input_amount = p2sh_p2wpkh_transaction.inputs()[input_index]
        .previous_output().metadata.cache.value();

    code ec;

    // 1) Evaluate input script.
    //--------------------------------------------------------------------------

    program input_program(input_script, p2sh_p2wpkh_transaction,
        input_index, forks);
    std::cout << "=========== Input script evaluation ==========="
              << std::endl;
    if ((ec = debug_program(input_program, input_script)))
        return ec;

    // 2) Evaluate output script.
    //--------------------------------------------------------------------------

    program output_program(prevout_script, input_program);
    std::cout << "\n"
              << "=========== Output script evaluation =========="
              << std::endl;
    if ((ec = debug_program(output_program, prevout_script)))
        return ec;

    // 3) Evaluate stack after input/output run.
    //--------------------------------------------------------------------------

    if (!output_program.stack_result(false))
    {
        return error::stack_false;
    }

    // 4) Check for p2w pattern.
    //--------------------------------------------------------------------------

    bool witnessed(false);

    if ((forks & rule_fork::bip141_rule) == rule_fork::bip141_rule)
    {
        if ((witnessed = script::is_witness_program_pattern(
            prevout_script.operations())))
        {
            //Omitted: Evaluate witness program
        }
    }

    // 5) Detect P2SH pattern in output script (BIP16)
    //--------------------------------------------------------------------------

    if (!((forks & rule_fork::bip16_rule) == rule_fork::bip16_rule))
    {
        return error::success;
    }

    if (prevout_script.output_pattern() == script_pattern::pay_script_hash)
    {

        std::cout << "\n"
                  << "----------- P2SH pattern detected -------------"
                  << std::endl;

        // Valid embedded script push in input script.
        if (!script::is_relaxed_push(input_script.operations()))
            return error::invalid_script_embed;

        // Extract embedded script at the top of the stack.
        script embedded_script(input_program.pop(), false);
        program embedded_program(embedded_script, std::move(input_program), true);

        std::cout << "\n"
                  << "======= P2SH Embedded script evaluation ======="
                  << std::endl;

        if ((ec = debug_program(embedded_program, embedded_script)))
            return ec;

        if (!embedded_program.stack_result(false))
            return error::stack_false;

        // 6) Detect witness program pattern in P2SH embedded script (bip141)
        //----------------------------------------------------------------------

        if (!((forks & rule_fork::bip141_rule) == rule_fork::bip141_rule))
        {
            return error::success;
        }

        if ((witnessed = script::is_witness_program_pattern(
            embedded_script.operations())))
        {

            std::cout << "\n"
                      << "---------- Witness program detected -----------"
                      << std::endl;

            // 7) Extract and run extracted witness program script.
            //--------------------------------------------------------------

            // The input script must be a push of the embedded_script (bip141).
            if (input_script.size() != 1)
            {
                return error::dirty_witness;
            }

            const auto version = embedded_script.version();

            // Detect version 0 of witness program.
            if (version == script_version::zero)
            {
                script script;
                data_stack stack;

                if (!witness.extract_embedded_script(
                    script, stack, embedded_script))//
                {
                    return error::invalid_witness;
                }

                std::cout
                    << "\n"
                    << "============== Witness evaluation ============="
                    << std::endl;

                program witness(script, p2sh_p2wpkh_transaction, input_index,
                    forks, std::move(stack), input_amount,
                    version);

                ec = debug_program(witness, script);

                if (!witness.stack_result(false))
                    return error::stack_false;
            }
        }
    } // End p2sh run.

    // Witness must be empty
    if (!witnessed && !witness.empty())
        return error::unexpected_witness;

    return error::success;
}



int main() {

  auto template_transaction = create_transaction_template();

  code ec = run_custom_script(template_transaction, 0 , rule_fork::all_rules);
  std::cout << "\n"
            << "-------- My script evaluation complete --------"
            << std::endl;
  std::cout << ec.message() << "\n" << std::endl;

  auto p2sh_p2wphk_transaction = create_p2sh_p2wpkh(template_transaction);
  ec = run_p2sh_p2wpkh(p2sh_p2wphk_transaction, 0, rule_fork::all_rules);
  std::cout << "\n"
            << "--------- Script evaluation complete ----------"
            << std::endl;
  std::cout << ec.message() << "\n" << std::endl;

  return 0;

}
```
