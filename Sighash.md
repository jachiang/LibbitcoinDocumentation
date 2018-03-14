# Sighash: Partial Transaction Signing

The op_codes `CHECKSIG`, `CHECKSIGVERIFY`, `CHECKMULTISIG`, `CHECKMULTISIGVERIFY` all verify signatures together with an off-stack one-byte sighash argument, which indicates which part of the transaction the signature specifically endorses.  

The input signature can endorse an entire transaction with a fixed set of inputs and outputs. The signature can also be selectively applied to specific outputs, so the transaction can include further outputs not endorsed by the signature.

| Sighash Type | Marker | Description |
| -------------|--------| ------------|
| ALL          | 0x01   | Sign all inputs and outputs |
| NONE         | 0x02   | Sign all inputs, outputs modifiable |
| SINGLE       | 0x03   | Sign all inputs and single output, other outputs modifiable |

## Sighash ALL

The illustration below illustrates the signature derivation for an input with the sighash set to `ALL`:

![SIGHASH ALL](https://ipfs.io/ipfs/QmbEYaLrNkCV8zHcvvHYPwHDA4f87Zp9w1pkrBMhxAQQWf)

Since the entire transaction is included in the sighash that is signed, we must first construct the complete set of inputs and outputs before the signatures are created.

*Note that for each input signing, we place the previous transaction's scriptPubKey or locking script in the scriptSig field of the respective input*

```c++
#include <bitcoin/bitcoin.hpp>
#include <string.h>

//Namespace
using namespace bc;
using namespace machine;    //opcode
using namespace chain;      //transaction, inputs, outputs, script
```

<!-- Example 1 -->
```c++
//Not shown:
//Build inputs (w/o scriptSig)
//Build outputs

//Finalise TX - can't be modified later
tx.inputs().push_back(input_0);   //first input
tx.inputs().push_back(input_1);   //second input
                                  //...nth input
tx.outputs().push_back(output_0); //first output
tx.outputs().push_back(output_1); //second output
                                  //...nth output

//Construct previous locking script of input_0 & input_1
script prev_script_0 = script::to_pay_key_hash_pattern(my_address0.hash());
script prev_script_1 = script::to_pay_key_hash_pattern(my_address1.hash());

//TX signature for input_0
endorsement sig_0;
uint8_t input0_index(0u);
script::create_endorsement(sig_0, my_secret0, prev_script_0, tx, input0_index, 0x01);

//TX signature for input_1
endorsement sig_1;
uint8_t input1_index(1u);
script::create_endorsement(sig_1, my_secret1, prev_script_1, tx, input1_index, 0x01);

//Construct unlocking script_0
operation::list sig_script_0;
sig_script_0.push_back(operation(sig_0));
sig_script_0.push_back(operation(to_chunk(pubkey0)));
script my_unlocking_script_0(sig_script_0);

//Construct unlocking script_1
operation::list sig_script_1;
sig_script_1.push_back(operation(sig_1));
sig_script_1.push_back(operation(to_chunk(pubkey1)));
script my_unlocking_script_1(sig_script_1);

//Add unlockingscript to TX
tx.inputs()[0].set_script(my_unlocking_script_0);
tx.inputs()[1].set_script(my_unlocking_script_1);

//SINGLE: We cannot modify TX after signing

//Print out
std::cout << encode_base16(tx.to_data()) << std::endl;
```

## Sighash NONE

Signing an input with the sighash marker set to `NONE` omits all outputs in the sighash which is signed.

![SIGHASH NONE](https://ipfs.io/ipfs/QmaEDF2XYYwMtHzd7jcqRcWbPnMXWFtfbsg9B2TzxAj3su)

We can therefore modify any output after the input signatures are created.
<!-- Example 2 -->
```c++
//Not shown:
//Build inputs (w/o scriptSig)
//Build outputs

//We only need to finalise inputs. Outputs can be modified after signing.
tx.inputs().push_back(input_0);   //first input
tx.inputs().push_back(input_1);   //second input
                                  //...nth input

//Construct previous locking script of input_0 & input_1
script prev_script_0 = script::to_pay_key_hash_pattern(my_address0.hash());
script prev_script_1 = script::to_pay_key_hash_pattern(my_address1.hash());

//TX signature for input_0
endorsement sig_0;
uint8_t input0_index(0u);
script::create_endorsement(sig_0, my_secret0, prev_script_0, tx, input0_index, 0x02);

//TX signature for input_1
endorsement sig_1;
uint8_t input1_index(1u);
script::create_endorsement(sig_1, my_secret1, prev_script_1, tx, input1_index, 0x02);

//Construct unlocking script_0
operation::list sig_script_0;
sig_script_0.push_back(operation(sig_0));
sig_script_0.push_back(operation(to_chunk(pubkey0)));
script my_unlocking_script_0(sig_script_0);

//Construct unlocking script_1
operation::list sig_script_1;
sig_script_1.push_back(operation(sig_1));
sig_script_1.push_back(operation(to_chunk(pubkey1)));
script my_unlocking_script_1(sig_script_1);

//Add unlockingscript to TX
tx.inputs()[0].set_script(my_unlocking_script_0);
tx.inputs()[1].set_script(my_unlocking_script_1);

//NONE: We can modify all outputs after signing
tx.outputs().push_back(output_0); //first output
tx.outputs().push_back(output_1); //second output
                                  //...nth output

//Print out
std::cout << encode_base16(tx.to_data()) << std::endl;
```

## Sighash SINGLE

A signature with the sighash marker set to `SINGLE` will only endorse or fix a single output with the same index as the input being signed. All other outputs can be modified later.

![SIGHASH SINGLE](https://ipfs.io/ipfs/QmNhHs2y7LhHx6b7xdCN2tJsB3j8WaZNXeuMqJiQjPEcz2)

In the following example, we will sign a transaction with 2 inputs and 2 outputs, but add further outputs later on.
<!-- Example 3 -->
```c++
//Not shown:
//Build inputs (w/o scriptSig)
//Build outputs

//We sign all inputs and single output with same index
tx.inputs().push_back(input_0);   //first input
tx.outputs().push_back(output_0); //first output
tx.inputs().push_back(input_1);   //second input
tx.outputs().push_back(output_1); //second output
                                  //...nth input
                                  //...nth output

//Construct previous locking script of input_0 & input_1
script prev_script_0 = script::to_pay_key_hash_pattern(my_address0.hash());
script prev_script_1 = script::to_pay_key_hash_pattern(my_address1.hash());

//TX signature for input_0
endorsement sig_0;
uint8_t input0_index(0u);
script::create_endorsement(sig_0, my_secret0, prev_script_0, tx, input0_index, 0x03);

//TX signature for input_1
endorsement sig_1;
uint8_t input1_index(1u);
script::create_endorsement(sig_1, my_secret1, prev_script_1, tx, input1_index, 0x03);

//Construct unlocking script_0
operation::list sig_script_0;
sig_script_0.push_back(operation(sig_0));
sig_script_0.push_back(operation(to_chunk(pubkey0)));
script my_unlocking_script_0(sig_script_0);

//Construct unlocking script_1
operation::list sig_script_1;
sig_script_1.push_back(operation(sig_1));
sig_script_1.push_back(operation(to_chunk(pubkey1)));
script my_unlocking_script_1(sig_script_1);

//Add unlockingscript to TX
tx.inputs()[0].set_script(my_unlocking_script_0);
tx.inputs()[1].set_script(my_unlocking_script_1);

//SINGLE: We can add additional outputs after signing
tx.outputs().push_back(output_2); //third output
                                  //...nth output

//Print out
std::cout << encode_base16(tx.to_data()) << std::endl;
```  

## Sighash Modifier: ANYONECANPAY

The sighash modifier `ANYONECANPAY` enables inputs to be modified after signing, and can be combined with the previous sighash markers.

| Sighash Type            | Marker | Description |
| ------------------------|--------| ------------|
| ALL + ANYONECANPAY      | 0x81   | Sign single input and all outputs, inputs modifiable |
| NONE + ANYONECANPAY     | 0x82   | Sign single input, inputs & outputs modifiable |
| SINGLE + ANYONECANPAY   | 0x83   | Sign single input, and single output, other inputs & outputs modifiable |

The following image illustrates the signature derivation for an input set to `SINGLE|ANYONECANPAY`

![SIGHASH SINGLE|ANYONECANPAY](https://ipfs.io/ipfs/QmZX7tS7tvtHnfKEokMTfq57yeweiEBQh8DHTqfb4fgET2)

**Sighash Example: NONE|ANYONECANPAY**  

We construct a transaction with a `NONE|ANYONECANPAY` input signature below"
<!-- Example 4 -->
```c++
//Not shown:
//Build input (NONE,ANYONECANPAY) signed at index 1
//Build outputs

//We only sign a single input
tx.inputs().push_back(input_0);

//Construct previous locking script of input_0
script prev_script_0 = script::to_pay_key_hash_pattern(my_address0.hash());

//TX signature for input_0
endorsement sig_0;
uint8_t input0_index(0u);
script::create_endorsement(sig_0, my_secret0, prev_script_0, tx, input0_index, 0x82);

//Construct unlocking script_0
operation::list sig_script_0;
sig_script_0.push_back(operation(sig_0));
sig_script_0.push_back(operation(to_chunk(pubkey0)));
script my_unlocking_script_0(sig_script_0);

//Add unlockingscript to TX
tx.inputs()[0].set_script(my_unlocking_script_0);

//ANYONECANPAY: We can modify other inputs after signing
//Important: input added here must include valid scriptSig!
//...and previously be signed with tx index = 1
tx.inputs().push_back(input_1);   //second input
                                  //...nth input

//NONE: We can modify all outputs after signing
tx.outputs().push_back(output_0); //first output
tx.outputs().push_back(output_1); //second output
                                  //...nth output

//Print out
std::cout << encode_base16(tx.to_data()) << std::endl;
```  

[**Previous** -- Building Transactions](https://github.com/libbitcoin/libbitcoin/wiki/Building-Transactions)  
[**Return to Index**](https://github.com/libbitcoin/libbitcoin/wiki)
