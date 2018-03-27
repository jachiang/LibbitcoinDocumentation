# P2WSH inside P2SH

## Sending to a P2SH(P2WSH) Output
Sending a transaction to a Pay-to-Witness-Script-Hash (P2WSH) address wrapped in a pay-to-script-hash address requires the construction of the following output script:

| Transaction Element | Script                 								                                |
| --------------------|-----------------------------------------------------------------------|
| Output Script       | `HASH160` `[20-byte hash160(<zero> <sha256(witness script)>)]` `EQUAL`|

The redeemscript of the P2SH output script shown above is `<zero> <sha256(WitnessScript)>)`. The construction of a Pay-to-Witness-Script-Hash (P2WSH) wrapped in a Pay-to-Script-Hash output mirrors that of a regular Pay-to-Script-Hash output.

```c++
// P2SH(P2WSH(multisig)) output.
// 2-of-3 Multisig witness script & script code.
uint8_t signatures(2u); //2 of 3
point_list points;
points.push_back(pubkey_witness_aware);
points.push_back(pubkey_witness_aware1);
points.push_back(pubkey_witness_aware2);
script witness_script = script::to_pay_multisig_pattern(signatures, points);

// P2WSH(Multisig) redeem script.
//    0 [34-byte sha256(witness script)]
hash_digest witness_script_hash = sha256_hash(witness_script.to_data(false));
operation::list p2wsh_operations;
p2wsh_operations.push_back(operation(opcode::push_size_0));
p2wsh_operations.push_back(operation(to_chunk(witness_script_hash)));
script p2wsh_script(p2wsh_operations);

// P2SH(P2WSH) output script.
//    hash160 [sha256(redeem script)] equal
short_hash redeem_script_hash = bitcoin_short_hash(p2wsh_script.to_data(false));
script output_script = script::to_pay_script_hash_pattern(redeem_script_hash);

// Build output.
std::string btc_amount = "0.648";
uint64_t output_amount;
decode_base10(output_amount, btc_amount, btc_decimal_places);
output p2sh_p2wpkh_output(output_amount, output_script);
```

If the spending of the previous transaction output(s) do not require the construction of witnesses, the rest of the transaction is built and signed according to the documentation sections [building transactions](https://github.com/libbitcoin/libbitcoin/wiki/Building-Transactions) and [sighash](https://github.com/libbitcoin/libbitcoin/wiki/Sighash-&-TX-Signing).

You can find the complete P2SH(P2WSH) example script [here](https://github.com/libbitcoin/libbitcoin/wiki/Examples:-Transactions-with-Input-Witnesses).

## Spending a P2SH(P2WSH) Output

Spending a P2WSH output requires constructing the transaction according to the following scheme.

| Transaction Element | Script                                               |
| --------------------|------------------------------------------------------|
| Output Script       | `According to destination address`                   |
| Input Script        | `[zero <34-byte sha256(witness script)>]`            |
| Script Code         | `witness script`                                     |
| Witness             | `[input script of witness script]` `[witness script]`|

Compared to spending a native P2WSH output, the input script is not left empty in a P2SH(P2WSH) transaction, but is instead populated with a single data push representing the serialised P2SH redeem script (specifically, a P2WSH script).

We construct the input object for our P2SH(P2WSH) output spending example.

```c++
// Omitted: Construction of output according to destination address.
```
```c++
// P2SH(P2WSH(Multisig)) input.
// Previous transaction hash.
std::string prev_tx =
    "40d5ecc46fb5f99971cda1fcd1bc0d465b0ce3f80176f30ef0b36f89c5568270";
hash_digest prev_tx_hash;
decode_hash(prev_tx_hash,prev_tx);
// Previous UXTO.
uint32_t index = 0;
output_point uxto_to_spend(prev_tx_hash, index);
// Build input object.
input p2sh_p2wsh_input;
p2sh_p2wsh_input.set_previous_output(uxto_to_spend);
p2sh_p2wsh_input.set_sequence(max_input_sequence);

// Build transaction.
transaction tx;
tx.set_version(1u);
tx.inputs().push_back(p2sh_p2wsh_input);
tx.outputs().push_back(p2wpkh_output);
tx.set_locktime(0u);
```

### Signing a Transaction with a P2SH(P2WPKH) Input

[BIP143](https://github.com/bitcoin/bips/blob/master/bip-0143.mediawiki) describes a witness-specific sighash generation algorithm for signatures evaluated by `CHECKSIG`, `CHECKSIGVERIFY`, `CHECKMULTISIG`, `CHECKMULTISIGVERIFY`.  

In particular, a script code and the previous input amount are required for the signature algorithm. The ScriptCode for signing a P2WSH transaction is the witness script, which in our example is the 2-of-3 multisig script.

```c++
// Witness script: multisig = Script code
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
std::string btc_amount_in = "0.648";
uint64_t input_amount;
decode_base10(input_amount, btc_amount_in, btc_decimal_places);
script::create_endorsement(sig0, my_secret_witness_aware, witness_script, tx,
      input0_index, sighash_algorithm::all, script_version::zero, input_amount);
script::create_endorsement(sig1, my_secret_witness_aware1, witness_script, tx,
      input0_index, sighash_algorithm::all, script_version::zero, input_amount);

```
The `script::create_endorsement` method will generate a sighash according to the witness script version parameter. For inputs requiring a witness of the current version, this argument will be set to version zero in order for the witness-specific sighash algorithm to be applied.

### P2SH(P2WSH) Input Script

The required input script for spending the P2SH(P2WSH) output is the redeem script. The input script does not include any signatures, as these will be included in the witness.

The redeem script is a single data push of the P2WSH script.

```c++
// P2SH output script.
//    0 <34-byte sha256(witness script)>
hash_digest witness_script_hash = sha256_hash(witness_script.to_data(false));
operation::list p2wsh_operations;
p2wsh_operations.push_back(operation(opcode::push_size_0));
p2wsh_operations.push_back(operation(to_chunk(witness_script_hash)));
script p2wsh_script(p2wsh_operations);

// Input script.
// Wrap redeem script as single data push (P2SH).
data_chunk p2sh_redeem_script_chunk = to_chunk(p2wsh_script.to_data(true));
script p2sh_redeem_script_wrapper(p2sh_redeem_script_chunk, false);
tx.inputs()[0].set_script(p2sh_redeem_script_wrapper);
```

### P2SH(P2WSH) Witness
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

The serialised encoding of the witness may appear similar to Bitcoin scripts, but differs in that it only consists of serialised data pushes with the aforementioned length prefixes.

This can be observed in the serialised witness from our example:

```c++
// Number of following witness elements for first input.
04
// Leading zero for multisig.
00
// Length of 1st 71-byte Endorsement.
47
// 1st Endorsement( DER Signature + sighash marker ).
30440220010d2854b86b90b7c33661ca25f9d9f15c24b88c5c4992630f77ff004b998fb802204106fc3ec8481fa98e07b7e78809ac91b6ccaf60bf4d3f729c5a75899bb664a501
// Length of 1st 71-byte Endorsement.
47
// 2nd Endorsement( DER Signature + sighash marker ).
3044022046d66321c6766abcb1366a793f9bfd0e11e0b080354f18188588961ea76c5ad002207262381a0661d66f5c39825202524c45f29d500c6476176cd910b1691176858701
// Length of witness script.
69
// 2-of-3 Multisig output script.
5221026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf32103befa190c0c22e2f53720b1be9476dcf11917da4665c44c9c71c3a2d28a933c352102be46dc245f58085743b1cc37c82f0d63a960efa43b5336534275fc469b49f4ac53ae
```

### Serialised P2SH(P2WSH) Transaction

Finally, we can express our P2SH(P2WSH) spending transaction in the following serialised form:

```C++
// Complete transaction
std::cout << encode_base16(tx.to_data(true,true)) << std::endl;
```
```C++
01000000000101708256c5896fb3f00ef37601f8e30c5b460dbcd1fca1cd7199f9b56fc4ecd5400000000023220020615ae01ed1bc1ffaad54da31d7805d0bb55b52dfd3941114330368c1bbf69b4cffffffff01603edb0300000000160014bbef244bcad13cffb68b5cef3017c7423675552204004730440220010d2854b86b90b7c33661ca25f9d9f15c24b88c5c4992630f77ff004b998fb802204106fc3ec8481fa98e07b7e78809ac91b6ccaf60bf4d3f729c5a75899bb664a501473044022046d66321c6766abcb1366a793f9bfd0e11e0b080354f18188588961ea76c5ad002207262381a0661d66f5c39825202524c45f29d500c6476176cd910b1691176858701695221026ccfb8061f235cc110697c0bfb3afb99d82c886672f6b9b5393b25a434c0cbf32103befa190c0c22e2f53720b1be9476dcf11917da4665c44c9c71c3a2d28a933c352102be46dc245f58085743b1cc37c82f0d63a960efa43b5336534275fc469b49f4ac53ae00000000
```
Parsing the serialised form with BX gives us an overview of our constructed P2SH(P2WSH) transaction.

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

You can find the complete P2SH(P2WSH) example script [here](https://github.com/libbitcoin/libbitcoin/wiki/Examples:-Transactions-with-Input-Witnesses).
