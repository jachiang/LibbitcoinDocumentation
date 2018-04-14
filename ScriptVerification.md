# Script Verification

This section describes the verification of input and output scripts with the relevant non-script arguments (witnesses, lock-time, previous output amounts etc). Note that script verification does not verify any chain state information. Script verification assesses whether input & output scripts evaluate to true on the script stack for a given a set of consensus rules.

![script::verify](https://ipfs.io/ipfs/QmZBWeqguWPXJQ45YYTrwUmKWEK6jRPc5phbKrJruQWczJ)

In Libbitcoin, the [`script::verify()`](https://github.com/libbitcoin/libbitcoin/blob/master/include/bitcoin/bitcoin/chain/script.hpp#L212-L218) function parses the input and output scripts through the script interpreter, in order to evaluate if the entire script evaluates to true.

The following example demonstrates the spending of a P2PKH output. We construct both P2PKH output and input scripts and then check what the two scripts evaluate to.

```c++
// Omitted for brevity:
// Construction of p2pkh_transaction object used in example below.
```
```c++
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
p2pkh_transaction.inputs()[0].set_script(p2pkh_input_script);


// Verify input script, output script.
//---------------------------------------------------------------------------

// With all fork rules, no witness.
witness empty_witness;
auto ec = script::verify(p2pkh_transaction, 0, rule_fork::all_rules,
    p2pkh_input_script, empty_witness, p2pkh_output_script,
    previous_output_amount);

// Prints success
std::cout << ec.message() << std::endl;
```

You can find the complete ready-to-compile example code from this chapter [here](https://github.com/libbitcoin/libbitcoin/wiki/Examples:-Script-Verification).

Note that we have also passed in non-script arguments into [`script::verify()`](https://github.com/libbitcoin/libbitcoin/blob/master/include/bitcoin/bitcoin/chain/script.hpp#L212-L218), such as the witness and the previous output amount, which are required for verifying BIP143 signatures. The [`rule_fork`](https://github.com/libbitcoin/libbitcoin/blob/master/include/bitcoin/bitcoin/machine/rule_fork.hpp#L27-L101) argument tells the script interpreter which Bitcoin soft [fork rules](https://github.com/libbitcoin/libbitcoin/wiki/Fork-Rules) to apply during the verification of the script.

The script verify method returns a std::error_code object, with a value from the Libbitcoin [error code enum](https://github.com/libbitcoin/libbitcoin/blob/master/include/bitcoin/bitcoin/error.hpp#L47-L244).

**Note: Changes to verify method in upcoming version 4:**

Note that the function signature for the verify function will [change](https://github.com/libbitcoin/libbitcoin/blob/master/include/bitcoin/bitcoin/chain/script.hpp#L212-L217) for the upcoming version 4 of the Libbitcoin library. Input script and witness will be moved into the transaction parameter. Optionally, the previous output point can also be extracted from the transaction metadata. The transaction verification step in the previous example would be expressed as the following.

```c++
// Libbitcoin  version4: Changes to script::verify().
// Input script and witness parameters are moved into tx.
auto ec1 = script::verify(p2pkh_transaction, input0_index,
    rule_fork::all_rules, p2pkh_output_script, previous_output_amount);

// Libbitcoin version4: Alternative script::verify() signature.
// Prevout script and amount can be moved into tx metadata.
p2pkh_transaction.inputs()[input0_index]
    .previous_output().metadata.cache.set_script(p2pkh_output_script);
p2pkh_transaction.inputs()[input0_index]
    .previous_output().metadata.cache.set_value(previous_output_amount);
auto ec2 = script::verify(p2pkh_transaction, input0_index,
    rule_fork::all_rules);
```
