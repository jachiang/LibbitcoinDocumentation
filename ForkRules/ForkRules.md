# Fork Rules

The list of Bitcoin fork rules supported in Libbitcoin is shown in the table below. Each fork rule can be individually toggled on and off by flipping the respective bit in the Libbitcoin [`rule_fork`](https://github.com/libbitcoin/libbitcoin/blob/master/include/bitcoin/bitcoin/machine/rule_fork.hpp#L27-L101) object.

*Note: The fork rule bit assignment in the table below reflects the most recent stable version 3 branch. The fork rule bit assignment on the current master branch and upcoming version 4 release is different and can found [here](https://github.com/libbitcoin/libbitcoin/blob/master/include/bitcoin/bitcoin/machine/rule_fork.hpp#L27-L101).*

| Fork Rule | Bit |
|-----------|-----|
| Testnet: Minimum difficulty blocks | [`0`](https://github.com/libbitcoin/libbitcoin/blob/version3/include/bitcoin/bitcoin/machine/rule_fork.hpp#L31-L32)|
| [BIP16](https://github.com/bitcoin/bips/blob/master/bip-0016.mediawiki): Pay-to-Script-Hash | [`1`](https://github.com/libbitcoin/libbitcoin/blob/version3/include/bitcoin/bitcoin/machine/rule_fork.hpp#L34-L35) |
| [BIP30](https://github.com/bitcoin/bips/blob/master/bip-0030.mediawiki): No duplicated unspent transaction ID's | [`2`](https://github.com/libbitcoin/libbitcoin/blob/version3/include/bitcoin/bitcoin/machine/rule_fork.hpp#L37-L38) |
| [BIP34](https://github.com/bitcoin/bips/blob/master/bip-0034.mediawiki): Coinbase must include height | [`3`](https://github.com/libbitcoin/libbitcoin/blob/version3/include/bitcoin/bitcoin/machine/rule_fork.hpp#L40-L41) |
| [BIP66](https://github.com/bitcoin/bips/blob/master/bip-0066.mediawiki): Strict DER signature encoding | [`4`](https://github.com/libbitcoin/libbitcoin/blob/version3/include/bitcoin/bitcoin/machine/rule_fork.hpp#L43-L44) |
| [BIP65](https://github.com/bitcoin/bips/blob/master/bip-0065.mediawiki): Check locktime verify | [`5`](https://github.com/libbitcoin/libbitcoin/blob/version3/include/bitcoin/bitcoin/machine/rule_fork.hpp#L46-L47) |
| [BIP90](https://github.com/bitcoin/bips/blob/master/bip-0090.mediawiki): Hard code bip34 activation heights |[`6`](https://github.com/libbitcoin/libbitcoin/blob/version3/include/bitcoin/bitcoin/machine/rule_fork.hpp#L49-L50) |
| [BIP68](https://github.com/bitcoin/bips/blob/master/bip-0068.mediawiki): Enforcement of relative locktime | [`8`](https://github.com/libbitcoin/libbitcoin/blob/version3/include/bitcoin/bitcoin/machine/rule_fork.hpp#L55-L56) |
| [BIP112](https://github.com/bitcoin/bips/blob/master/bip-0112.mediawiki): Check sequence verify | [`9`](https://github.com/libbitcoin/libbitcoin/blob/version3/include/bitcoin/bitcoin/machine/rule_fork.hpp#L58-L59) |
| [BIP113](https://github.com/bitcoin/bips/blob/master/bip-0113.mediawiki): Median time past for locktime | [`10`](https://github.com/libbitcoin/libbitcoin/blob/version3/include/bitcoin/bitcoin/machine/rule_fork.hpp#L61-L62) |
| [BIP141](https://github.com/bitcoin/bips/blob/master/bip-0141.mediawiki): Segregated witness | [`11`](https://github.com/libbitcoin/libbitcoin/blob/version3/include/bitcoin/bitcoin/machine/rule_fork.hpp#L64-L65) |
| [BIP143](https://github.com/bitcoin/bips/blob/master/bip-0143.mediawiki): Signature Verification for version 0 witness | [`12`](https://github.com/libbitcoin/libbitcoin/blob/version3/include/bitcoin/bitcoin/machine/rule_fork.hpp#L67-L68) |
| [BIP147](https://github.com/bitcoin/bips/blob/master/bip-0147.mediawiki): Dummy stack element malleability fix | [`13`](https://github.com/libbitcoin/libbitcoin/blob/version3/include/bitcoin/bitcoin/machine/rule_fork.hpp#L70-L71) |
| Local Regtest: Difficulty retargeting | [`30`](https://github.com/libbitcoin/libbitcoin/blob/version3/include/bitcoin/bitcoin/machine/rule_fork.hpp#L73-L74) |

The `rule_fork` object is a uint32_t enum, which represents which represents the set of currently active forks. Creating a custom set of fork rules means selectively setting the respective bits.

```c++
// Turn off BIP16 soft fork, by selectively deactivating the BIP16 fork bit.
// Note: all rules includes testnet and regtest.
uint32_t my_fork_rules = rule_fork::all_rules ^ rule_fork::bip16_rule;

// Turn off BIP141/143 soft fork(s).
uint32_t my_fork_rules = rule_fork::all_rules ^ rule_fork::bip141_rule
    ^ rule_fork::bip143_rule;
```
The ability to selectively support specific fork rules grants library users the freedom to chose which specific fork rules to support. Toggling support for a fork rule can also be helpful feature during testing. The entire list of fork rules supported by Libbitcoin version 3 can be found [here](https://github.com/libbitcoin/libbitcoin/blob/version3/include/bitcoin/bitcoin/machine/rule_fork.hpp#L27-L99).

## Example: BIP16 Soft Fork

We may also toggle the activation of specific fork rules to effectively illustrate the backwards compatibility of past soft forks, such as BIP16(pay-to-script-hash).

[BIP16](https://github.com/bitcoin/bips/blob/master/bip-0016.mediawiki) did not add any new script operators, but introduced a new stack evaluation rule: First, the embedded script data push in the input script would be verified, to ensure it hashed to the embedded script hash in the P2SH script. If this evaluated to true, the embedded script would then be evaluated again, but this time together with the input script.

![BIP16](https://ipfs.io/ipfs/Qmaq7aBYB8ncoVRsmCHUYMBDTDmFBCv5c8d8gPpQqWAEKU)

We can demonstrate the BIP16 soft fork in Libbitcoin by constructing a P2SH(2-of-2 Multisig) script and passing it to the interpreter together with two different input scripts: One input script which contains only the embedded script, and another one, which also includes an input script, as illustrated above.

```c++
// Omitted for brevity:
// Construction of p2sh_transaction object used in example below.
```
```c++
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
```

We now construct two different input scripts for comparison. The first input script only contains the embedded script which hashes to the embedded script hash in the P2SH(2-of-2 Multisig) script.

The second input script also includes the input script with the two required signatures.

```c++
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
```
We can now demonstrate that the our first script does indeed evaluate to true pre BIP16. You can easily check that it does not follow the BIP16 rules by activating BIP16 and running the script again.

```c++
// Script Verification w/o BIP16 activation.
//---------------------------------------------------------------------------

// Add input script to transaction.
p2sh_transaction.inputs()[0].set_script(unsigned_input_script);

// Turn off BIP16 soft fork.
// Note: all rules includes testnet and regtest.
auto my_fork_rules = rule_fork::all_rules ^ rule_fork::bip16_rule;

// Verify w/o signatures.
witness empty_witness;
auto ec = script::verify(p2sh_transaction, 0, my_fork_rules, unsigned_input_script,
    empty_witness, p2sh_multisig_script, previous_output_amount);

// Prints success
std::cout << ec.message() << std::endl;
```

We will now activate BIP16 in the following step. In order to conform to BIP16 rules, our input script must contain the 2-of-2 multisig input script.

```c++
// Script verification with BIP16 activation.
//---------------------------------------------------------------------------

// Add input script to transaction.
p2sh_transaction.inputs()[0].set_script(signed_input_script);

// BIP16 is active.
// Note: all rules includes testnet and regtest.
my_fork_rules = rule_fork::all_rules;

// Input script also works without BIP16 activation.
// my_fork_rules = rule_fork::all_rules ^ rule_fork::bip16_rule;

// Verify with signatures.
ec = script::verify(p2sh_transaction, 0, my_fork_rules, signed_input_script,
    empty_witness, p2sh_multisig_script, previous_output_amount);

// Prints success
std::cout << ec.message() << std::endl;
```
By deactivating BIP16 in the example above you can verify that the input script is backwards compatible.

You can find the complete example script of this section [here](https://github.com/libbitcoin/libbitcoin/wiki/Examples:-Fork-Rules).

## Example: BIP141/143 Soft fork

BIP141 introduced the witness as part of the transaction serialisation format, which contains signatures previously located in the input script. BIP143 updated the sighash serialisation format for witness signatures.

Let us consider a P2WPKH script, which is backwards compatible to previous consensus rules, meaning that it can spent by anyone when BIP141 is not activated.

![BIP141/143](https://ipfs.io/ipfs/QmVT7XmheFAPzsokDGCUA7NtTihiArCKxoMdTdUZ2gurhL)

To demonstrate this in Libbitcoin, we will first construct the P2WPKH script, and then attempt to spend it with an empty input script.

```c++
// Omitted for brevity:
// Construction of p2wpkh_transaction object used in example below.
```
```c++
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
decode_base10(previous_output_amount, previous_btc_amount, btc_decimal_places);
```

With BIP141/143 deactivated, this output script will evaluate to true with an empty input script.

```c++
// Script Verification w/o BIP141/143 activation.
//---------------------------------------------------------------------------

// Deactivate BIP141/143.
// Note: all rules includes testnet and regtest.
auto my_fork_rules = rule_fork::all_rules ^ rule_fork::bip141_rule
    ^ rule_fork::bip143_rule;

witness empty_witness;
script empty_input_script;

auto ec = script::verify(p2wpkh_transaction, 0, my_fork_rules,
    empty_input_script, empty_witness, p2wpkh_operations,
    previous_output_amount);

// Prints success
std::cout << ec.message() << std::endl;
```

Set the fork rules to `all_rules` to verify that it is not valid with BIP141/143 activated. With BIP141/143 activated, we need to construct a valid witness.

```c++
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
```

The witness is added to transaction and passed to `script::verify()`. We leave the input script empty as the signatures are included in the witness.

```c++
// Script Verification with BIP141/143 activation.
//---------------------------------------------------------------------------

my_fork_rules = rule_fork::all_rules;
// Without bip141: error code 77, unexpected witness.
// Without bip143: error code 65, stack false...
//    (BIP143 signature serialisation not activated).

// Add witness to transaction.
p2wpkh_transaction.inputs()[0].set_witness(p2wpkh_witness);

ec = script::verify(p2wpkh_transaction, 0, my_fork_rules, empty_input_script,
    p2wpkh_witness, p2wpkh_operations, previous_output_amount);

// Prints success
std::cout << ec.message() << std::endl;
```
Note that the witness transaction above is not valid when BIP141/143 are deactivated. In Libbitcoin, you will have to replace the witness object with an empty one. Over-the-wire however, the witness data would not be included in messages to nodes who have not activated BIP141/143, so that this transaction would still be backwards compatible nonetheless.

You can find the complete example script of this section [here](https://github.com/libbitcoin/libbitcoin/wiki/Examples:-Fork-Rules).
