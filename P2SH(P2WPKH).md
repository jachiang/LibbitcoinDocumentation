# Segwit: P2SH(P2WPKH)

## Sending to P2SH(P2WPKH)
Sending a transaction to a Pay-to-Witness-Public-Key-Hash (P2WPKH) address wrapped in a P2SH address requires the construction of the following scriptPubKey:

| TX Element 	 | Script/Serialization 									                  |
| -------------|----------------------------------------------------------|
| ScriptPubKey | `HASH160` `[20-byte hash160(P2WPKH(PubKeyHash))]` `EQUAL`|

The construction of a `P2SH(P2WPKH)` output mirrors that of a regular `P2SH` scriptPubKey. The `redeemscript` of the P2SH scriptPubKey shown above is the `P2WPKH(PubKeyHash)` script.

```c++
// P2SH(P2WPKH) Output
// P2SH Redeemscript = P2WPKH(PubKeyHash)
// 0 [20-byte publicKeyHash]
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
```
If the input(s) are non-segwit, rest of the transaction is built and signed according to the documentation sections [building transactions](https://github.com/libbitcoin/libbitcoin/wiki) and [sighash](https://github.com/libbitcoin/libbitcoin/wiki).

The complete P2SH(P2WPKH) example script can be found [here](https://github.com/libbitcoin/libbitcoin/wiki).

## Sending from P2SH(P2WPKH)  

Spending a P2SH(P2WPKH) output requires constructing the transaction according to the following scheme.

| TX Element   | Script/Serialization                                                    |
| -------------|-------------------------------------------------------------------------|
| ScriptPubKey | `According to destination address`                                      |
| ScriptSig    | `[zero <20-byte publicKeyHash>]`                                        |
| ScriptCode   | `DUP` `HASH160` `[20-byte hash160(PublicKey)]` `EQUALVERIFY` `CHECKSIG` |
| Witness      | `[Signature]` `[PublicKey]`                                             |

Compared to spending a native P2WPKH output, the ScriptSig is not left empty in a P2SH(P2WPKH) transaction, but is instead populated with a single data push representing the serialised P2SH redeemscript (specifically, a P2WPKH script).

We construct the `input` object for our `P2SH(P2WPKH)` spending example.

```c++
// Omitted: Construction of output according to destination address
```

```c++
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

//Build TX
transaction tx;
tx.set_version(1u);
tx.inputs().push_back(p2sh_p2wpkh_input);
tx.outputs().push_back(p2pkh_output);
```

### Signing a P2SH(P2WPKH) transaction
[BIP143](https://github.com/bitcoin/bips/blob/master/bip-0143.mediawiki) describes a segwit-specific sighash generation algorithm for signatures evaluated by `CHECKSIG`, `CHECKSIGVERIFY`, `CHECKMULTISIG`, `CHECKMULTISIGVERIFY`.  

In particular, a ScriptCode and the previous input amount are required for the signature algorithm. The ScriptCode for spending a P2WPKH is simply a `P2PKH(pubKeyHash)` script.

```c++
//Create Segwit Signature

//ScriptCode:
script script_code = script::to_pay_key_hash_pattern(
    bitcoin_short_hash(pubkey_segwit));

//Previous Input Amount:
uint8_t input_index(0u);
std::string btc_amount_string_in = "1.298";
uint64_t satoshi_amount_in;
decode_base10(satoshi_amount_in, btc_amount_string_in, btc_decimal_places);

//Segwit Endorsement: Pass script_version::zero & prev input amount
endorsement sig;
script::create_endorsement(sig, my_secret_segwit, script_code, tx, input_index,
    0x01, script_version::zero, satoshi_amount_in); //turn on witness
```
The `script::create_endorsement` method will serialise the sighash according to the `script_version` argument. For segwit inputs, this argument will be set to `script_version::zero` in order for the segwit specific sighash algorithm to be applied.

### P2SH(P2WPKH) ScriptSig
The ScriptSig for spending the P2SH(P2WPKH) will be the redeemscript, but does not include any signatures or witnesses.

The redeemscript is the P2WPKH script `[zero <20-byte publicKeyHash>]` as a single data push.

```c++
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
```

### P2SH(P2WPKH) Witness

The witness of a P2SH(P2WPKH) input is identical to that of a native P2WPKH input.  

```c++
// Witness:
// 02 [signature] [publicKey]
data_stack witness_stack;
witness_stack.push_back(sig);
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
//Endorsement(DER Signature + Sighash Marker)
304402207ecbb796a2bc706d90e2ed7efb58f59822bdc4c253b91f6eecd26ca5df1a6bb60220700b737f3c49b2f21bb228fadeab786e2ac78fd87890ede3f5d299e81880d96301
// Length of 33-byte Public Key
21
// Even Public Key
026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf3
```

### Serialised P2SH(P2WPKH) Transaction

Finally, we can express our P2SH(P2WPKH) spending transaction in the following serialised form:

```C++
// Serialize TX
std::cout << encode_base16(tx.to_data(true,true)) << std::endl;
```
```C++
01000000000101edff78210438890748b3b60560dde8b6f06823cd12f7bdd72b6fca7e02a9318200000000171600149a19a31c2fda7d0c30215ec954a20a542aa84ad3ffffffff016003b807000000001976a914bbef244bcad13cffb68b5cef3017c7423675552288ac0247304402207ecbb796a2bc706d90e2ed7efb58f59822bdc4c253b91f6eecd26ca5df1a6bb60220700b737f3c49b2f21bb228fadeab786e2ac78fd87890ede3f5d299e81880d9630121026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf300000000
```
Parsing the serialised transaction with BX gives us an overview of our constructed segwit P2SH(P2WPKH) transaction.
```
BX tx-decode -f json 01000000000101edff78210438890748b3b60560dde8b6f06823cd12f7bdd72b6fca7e02a9318200000000171600149a19a31c2fda7d0c30215ec954a20a542aa84ad3ffffffff016003b807000000001976a914bbef244bcad13cffb68b5cef3017c7423675552288ac0247304402207ecbb796a2bc706d90e2ed7efb58f59822bdc4c253b91f6eecd26ca5df1a6bb60220700b737f3c49b2f21bb228fadeab786e2ac78fd87890ede3f5d299e81880d9630121026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf300000000
```
```json
{
    "transaction": {
        "hash": "e85f4cbe53a60ba027ba565653915499d9b6a7e824e260f23cd69bfab1992624",
        "inputs": [
            {
                "address_hash": "dcdc2f89b96c420751e3750da7d5073a81b16946",
                "previous_output": {
                    "hash": "8231a9027eca6f2bd7bdf712cd2368f0b6e8dd6005b6b348078938042178ffed",
                    "index": "0"
                },
                "script": "[00149a19a31c2fda7d0c30215ec954a20a542aa84ad3]",
                "sequence": "4294967295",
                "witness": "[304402207ecbb796a2bc706d90e2ed7efb58f59822bdc4c253b91f6eecd26ca5df1a6bb60220700b737f3c49b2f21bb228fadeab786e2ac78fd87890ede3f5d299e81880d96301] [026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf3]"
            }
        ],
        "lock_time": "0",
        "outputs": [
            {
                "address_hash": "bbef244bcad13cffb68b5cef3017c74236755522",
                "script": "dup hash160 [bbef244bcad13cffb68b5cef3017c74236755522] equalverify checksig",
                "value": "129500000"
            }
        ],
        "version": "1"
    }
}
```

You can find the complete P2SH(P2WPKH) example script [here](https://github.com/libbitcoin/libbitcoin/wiki).
