# Segwit: P2WSH

## Sending to P2WSH
Sending a transaction to a Segwit Pay-to-Witness-Script-Hash (P2WPKH) destination requires the construction of the following scriptPubKey:

| TX Element 	 | Script/Serialization 									  |
| -------------|------------------------------------------|
| ScriptPubKey | `zero` `[32-byte sha256(WitnessScript)]` |

The WitnessScript is the redeemscript for the native P2WSH output.

We can construct such a P2WSH scriptPubKey by populating a `operation::list` object with the respective `operations`.

```c++
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
```
The rest of the transaction is built and signed in the same fashion as described in [building transactions](https://github.com/libbitcoin/libbitcoin/wiki) and [sighash](https://github.com/libbitcoin/libbitcoin/wiki) documentation sections if the input(s) are non-segwit.

You can find the complete P2WSH example script [here](https://github.com/libbitcoin/libbitcoin/wiki).

## Sending from P2WSH
Spending a P2WSH output requires adherence to the following transaction scheme.

| TX Element   | Script/Serialization               |
| -------------|------------------------------------|
| ScriptPubKey | `According to destination address` |
| ScriptSig    | `"empty"`                          |
| ScriptCode   | `WitnessScript`                    |
| Witness      | `[Signature(s)]` `[WitnessScript]` |

The construction of the input in Libbitcoin initially follows the regular steps of populating `input` objects with the previous transaction elements.
```c++
//Omitted: Construction of output according to destination address
```
```c++
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

// Build TX
transaction tx;
tx.set_version(1u);
tx.inputs().push_back(p2wsh_input);
tx.outputs().push_back(p2pkh_output);
tx.set_locktime(0u);
```
### Signing a P2WSH transaction
[BIP143](https://github.com/bitcoin/bips/blob/master/bip-0143.mediawiki) describes a segwit-specific sighash generation algorithm for signatures evaluated by `CHECKSIG`, `CHECKSIGVERIFY`, `CHECKMULTISIG`, `CHECKMULTISIGVERIFY`.  

In particular, a ScriptCode and the previous input amount are required for the signature algorithm.
We will first construct the WitnessScript, which in this specific example will be a 2-of-3 Multisig locking script.

```c++
// ScriptCode: WitnessScript = e.g. MultiSig
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
uint64_t satoshi_amount_in;
std::string btc_amount_string_in = "1.295";
decode_base10(satoshi_amount_in, btc_amount_string_in, btc_decimal_places);
script::create_endorsement(sig0, my_secret_segwit, witness_script, tx,
    input0_index, 0x01, script_version::zero, satoshi_amount_in);
script::create_endorsement(sig1, my_secret_segwit1, witness_script, tx,
    input0_index, 0x01, script_version::zero, satoshi_amount_in);
```
The `script::create_endorsement` method will serialise the sighash according to the `script_version` argument. For segwit inputs, this argument will be set to `script_version::zero` in order for the correct sighash algorithm to be applied.

### P2WSH Witness

Once the signature `endorsement` object is created, the witness object of the transaction must be populated with the required signatures and 2-of-3 multisig script used in our example.  

Note that we add an additional empty `data_chunk` to represent the leading 0 required for multisig unlocking scripts. Once the `witness` object is serialised, each `data_chunk` entry will be prepended with a single-byte length prefix to each `data_chunk` in its stack. The leading empty `data_chunk` will be serialised to `OxOO`, which is what we need for our multisig unlocking script.

```c++
// Create Witness
data_stack witness_stack;
data_chunk empty_chunk;
witness_stack.push_back(empty_chunk);
witness_stack.push_back(sig0);
witness_stack.push_back(sig1);
witness_stack.push_back(witness_script.to_data(false));
witness p2wsh_witness(witness_stack);
tx.inputs()[0].set_witness(p2wsh_witness);
```

>The serialised encoding of the witness may appear similar to Bitcoin scripts, but only consists of serialised data chunk pushes, each with a single-byte length prefix.

This can be observed in the witness from our example, which is serialised as the following:

```c++
// Number of following witness elements for first input
04
// Leading zero for multisig
00
// Length of 1st 71-byte Endorsement
47
// 1st Endorsement(DER Signature + Sighash Marker)
304402203cdcaf02a44e37e409646e8a506724e9e1394b890cb52429ea65bac4cc2403f1022024b934297bcd0c21f22cee0e48751c8b184cc3a0d704cae2684e14858550af7d01
// Length of 2nd 72-byte Endorsement
48
// 2nd Endorsement(DER Signature + Sighash Marker)
3045022100feb4e1530c13e72226dc912dcd257df90d81ae22dbddb5a3c2f6d86f81d47c8e022069889ddb76388fa7948aaa018b2480ac36132009bb9cfade82b651e88b4b137a01
// Length of Witness Script
69
// 2-of-3 Multisig ScriptPubKey
5221026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf32103befa190c0c22e2f53720b1be9476dcf11917da4665c44c9c71c3a2d28a933c352102be46dc245f58085743b1cc37c82f0d63a960efa43b5336534275fc469b49f4ac53ae
```
---

Finally, we can express our P2WSH spending transaction in the following serialised form:
```c++
// Complete TX
std::cout << encode_base16(tx.to_data(true,true)) << std::endl;
```

```c++
0100000000010193a2db37b841b2a46f4e9bb63fe9c1012da3ab7fe30b9f9c974242778b5af8980000000000ffffffff01806fb307000000001976a914bbef244bcad13cffb68b5cef3017c7423675552288ac040047304402203cdcaf02a44e37e409646e8a506724e9e1394b890cb52429ea65bac4cc2403f1022024b934297bcd0c21f22cee0e48751c8b184cc3a0d704cae2684e14858550af7d01483045022100feb4e1530c13e72226dc912dcd257df90d81ae22dbddb5a3c2f6d86f81d47c8e022069889ddb76388fa7948aaa018b2480ac36132009bb9cfade82b651e88b4b137a01695221026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf32103befa190c0c22e2f53720b1be9476dcf11917da4665c44c9c71c3a2d28a933c352102be46dc245f58085743b1cc37c82f0d63a960efa43b5336534275fc469b49f4ac53ae00000000
```
Parsing the serialised transaction with BX gives us an overview of our constructed segwit P2WPKH transaction.
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
You can find the complete P2WSH transaction script [here](https://github.com/libbitcoin/libbitcoin/wiki).
