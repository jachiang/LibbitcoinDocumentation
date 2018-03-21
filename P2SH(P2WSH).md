# Segwit: P2SH(P2WSH)

## Sending to P2SH(P2WSH)
Sending a transaction to a Pay-to-Witness-Script-Hash (P2WSH) address wrapped in a P2SH address requires the construction of the following scriptPubKey:

| TX Element | Script/Serialization |
| -------------|--------| ------------|
| ScriptPubKey | `HASH160` `[20-byte hash160(WitnessScript)]` `EQUAL`|

The `redeemscript` of the P2SH `ScriptPubKey` shown above is the `WitnessScript` script. Therefore, the construction of a `P2SH(P2WSH)` output mirrors that of a regular `P2SH` example.

```c++
// P2SH(P2WSH(MultSig)) Output
// 2-of-3 MultiSig WitnessScript & ScriptCode:
uint8_t signatures(2u); //2 of 3
point_list points;
points.push_back(pubkey_segwit);
points.push_back(pubkey_segwit1);
points.push_back(pubkey_segwit2);
script witness_script = script::to_pay_multisig_pattern(signatures, points);

// P2WSH(MultiSig) Redeemscript:
//    0 [34-byte sha256(WitnessScript)]
hash_digest witness_script_hash = sha256_hash(witness_script.to_data(false));
operation::list p2wsh_oplist;
p2wsh_oplist.push_back(operation(opcode::push_size_0));
p2wsh_oplist.push_back(operation(to_chunk(witness_script_hash)));
script p2wsh_script(p2wsh_oplist);

// P2SH(P2WSH) scriptPubKey:
//    hash160 [sha256(Redeemscript)] equal
short_hash redeemscript_hash = bitcoin_short_hash(p2wsh_script.to_data(false));
script script_pubkey = script::to_pay_script_hash_pattern(redeemscript_hash);

// Build output:
std::string btc_amount_string = "0.648";
uint64_t satoshi_amount;
decode_base10(satoshi_amount, btc_amount_string, btc_decimal_places);
output p2sh_p2wpkh_output(satoshi_amount, script_pubkey);
```

The rest of the transaction is built and signed in the same fashion as described in [building transactions](https://github.com/libbitcoin/libbitcoin/wiki) and [sighash](https://github.com/libbitcoin/libbitcoin/wiki) documentation sections if the input(s) are non-segwit.

You can find the complete P2SH(P2WSH) example script [here](https://github.com/libbitcoin/libbitcoin/wiki).

## Sending from P2SH(P2WSH)
Spending a P2SH(P2WSH) output requires adherence to the following transaction scheme.

| TX Element   | Script/Serialization                                                       |
| -------------|----------------------------------------------------------------------------|
| ScriptPubKey | `According to destination address`                                         |
| ScriptSig    | `[zero <34-byte sha256(WitnessScript)>]`                                   |
| ScriptCode   | `[WitnessScript]`                                                          |
| Witness      | `[Signature(s)]` `[WitnessScript]`                                         |

Compared to spending a native P2WPKH output, the ScriptSig is not left empty in a P2SH(P2WPKH) transaction, but is instead populated with a single data push representing the serialised P2SH redeemscript (specifically, a P2WPKH script).

We construct the `input` object for our `P2SH(P2WPKH)` spending example.

```c++
// Omitted: Construction of output according to destination address
```

```c++
// P2SH(P2WSH(MultiSig)) Input
// Previous TX hash
std::string prev_tx_string = "40d5ecc46fb5f99971cda1fcd1bc0d465b0ce3f80176f30ef0b36f89c5568270";
hash_digest prev_tx_hash;
decode_hash(prev_tx_hash,prev_tx_string);
// Previous UXTO:
uint32_t index = 0;
output_point uxto_tospend(prev_tx_hash, index);
// Build input object
input p2sh_p2wsh_input;
p2sh_p2wsh_input.set_previous_output(uxto_tospend);
p2sh_p2wsh_input.set_sequence(0xffffffff);

//Build TX
transaction tx;
tx.set_version(1u);
tx.inputs().push_back(p2sh_p2wsh_input);
tx.outputs().push_back(p2wpkh_output);
tx.set_locktime(0u);
```

We can also (re)construct our 2-of-3 multisig WitnessScript, which we will need for signing and later to construct our ScriptSig.

```C++

```

### Signing a P2SH(P2WPKH) transaction
[BIP143](https://github.com/bitcoin/bips/blob/master/bip-0143.mediawiki) describes a segwit-specific sighash generation algorithm for signatures evaluated by `CHECKSIG`, `CHECKSIGVERIFY`, `CHECKMULTISIG`, `CHECKMULTISIGVERIFY`.  

In particular, a ScriptCode and the previous input amount are required for the signature algorithm. The ScriptCode for signing a P2SH(P2WPKH) transaction is the `WitnessScript` script.

```c++
//WitnessScript: MultiSig = ScriptCode
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
std::string btc_amount_string_in = "0.648";
uint64_t satoshi_amount_in;
decode_base10(satoshi_amount_in, btc_amount_string_in, btc_decimal_places);
script::create_endorsement(sig0, my_secret_segwit, witness_script, tx, input0_index, 0x01, script_version::zero, satoshi_amount_in);
script::create_endorsement(sig1, my_secret_segwit1, witness_script, tx, input0_index, 0x01, script_version::zero, satoshi_amount_in);

```
>The `script::create_endorsement` method will serialise the sighash according to the `script_version` argument. For segwit inputs, this argument will be set to `script_version::zero` in order for the correct sighash algorithm to be applied.

### P2SH(P2WSH) ScriptSig
The ScriptSig for spending the P2SH(P2WPKH) will be the redeemscript, but does not include any signatures or witnesses.

The ScriptSig is `[zero [32-byte sha256(WitnessScript)]]` as a single data push.

```c++
//P2SH scriptPubKey:
//    0 <32-byte sha256(WitnessScript)>
hash_digest witness_script_hash = sha256_hash(witness_script.to_data(false));
operation::list p2wsh_oplist;
p2wsh_oplist.push_back(operation(opcode::push_size_0));
p2wsh_oplist.push_back(operation(to_chunk(witness_script_hash)));
script p2wsh_script(p2wsh_oplist);

//ScriptSig
//Wrap redeemscript as single data push (P2SH)
data_chunk p2sh_redeemscript_chunk = to_chunk(p2wsh_script.to_data(true)); //true: include size prefix
script p2sh_redeemscript_wrapper(p2sh_redeemscript_chunk, false); //false: interpret as single data push
tx.inputs()[0].set_script(p2sh_redeemscript_wrapper);
```

### P2SH(P2WSH) Witness
Once the signature `endorsement` object is created, the witness object of the transaction must be populated with the required signatures and 2-of-3 multisig script used in our example.  

Note that we add an additional empty `data_chunk` to represent the leading 0 required for multisig unlocking scripts. Once the `witness` object is serialised, each `data_chunk` entry will be prepended with a single-byte length prefix to each `data_chunk` in its stack. The leading empty `data_chunk` will be serialised to `OxOO`, which is what we need for our multisig unlocking script.

```c++
//Create Witness
data_stack witness_stack;
data_chunk empty_chunk;
witness_stack.push_back(empty_chunk); //works
witness_stack.push_back(sig0);
witness_stack.push_back(sig1);
witness_stack.push_back(witness_script.to_data(false));
witness p2wsh_witness(witness_stack);
tx.inputs()[0].set_witness(p2wsh_witness);
```
>The serialised encoding of the witness may appear similar to Bitcoin scripts, but only consists of serialised data pushes, each with a single-byte length prefix.

This can be observed in the witness from our example, which is serialised as the following:

```c++
// Number of following witness elements for first input
04
// Leading zero for multisig
00
// Length of 1st 71-byte Endorsement
47
// 1st Endorsement(DER Signature + Sighash Marker)
30440220010d2854b86b90b7c33661ca25f9d9f15c24b88c5c4992630f77ff004b998fb802204106fc3ec8481fa98e07b7e78809ac91b6ccaf60bf4d3f729c5a75899bb664a501
// Length of 1st 71-byte Endorsement
47
// 2nd Endorsement(DER Signature + Sighash Marker)
3044022046d66321c6766abcb1366a793f9bfd0e11e0b080354f18188588961ea76c5ad002207262381a0661d66f5c39825202524c45f29d500c6476176cd910b1691176858701
// Length of Witness Script
69
// 2-of-3 Multisig ScriptPubKey
5221026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf32103befa190c0c22e2f53720b1be9476dcf11917da4665c44c9c71c3a2d28a933c352102be46dc245f58085743b1cc37c82f0d63a960efa43b5336534275fc469b49f4ac53ae
```

---

Finally, we can express our P2SH(P2WPKH) spending transaction in the following serialised form:

```C++
// Serialize TX
std::cout << encode_base16(tx.to_data(true,true)) << std::endl;
```
```C++
01000000000101708256c5896fb3f00ef37601f8e30c5b460dbcd1fca1cd7199f9b56fc4ecd5400000000023220020615ae01ed1bc1ffaad54da31d7805d0bb55b52dfd3941114330368c1bbf69b4cffffffff01603edb0300000000160014bbef244bcad13cffb68b5cef3017c7423675552204004730440220010d2854b86b90b7c33661ca25f9d9f15c24b88c5c4992630f77ff004b998fb802204106fc3ec8481fa98e07b7e78809ac91b6ccaf60bf4d3f729c5a75899bb664a501473044022046d66321c6766abcb1366a793f9bfd0e11e0b080354f18188588961ea76c5ad002207262381a0661d66f5c39825202524c45f29d500c6476176cd910b1691176858701695221026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf32103befa190c0c22e2f53720b1be9476dcf11917da4665c44c9c71c3a2d28a933c352102be46dc245f58085743b1cc37c82f0d63a960efa43b5336534275fc469b49f4ac53ae00000000
```
Parsing the serialised transaction with BX gives us an overview of our constructed segwit P2SH(P2WSH) transaction.

```
BX tx-decode -f json 01000000000101708256c5896fb3f00ef37601f8e30c5b460dbcd1fca1cd7199f9b56fc4ecd5400000000023220020615ae01ed1bc1ffaad54da31d7805d0bb55b52dfd3941114330368c1bbf69b4cffffffff01603edb0300000000160014bbef244bcad13cffb68b5cef3017c7423675552204004730440220010d2854b86b90b7c33661ca25f9d9f15c24b88c5c4992630f77ff004b998fb802204106fc3ec8481fa98e07b7e78809ac91b6ccaf60bf4d3f729c5a75899bb664a501473044022046d66321c6766abcb1366a793f9bfd0e11e0b080354f18188588961ea76c5ad002207262381a0661d66f5c39825202524c45f29d500c6476176cd910b1691176858701695221026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf32103befa190c0c22e2f53720b1be9476dcf11917da4665c44c9c71c3a2d28a933c352102be46dc245f58085743b1cc37c82f0d63a960efa43b5336534275fc469b49f4ac53ae00000000
```

```json
{
    "transaction": {
        "hash": "17f7eaee671b3840eb192d9f8bd73a6ae54ff88850ec0c3ec3a83da2b21bed2e",
        "inputs": [
            {
                "address_hash": "23b1651b100da52aa29902ccc91edab329efeb12",
                "previous_output": {
                    "hash": "40d5ecc46fb5f99971cda1fcd1bc0d465b0ce3f80176f30ef0b36f89c5568270",
                    "index": "0"
                },
                "script": "[0020615ae01ed1bc1ffaad54da31d7805d0bb55b52dfd3941114330368c1bbf69b4c]",
                "sequence": "4294967295",
                "witness": "[] [30440220010d2854b86b90b7c33661ca25f9d9f15c24b88c5c4992630f77ff004b998fb802204106fc3ec8481fa98e07b7e78809ac91b6ccaf60bf4d3f729c5a75899bb664a501] [3044022046d66321c6766abcb1366a793f9bfd0e11e0b080354f18188588961ea76c5ad002207262381a0661d66f5c39825202524c45f29d500c6476176cd910b1691176858701] [5221026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf32103befa190c0c22e2f53720b1be9476dcf11917da4665c44c9c71c3a2d28a933c352102be46dc245f58085743b1cc37c82f0d63a960efa43b5336534275fc469b49f4ac53ae]"
            }
        ],
        "lock_time": "0",
        "outputs": [
            {
                "script": "zero [bbef244bcad13cffb68b5cef3017c74236755522]",
                "value": "64700000"
            }
        ],
        "version": "1"
    }
}
```

You can find the complete P2SH(P2WSH) example script [here](https://github.com/libbitcoin/libbitcoin/wiki).
