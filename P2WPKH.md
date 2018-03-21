# Segwit: P2WPKH

## Sending to P2WPKH
Sending a transaction to a Segwit Pay-to-Witness-Public-Key-Hash (P2WPKH) destination requires the construction of the following scriptPubKey:

| TX Element 	 | Script/Serialization 									 |
| -------------|-----------------------------------------|
| ScriptPubKey | `zero` ``[20-byte hash160(PublicKey)]`` |

We can construct such a P2WPKH scriptPubKey by populating a `operation::list` object with the relevant operations shown above.

```c++
//P2WPKH ScriptPubKey
//zero [20-byte hash160(pubKey)]
operation::list p2wpkh_oplist;
p2wpkh_oplist.push_back(operation(opcode::push_size_0));
p2wpkh_oplist.push_back(operation(to_chunk(bitcoin_short_hash(pubkey_segwit))));

//Build P2WPKH Output
std::string btc_amount_string = "0.995";
uint64_t satoshi_amount;
decode_base10(satoshi_amount, btc_amount_string, btc_decimal_places);
output p2wpkh_output(satoshi_amount, p2wpkh_oplist);
```

If the input(s) are non-segwit, rest of the transaction is built and signed according to the documentation sections [building transactions](https://github.com/libbitcoin/libbitcoin/wiki/Building-Transactions) and [sigHash](https://github.com/libbitcoin/libbitcoin/wiki/Sighash-&-TX-Signing).

You can find the complete P2WPKH example script [here](https://github.com/libbitcoin/libbitcoin/wiki/Segwit-TX-Examples).

## Sending from P2WPKH
Spending a P2WPKH output requires constructing the transaction according to the following scheme.

| TX Element 	 | Script/Serialization 																									 |
| -------------|-------------------------------------------------------------------------|
| ScriptPubKey | `According to destination address` 																		 |
| ScriptSig 	 | `"empty"`																															 |
| ScriptCode 	 | `DUP` `HASH160` `[20-byte hash160(PublicKey)]` `EQUALVERIFY` `CHECKSIG` |
| Witness 		 | `[Signature]` `[PublicKey]` 																						 |

The construction of the input in Libbitcoin initially follows the regular steps of populating `input` objects with the previous transaction elements.

```c++
//Omitted: Construction of output according to destination address
```
```c++
//P2WPKH Input
//Previous TX hash
std::string prev_tx_string = "26c9768cdbb00332ff1052f27e71eb7e82b578bf02fb6d7eecfd0b43412e9d10";
hash_digest prev_tx_hash;
decode_hash(prev_tx_hash,prev_tx_string);
//Previous UXTO index:
uint32_t index = 0;
output_point uxto_tospend(prev_tx_hash, index);
//Build input_0 object
input p2wpkh_input;
p2wpkh_input.set_previous_output(uxto_tospend);
p2wpkh_input.set_sequence(0xffffffff);

//Build TX
transaction tx;
tx.set_version(1u);
tx.inputs().push_back(p2wpkh_input);
tx.outputs().push_back(p2wpkh_output);
```

### Signing a P2WPKH transaction
[BIP143](https://github.com/bitcoin/bips/blob/master/bip-0143.mediawiki) describes a segwit-specific sighash generation algorithm for signatures evaluated by `CHECKSIG`, `CHECKSIGVERIFY`, `CHECKMULTISIG`, `CHECKMULTISIGVERIFY`.  

In particular, a ScriptCode and the previous input amount are required for the signature algorithm. The ScriptCode for spending a P2WPKH is simply a `P2PKH(pubKeyHash)` script.

```c++
//Create Segwit Signature

//ScriptCode:
script p2wpkh_script_code = script::to_pay_key_hash_pattern(bitcoin_short_hash(pubkey_segwit));

//Previous Input Amount:
uint8_t input_index(0u);
std::string prev_btc_amount_string = "0.995";
uint64_t prev_satoshi_amount;
decode_base10(prev_satoshi_amount, prev_btc_amount_string, btc_decimal_places);

//Segwit Endorsement: Pass script_version::zero & prev input amount
endorsement sig_1;
script::create_endorsement(sig_1, my_secret_segwit, p2wpkh_script_code, tx, input_index, 0x01, script_version::zero, prev_satoshi_amount);
```
The `script::create_endorsement` method will serialise the sighash according to the `script_version` argument. For segwit inputs, this argument will be set to `script_version::zero` in order for the segwit specific sighash algorithm to be applied.

### P2WPKH Witness

Once the signature `endorsement` object is created, the witness object of the transaction can be constructed:

```c++
//Create Witness
//02 [signature] [publicKey]
data_stack witness_stack;
witness_stack.push_back(sig_1);
witness_stack.push_back(to_chunk(pubkey_segwit));
witness p2wpkh_witness(witness_stack);
tx.inputs()[0].set_witness(p2wpkh_witness);
```

Note that the `witness` class is constructed from a `data_stack`, which in turn is an alias of the `vector<data_chunk>` class. The `witness` object produces a serialised format which prepends a single-byte length prefix to each `data_chunk` in its stack.

The serialised encoding of the witness may appear similar to Bitcoin scripts, but differs in that it only consists of serialised data pushes with the aforementioned length prefixes.

This can be observed in the serialised witness from our example:

```c++
// Number of following witness elements for first input
02
// Length of 71-byte Endorsement
47
// Endorsement(DER Signature + Sighash Marker)
304402202f7cac3494e521018ae0be4ca18517639ef7c00658d42a9f938b2b344c8454e2022039a54218832fad5d14b331329d9042c51ee6be287e95e49ee5b96fda1f5ce13f01
// Length of 33-byte Public Key
21
// Even Public Key
026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf3
```

### Serialised P2WPKH Transaction

Finally, we can express our P2WPKH spending transaction in the following serialised form:
```c++
//Serialize TX
std::cout << encode_base16(tx.to_data(true,true)) << std::endl;
```

```c++
01000000000101109d2e41430bfdec7e6dfb02bf78b5827eeb717ef25210ff3203b0db8c76c9260000000000ffffffff01a032eb0500000000160014bbef244bcad13cffb68b5cef3017c742367555220247304402202f7cac3494e521018ae0be4ca18517639ef7c00658d42a9f938b2b344c8454e2022039a54218832fad5d14b331329d9042c51ee6be287e95e49ee5b96fda1f5ce13f0121026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf300000000
```
Parsing the serialised transaction with BX gives us an overview of our constructed segwit P2WPKH transaction.
```
BX tx-decode -f json 01000000000101109d2e41430bfdec7e6dfb02bf78b5827eeb717ef25210ff3203b0db8c76c9260000000000ffffffff01a032eb0500000000160014bbef244bcad13cffb68b5cef3017c742367555220247304402202f7cac3494e521018ae0be4ca18517639ef7c00658d42a9f938b2b344c8454e2022039a54218832fad5d14b331329d9042c51ee6be287e95e49ee5b96fda1f5ce13f0121026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf300000000
```
```json
{
    "transaction": {
        "hash": "568c2b0c82bad3c7056ba7837f00a75178024dc3fafb61e442e4ee18bb31a5c2",
        "inputs": [
            {
                "previous_output": {
                    "hash": "26c9768cdbb00332ff1052f27e71eb7e82b578bf02fb6d7eecfd0b43412e9d10",
                    "index": "0"
                },
                "script": "",
                "sequence": "4294967295",
                "witness": "[304402202f7cac3494e521018ae0be4ca18517639ef7c00658d42a9f938b2b344c8454e2022039a54218832fad5d14b331329d9042c51ee6be287e95e49ee5b96fda1f5ce13f01] [026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf3]"
            }
        ],
        "lock_time": "0",
        "outputs": [
            {
                "script": "zero [bbef244bcad13cffb68b5cef3017c74236755522]",
                "value": "99300000"
            }
        ],
        "version": "1"
    }
}
```
You can find the complete P2WPKH transaction script [here](https://github.com/libbitcoin/libbitcoin/wiki/Segwit-TX-Examples).
