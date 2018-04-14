# P2WSH

## Sending to a P2WSH Output
Sending a transaction to a Pay-to-Witness-Script-Hash (P2WPKH) address requires the construction of the following output script:

| Transaction Element | Script								                    |
| --------------------|-------------------------------------------|
| Output Script       | `zero` `[32-byte sha256(witness script)]` |

The witness script is the redeem script for such a native P2WSH output. We can construct a P2WSH output script by populating a operation vector with the respective operations.

```c++
// Witness script: Multisig.
uint8_t signatures(2u); //2 of 3
point_list points;
points.push_back(pubkey_witness_aware);
points.push_back(pubkey_witness_aware1);
points.push_back(pubkey_witness_aware2);
script witness_script = script::to_pay_multisig_pattern(signatures, points);

// P2WSH output script.
//    0 [32-byte sha256(witness script)]
hash_digest redeem_script_hash = sha256_hash(witness_script.to_data(false));
operation::list p2sh_operations;
p2sh_operations.push_back(operation(opcode::push_size_0));
p2sh_operations.push_back(operation(to_chunk(redeem_script_hash)));

// Build output.
std::string btc_amount = "1.295";
uint64_t output_amount;
decode_base10(output_amount, btc_amount, btc_decimal_places);
output p2wsh_output(output_amount, p2sh_operations);
```

If the spending of the previous transaction output(s) do not require the construction of witnesses, the rest of the transaction is built and signed according to the documentation sections [building transactions](https://github.com/libbitcoin/libbitcoin/wiki/Building-Transactions) and [sighash](https://github.com/libbitcoin/libbitcoin/wiki/Sighash-&-TX-Signing).

You can find the complete P2WSH example script [here](https://github.com/libbitcoin/libbitcoin/wiki/Examples:-Transactions-with-Input-Witnesses).

## Spending a P2WSH Output

Spending a P2WSH output requires constructing the transaction according to the following scheme.

| Transaction Element | Script                                                |
| --------------------|-------------------------------------------------------|
| Output Script       | `According to destination address`                    |
| Input Script        | `"empty"`                                             |
| Script Code         | `WitnessScript`                                       |
| Witness             | `[input script of witness script]` `[witness script]` |

The construction of the input in Libbitcoin initially follows the steps of populating input objects with the previous transaction elements.

```c++
// Omitted: Construction of output according to destination address.
```
```c++
// P2WSH input.
// Previous transaction hash.
std::string prev_tx =
    "98f85a8b774242979c9f0be37faba32d01c1e93fb69b4e6fa4b241b837dba293";
hash_digest prev_tx_hash;
decode_hash(prev_tx_hash,prev_tx);
// Previous UXTO.
uint32_t index = 0;
output_point uxto_to_spend(prev_tx_hash, index);
// Build input object.
input p2wsh_input;
p2wsh_input.set_previous_output(uxto_to_spend);
p2wsh_input.set_sequence(max_input_sequence);

// Build transaction.
transaction tx;
tx.set_version(1u);
tx.inputs().push_back(p2wsh_input);
tx.outputs().push_back(p2pkh_output);
tx.set_locktime(0u);
```
### Signing a Transaction with a P2WSH Input

[BIP143](https://github.com/bitcoin/bips/blob/master/bip-0143.mediawiki) describes a witness-specific sighash generation algorithm for signatures evaluated by `CHECKSIG`, `CHECKSIGVERIFY`, `CHECKMULTISIG`, `CHECKMULTISIGVERIFY`.  

In particular, a script code and the previous input amount are required for the signature algorithm. The script code for signing a P2WSH transaction is the witness script, which in our example is the 2-of-3 multisig script.

```c++
// Script code: witness script = e.g. multisig
uint8_t signatures(2u); //2 of 3
point_list points;
points.push_back(pubkey_witness_aware);
points.push_back(pubkey_witness_aware1);
points.push_back(pubkey_witness_aware2);
script witness_script = script::to_pay_multisig_pattern(signatures, points);

// Create signatures for witness.
endorsement sig0;
endorsement sig1;
uint8_t input0_index(0u);
uint64_t input_amount;
std::string btc_amount_in = "1.295";
decode_base10(input_amount, btc_amount_in, btc_decimal_places);
script::create_endorsement(sig0, my_secret_witness_aware, witness_script, tx,
    input0_index, sighash_algorithm::all, script_version::zero, input_amount);
script::create_endorsement(sig1, my_secret_witness_aware1, witness_script, tx,
    input0_index, sighash_algorithm::all, script_version::zero, input_amount);
```

The `script::create_endorsement` method will generate a sighash according to the witness script version parameter. For inputs requiring a witness of the current version, this argument will be set to version zero in order for the witness-specific sighash algorithm to be applied.

### P2WSH Witness

Once the signatures are created, the witness object of the transaction can be populated with the required signatures and the 2-of-3 multisig script used in our example.  

Note that we add an additional empty data chunk to represent the leading 0 required for multisig unlocking scripts. Once the witness object is serialised, each data chunk element will be prepended with a single-byte length prefix. The leading empty data chunk will be serialised to `OxOO`, since it has a length of zero, resulting in a single zero byte we need to lead our multisig input script.

```c++
// Create witness.
data_stack witness_stack;
data_chunk empty_chunk;
witness_stack.push_back(empty_chunk);
witness_stack.push_back(sig0);
witness_stack.push_back(sig1);
witness_stack.push_back(witness_script.to_data(false));
witness p2wsh_witness(witness_stack);
tx.inputs()[0].set_witness(p2wsh_witness);
```

The serialised encoding of the witness may appear similar to Bitcoin scripts, but it differs in that it only consists of serialised data pushes with the aforementioned length prefixes.

This can be observed in the serialised witness from our example:

```c++
// Number of following witness elements for the first input.
04
// Leading zero for multisig input script.
00
// Length of 1st 71-byte endorsement.
47
// 1st endorsement( DER signature + sighash marker )
304402203cdcaf02a44e37e409646e8a506724e9e1394b890cb52429ea65bac4cc2403f1022024b934297bcd0c21f22cee0e48751c8b184cc3a0d704cae2684e14858550af7d01
// Length of 2nd 72-byte endorsement.
48
// 2nd endorsement( DER signature + sighash marker )
3045022100feb4e1530c13e72226dc912dcd257df90d81ae22dbddb5a3c2f6d86f81d47c8e022069889ddb76388fa7948aaa018b2480ac36132009bb9cfade82b651e88b4b137a01
// Length of Witness Script
69
// 2-of-3 Multisig output script
5221026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf32103befa190c0c22e2f53720b1be9476dcf11917da4665c44c9c71c3a2d28a933c352102be46dc245f58085743b1cc37c82f0d63a960efa43b5336534275fc469b49f4ac53ae
```
### Serialised P2WSH Transaction

Finally, we can express our P2WSH spending transaction in the following serialised form:
```c++
// Complete transaction.
std::cout << encode_base16(tx.to_data(true,true)) << std::endl;
```

```c++
0100000000010193a2db37b841b2a46f4e9bb63fe9c1012da3ab7fe30b9f9c974242778b5af8980000000000ffffffff01806fb307000000001976a914bbef244bcad13cffb68b5cef3017c7423675552288ac040047304402203cdcaf02a44e37e409646e8a506724e9e1394b890cb52429ea65bac4cc2403f1022024b934297bcd0c21f22cee0e48751c8b184cc3a0d704cae2684e14858550af7d01483045022100feb4e1530c13e72226dc912dcd257df90d81ae22dbddb5a3c2f6d86f81d47c8e022069889ddb76388fa7948aaa018b2480ac36132009bb9cfade82b651e88b4b137a01695221026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf32103befa190c0c22e2f53720b1be9476dcf11917da4665c44c9c71c3a2d28a933c352102be46dc245f58085743b1cc37c82f0d63a960efa43b5336534275fc469b49f4ac53ae00000000
```
Parsing the serialised form with BX gives us an overview of our constructed P2WPKH transaction.
```
BX tx-decode -f json 0100000000010193a2db37b841b2a46f4e9bb63fe9c1012da3ab7fe30b9f9c974242778b5af8980000000000ffffffff01806fb307000000001976a914bbef244bcad13cffb68b5cef3017c7423675552288ac040047304402203cdcaf02a44e37e409646e8a506724e9e1394b890cb52429ea65bac4cc2403f1022024b934297bcd0c21f22cee0e48751c8b184cc3a0d704cae2684e14858550af7d01483045022100feb4e1530c13e72226dc912dcd257df90d81ae22dbddb5a3c2f6d86f81d47c8e022069889ddb76388fa7948aaa018b2480ac36132009bb9cfade82b651e88b4b137a01695221026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf32103befa190c0c22e2f53720b1be9476dcf11917da4665c44c9c71c3a2d28a933c352102be46dc245f58085743b1cc37c82f0d63a960efa43b5336534275fc469b49f4ac53ae00000000
```
```json
{
    "transaction": {
        "hash": "440fe853c40f94bee4993008917fa6ee809ad52e4a0ceea8632b58be7895f558",
        "inputs": [
            {
                "previous_output": {
                    "hash": "98f85a8b774242979c9f0be37faba32d01c1e93fb69b4e6fa4b241b837dba293",
                    "index": "0"
                },
                "script": "",
                "sequence": "4294967295",
                "witness": "[] [304402203cdcaf02a44e37e409646e8a506724e9e1394b890cb52429ea65bac4cc2403f1022024b934297bcd0c21f22cee0e48751c8b184cc3a0d704cae2684e14858550af7d01] [3045022100feb4e1530c13e72226dc912dcd257df90d81ae22dbddb5a3c2f6d86f81d47c8e022069889ddb76388fa7948aaa018b2480ac36132009bb9cfade82b651e88b4b137a01] [5221026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf32103befa190c0c22e2f53720b1be9476dcf11917da4665c44c9c71c3a2d28a933c352102be46dc245f58085743b1cc37c82f0d63a960efa43b5336534275fc469b49f4ac53ae]"
            }
        ],
        "lock_time": "0",
        "outputs": [
            {
                "address_hash": "bbef244bcad13cffb68b5cef3017c74236755522",
                "script": "dup hash160 [bbef244bcad13cffb68b5cef3017c74236755522] equalverify checksig",
                "value": "129200000"
            }
        ],
        "version": "1"
    }
}
```
You can find the complete P2WSH transaction script [here](https://github.com/libbitcoin/libbitcoin/wiki/Examples:-Transactions-with-Input-Witnesses).
