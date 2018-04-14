# The Script Machine

This section provides a deeper look into how scripts can be parsed and evaluated by the Libbitcoin script machine. The previous [script verification](https://github.com/libbitcoin/libbitcoin/wiki/Script-Verification) chapter provided an introduction to the evaluation of transaction scripts via the [`script::verify()`](https://github.com/libbitcoin/libbitcoin/blob/master/include/bitcoin/bitcoin/chain/script.hpp#L212-L218) method without exposing the reader to many details of the script machine. However, when working with more complex Bitcoin scripts, the ability to use lower level Libbitcoin script machine API functionality can be helpful in the design and debugging of more complex scripts.

The full ready-to-compile code examples from this chapter can be found [here](https://github.com/libbitcoin/libbitcoin/wiki/Examples:-Script-Machine).

## The Script Program

The basis of running a script in the Libbitcoin script machine is the [`machine::program`](https://github.com/libbitcoin/libbitcoin/blob/master/include/bitcoin/bitcoin/machine/program.hpp#L36-L165) class. A Libbitcoin program contains a script, stack and other internal  objects which capture the current state of the program during a script run.

![machine::program](https://ipfs.io/ipfs/QmQ1fVnV8pLK1BY36gm43mKfM4tA1cPfhdKuKEjyb6gaHK)

During a script run, the current operation can affect the stack, alternate stack and the flow control state of the program, if the operation is conditional. At the beginning of a script run, the stack can be either empty, or adopt an initial state. The flow control state must be neutral at the beginning and end of each script run, ensuring that all conditional blocks are  closed (OP_IF requires OP_ENDIF).

The following example function `debug_program()` will loop through all valid operations of a program, and print out the current state of the stack after the execution of each individual operation. The printed output provides the opportunity to debug a script by following the script run step by step. It is identical to the Libbitcoin  [`program::evaluate()`](https://github.com/libbitcoin/libbitcoin/blob/master/include/bitcoin/bitcoin/machine/program.hpp#L88) method, save for the command-line debugging output, but implements the script evaluation logic with publicly accessible library methods to facilitate understanding.

`program::evaluate()`/`debug_program()` evaluation logic:

* Check if script is valid.
  * No size mismatch in push operations.
  * Check if script is not unspendable.
    * Does not contain op_return.
    * Size of script <= 10'000 bytes.
* Loop through each operation of program script.
  * Check that operation <= 520 bytes.
  * Check that operation is not [disabled](https://en.bitcoin.it/wiki/Script).
  * Check max operation count (201) not exceeded.
  * Check conditional state before execution.
  * Check that no stack overflow (<1000)
  * Execute operation.
  * *(debug_program: Print operation & Stack)*

```c++
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
```
We can use the program debugger to run any valid sample script. In the example below, we will run a simple script with if/else, data-push, hashing and check-equal operations. You can find the full ready-to-compile example code in its entirety [here](https://github.com/libbitcoin/libbitcoin/wiki/Examples:-Script-Machine).

**Example Bitcoin script:**

* `[sha256(my_data)]`
* `[my_data]`
* `OP_0`
* `OP_IF`
  * `OP_HASH160`
  * `OP_EQUAL`
* `OP_ELSE`
  * `OP_SHA256`
  * `OP_EQUAL`
* `OP_ENDIF`

The debugger executes 8 operations for this example and will print the following stack states to the terminal.

```
============= My script evaluation ============

> Operation: 0
[5b60a9080bcb22891c91bbdea31dcda679477b198b903c06c938381810dab208]

> Stack after operation: 0
[5b60a9080bcb22891c91bbdea31dcda679477b198b903c06c938381810dab208]

>> Operation: 1
[ba04f00874a25fb06d70b8b043ec118113fa3c4d337e8da59f64cb52f07321a7]

>> Stack after operation: 1
[ba04f00874a25fb06d70b8b043ec118113fa3c4d337e8da59f64cb52f07321a7]
[5b60a9080bcb22891c91bbdea31dcda679477b198b903c06c938381810dab208]

>>> Operation: 2
zero

>>> Stack after operation: 2
[]
[ba04f00874a25fb06d70b8b043ec118113fa3c4d337e8da59f64cb52f07321a7]
[5b60a9080bcb22891c91bbdea31dcda679477b198b903c06c938381810dab208]

>>>> Operation: 3
if

>>>> Stack after operation: 3
[ba04f00874a25fb06d70b8b043ec118113fa3c4d337e8da59f64cb52f07321a7]
[5b60a9080bcb22891c91bbdea31dcda679477b198b903c06c938381810dab208]

>>>>> Operation: 4
else

>>>>> Stack after operation: 4
[ba04f00874a25fb06d70b8b043ec118113fa3c4d337e8da59f64cb52f07321a7]
[5b60a9080bcb22891c91bbdea31dcda679477b198b903c06c938381810dab208]

>>>>>> Operation: 5
sha256

>>>>>> Stack after operation: 5
[5b60a9080bcb22891c91bbdea31dcda679477b198b903c06c938381810dab208]
[5b60a9080bcb22891c91bbdea31dcda679477b198b903c06c938381810dab208]

>>>>>>> Operation: 6
equal

>>>>>>> Stack after operation: 6
[01]

>>>>>>>> Operation: 7
endif

>>>>>>>> Stack after operation: 7
[01]
```
We can observe that the top stack element evaluates to true after the completion of the script run. The full ready-to-compile code examples from this chapter can be found [here](https://github.com/libbitcoin/libbitcoin/wiki/Examples:-Script-Machine).

## Evaluating Input and Output Scripts

When evaluating input and output scripts during the validation of a Bitcoin transaction, both scripts are not concatenated, but instead are evaluated in separate runs. The input script is evaluated first, while the output script will begin its run thereafter initiated with the stack resulting from the first run.

*Note: Input and output scripts are run separately, because concatenation of both scripts could allow the output script to be altered by the preceding input script. For example, it would be possible to end the input script with a data push operator, rendering the entire output script into a single data push.*

![evaluation: no rules](https://ipfs.io/ipfs/QmRJyaAbwR7SwhteXWNWHYAMf3xLSoBivB6neHqsQAwFRw)

After completion of the output script run, the stack will evaluate to true if the top stack element is a non-zero element. In the case that fork rules BIP16 and BIP141 are inactive, the transaction script verification is complete.

### Evaluating P2SH Scripts (BIP16)

If BIP16 is activated, the transaction script evaluation is extended by an embedded script run. The embedded script data push from the top of the input script stack is popped off, and the remaining stack is carried over to the P2SH embedded script run.

![evaluation: BIP16](https://ipfs.io/ipfs/QmX3hgpG8am2JRnRpwFSTsiXtYB6dWMWvuHBMy45hoZEep)

### Evaluating Witness Program Scripts (BIP141)

If BIP141 is activated, the transaction script evaluation is extended similar to the P2SH case. After the output script run is completed, the output script is checked whether it fits the pay-to-witness pattern. If a pay-to-witness pattern match is positive and the witness program is a P2WPKH script, a P2PKH script of the witness public key hash will be run with the witness in its initial stack. In the case of a P2WSH witness program, the P2WSH embedded script is first popped off the witness and run with the remaining witness stack.

![evaluation: BIP141](https://ipfs.io/ipfs/QmbKFRn4D1hZN2u3KT6tn4jZf3KFjnhrrZBNoT29EbPDXb)

### P2SH(Pay-to-Witness Program) (BIP16/141)

For pay-to-witness programs wrapped in P2SH scripts, the initial script runs will mirror that of regular P2SH script evaluations. However, once the P2SH output script run is successfully completed, and a pay-to-witness pattern is detected in the embedded script, the extracted script run will be initiated with the witness stack.

![evaluation: BIP16+141](https://ipfs.io/ipfs/QmbeDUdWauariV4Qfe8WL7b8Kr24Z1fuw5Amp9kM7PgQZN)

### P2SH(P2WPKH) Example:

We illustrate a P2SH(P2WPKH) script evaluation example below. Using the previous `debug_program()` example function we can  facilitate a operation-by-operation read-out of the program stack during each script run.

Our example is wrapped in the function `run_p2sh_p2wpkh()`, which will extract the input, output scripts and witness from its transaction parameter:

You can find the ready-to-compile example code in its entirety [here](https://github.com/libbitcoin/libbitcoin/wiki/Examples:-Script-Machine).

```c++
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
```

**P2SH(P2WPKH) Input Script Run:**

The first part of the function will attempt a input script run.

```c++
    // 1) Evaluate input script.
    //--------------------------------------------------------------------------

    program input_program(input_script, p2sh_p2wpkh_transaction,
        input_index, forks);
    std::cout << "=========== Input script evaluation ==========="
              << std::endl;
    if ((ec = debug_program(input_program, input_script)))
        return ec;
```

This prints out the following for our P2SH(P2WPKH) spending transaction. The input script consists of a single data push of the P2SH embedded script.

```
=========== Input script evaluation ===========

> Operation: 0
[00149a19a31c2fda7d0c30215ec954a20a542aa84ad3]

> Stack after operation: 0
[00149a19a31c2fda7d0c30215ec954a20a542aa84ad3]
```

**P2SH(P2WPKH) Output Script Run:**

Subsequently, given that the input script run was successful, the output script is initiated with the stack state resulting from the first run. The stack after the output script run must evaluate to true.

```c++
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
```

The second run results in the following stack states printed by our example `debug_program()` function.

```
=========== Output script evaluation ==========

> Operation: 0
hash160

> Stack after operation: 0
[dcdc2f89b96c420751e3750da7d5073a81b16946]

>> Operation: 1
[dcdc2f89b96c420751e3750da7d5073a81b16946]

>> Stack after operation: 1
[dcdc2f89b96c420751e3750da7d5073a81b16946]
[dcdc2f89b96c420751e3750da7d5073a81b16946]

>>> Operation: 2
equal

>>> Stack after operation: 2
[01]
```
**P2SH(P2WPKH) P2SH Script Run:**

Since the stack evaluates to true after running the P2SH script, the output script is first checked whether it matches a pay-to-witness program. It does not, so subsequently it is matched against the P2SH output script pattern. This match is positive, so the embedded script is popped off the stack resulting from the input script run and is initialised with the remaining input stack.

```c++
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
```

The P2SH embedded script run prints out the following:

```
----------- P2SH pattern detected -------------

======= P2SH Embedded script evaluation =======

> Operation: 0
zero

> Stack after operation: 0
[]

>> Operation: 1
[9a19a31c2fda7d0c30215ec954a20a542aa84ad3]

>> Stack after operation: 1
[9a19a31c2fda7d0c30215ec954a20a542aa84ad3]
[]
```

**P2SH(P2WPKH) Witness Program Script Run:**

Finally, our P2SH embedded script is positively matched against the pay-to-witness program pattern. Our embedded script  `zero` `[20-byte hash(public key)]` represents a P2PKH script spendable by the owner of the private key corresponding to the public key in the pay-to-witness program.

The P2PKH script is initialised with the witness data stack.

```c++
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
```

The final witness program run prints out the following stack states.

```
---------- Witness program detected -----------

============== Witness evaluation =============

> Operation: 0
dup

> Stack after operation: 0
[026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf3]
[026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf3]
[304402206eb381e8f02839fb83c4970e716a8039228d357b42f4f5a50e994afae7be4313022067d8ad811f1c57f6e4d75dcc486112d8c367c562de7b26f441db393355d6f2ef01]

>> Operation: 1
hash160

>> Stack after operation: 1
[9a19a31c2fda7d0c30215ec954a20a542aa84ad3]
[026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf3]
[304402206eb381e8f02839fb83c4970e716a8039228d357b42f4f5a50e994afae7be4313022067d8ad811f1c57f6e4d75dcc486112d8c367c562de7b26f441db393355d6f2ef01]

>>> Operation: 2
[9a19a31c2fda7d0c30215ec954a20a542aa84ad3]

>>> Stack after operation: 2
[9a19a31c2fda7d0c30215ec954a20a542aa84ad3]
[9a19a31c2fda7d0c30215ec954a20a542aa84ad3]
[026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf3]
[304402206eb381e8f02839fb83c4970e716a8039228d357b42f4f5a50e994afae7be4313022067d8ad811f1c57f6e4d75dcc486112d8c367c562de7b26f441db393355d6f2ef01]

>>>> Operation: 3
equalverify

>>>> Stack after operation: 3
[026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf3]
[304402206eb381e8f02839fb83c4970e716a8039228d357b42f4f5a50e994afae7be4313022067d8ad811f1c57f6e4d75dcc486112d8c367c562de7b26f441db393355d6f2ef01]

>>>>> Operation: 4
checksig

>>>>> Stack after operation: 4
[01]
```
Our P2SH(P2WPKH) transaction example evaluates to true. The full ready-to-compile code examples from this chapter can be found [here](https://github.com/libbitcoin/libbitcoin/wiki/Examples:-Script-Machine).
