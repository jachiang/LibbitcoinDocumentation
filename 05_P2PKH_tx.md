# Building Transactions

Libbitcoin provides the `chain::transactions` type to capture all necessary components to build a Bitcoin transaction in its serialised format for transmission on the Bitcoin network.

<!-- Image of Libbitcoin Transaction classes and subclasses -->
![Libbitcoin TX](https://ipfs.io/ipfs/Qmf1HPdedXhxTfKy2gYChhXSNvZydaaPdMQbPWpX8tfnd1)

The below table explicitly juxtaposes Bitcoin transaction components and the respective Libbitcoin types contained within `chain::transaction`.

| TX component             | Libbitcoin Type                   |
| -------------------------|-----------------------------------|
| Version                  | uint32_t                          |
| Number of inputs         | inputs.size()                     |
| Input: Previous Tx Hash  | input > output_point > hash_digest|
| Input: Previous Tx Index | input > output_point > uint32_t   |
| Input: ScriptSig         | input > script                    |
| Input: Sequence          | input > sequence                  |
| Number of outputs        | outputs.size()                    |
| Output: Value            | output > uint64_t                 |
| Output: script PubKey    | output > script                   |
| Locktime                 | uint32_t                          |

For Segwit enabled addresses, the segwit witness is represented by the `witness` type within the respective input.

| TX component               | Libbitcoin transaction  |
| ---------------------------|-------------------------|
| Inpput: Segregated Witness | input > witness         |


## Constructing the Transaction Object

Constructing a Libbitcoin transaction can be as simple as populating a `chain::transaction` object with the version, inputs, outputs and locktime.

```c++
//Namespace
using namespace bc;
using namespace machine;    //opcode
using namespace chain;      //transaction, inputs, outputs, script
```
<!-- Example 1 (Part 1) -->
```c++
//Instantiate tx object
transaction tx;

//We will populate this tx object in
//subsequent chapter sections below
```
### Transaction Version
The 4-byte version number indicates the set of consensus rules used to evaluate the validity of the transaction. Currently set to 1.

<!-- Example 1 (Part 2) -->
```c++
//version 1
uint32_t version = 1u;
tx.set_version(version);

//print version in serialised format
auto serialised_version = to_little_endian(tx.version());
std::cout << encode_base16(to_chunk(serialised_version));
```
### Transaction Input(s)
The inputs represent the unspent uxto's which are to be spent by the transaction. The libbitcoin class `chain::inputs` is a type alias of `std::vector<inputs>`, to which we can push single input uxto's which are to be added to the transaction.

Each input object requires the previous `transaction hash`, `uxto index`, `input sequence` and unlocking script or `sigScript`. We can add multiple inputs by constructing them individually and then pushing them into the `inputs` vector.

*Note: We initially omit `sigScripts` in the following example, as the actual transaction (w/o `sigScripts`) is required to produce a sighash that will be signed. This will be detailed in the sighash section later in this chapter*

<!-- Example 1 (Part 3) -->
```c++
//Previous TX hash
std::string prev_tx_string_0 = "1eb7195911f69f605cfdc44e05a10835448f875a01c429ccd6a4487621523b0d";
hash_digest prev_tx_hash_0;
decode_hash(prev_tx_hash_0,prev_tx_string_0);

//Previous UXTO index:
uint32_t index0 = 0;
output_point uxto_tospend_0(prev_tx_hash_0, index0);

//Build input_0 object
input input_0;
input_0.set_previous_output(uxto_tospend_0);
input_0.set_sequence(0xffffffff);

//Additional input objects can be created for additional inputs

//All input objects can then be added to transaction
tx.inputs().push_back(input_0);       //first input
//tx.inputs().push_back(input_1);     //second input
                                      //...nth input

//Unlocking script/scriptSig will be added later.
//See section on sigHash below
```
**Sequence**
In the example above, the sequence of the uxto is set to the maximum value of `0xffffffff`.
If transaction `locktime` or the opcode `checklocktimeverify` is required, the sequence is set to a lower value, usually to `0xFFFFFFFE`. Sequence values lower than `0XF0000000` are interpreted as having a locktime relative to the age of the input.

### Transaction Output(s)

The transaction outputs require only two pieces of information: the Satoshi amount of the output and the locking script or *scriptPubKey* of the output.

<!-- Example 1 (Part 4) -->
```c++
auto my_address_raw = "mmbmNXo7QZWU2WgWwvrtnyQrwffngWScFe";
payment_address my_address1(my_address_raw);

//Create Output locking script/scriptPubKey from template:
operation::list locking_script_0=script::to_pay_key_hash_pattern(my_address1.hash());

//Define Output amount
std::string btc_amount_string_0 = "1.295";
uint64_t satoshi_amount_0;
decode_base10(satoshi_amount_0, btc_amount_string_0, btc_decimal_places); //8 decimals = btc_decimal_places

//Create output_0 object
output output_0(satoshi_amount_0, locking_script_0);

//Above can be repeated for other outputs

//Add outputs to TX
tx.outputs().push_back(output_0);     //first output
//tx.outputs().push_back(output_1);   //second output
//tx.outputs().push_back(output_n);   //...nth output
```
In the example above, we have utilised the pay-to-public-key-hash template in libbitcoin.
We can also recreate the script p2pkh using individual opcodes.

<!-- Example 1 (Part 5) -->
```c++
//We rebuild our P2PKH script manually:
operation::list my_own_p2pkh;
my_own_p2pkh.push_back(operation(opcode::dup));
my_own_p2pkh.push_back(operation(opcode::hash160));
operation op_pubkey = operation(to_chunk(my_address1.hash()));
my_own_p2pkh.push_back(op_pubkey); //includes hash length prefix
my_own_p2pkh.push_back(operation(opcode::equalverify));
my_own_p2pkh.push_back(operation(opcode::checksig));

//The two operation lists are equivalent
std::cout << (my_own_p2pkh == locking_script_0) << std::endl;
```

### Locktime  

Locktime prevents a valid transaction from being broadcast or mined before a certain time. If the locktime value is nonzero and below 500 million, it represents the blockheight at which the locktime will complete. If the value is 500 million or above, it is interpreted as the Unix Epoch time at which the locktime will complete. The input sequence values for a transaction with active locktime are commonly set to `2^32 - 1` or `0xFFFFFFFE`.

### Witness

The witness contains the `witness` information in Segwit-enabled addresses. Please refer to the Segwit chapter for further details.

### Missing scriptSig  

In our example, we have now defined our transaction object, with the exception for the scriptSig or unlocking script needed for our input.

The scriptSig can only be inserted once we have completed the signature of a hash of the serialised transaction *without* the scriptSig as described in the next section.

## Signing the Transaction Sighash

The op_codes `CHECKSIG`, `CHECKSIGVERIFY`, `CHECKMULTISIG`, `CHECKMULTISIGVERIFY` all verify signatures together with an off-stack one-byte sighash argument, which indicates which part of the transaction the signature specifically endorses.  

The input signature can endorse an entire transaction with a fixed set of inputs and outputs. The signature can also be selectively applied to specific outputs, so the transaction can include further outputs not endorsed by the signature.

| Sighash Type | Marker | Description |
| -------------|--------| ------------|
| ALL          | 0x01   | Sign all inputs and outputs |
| NONE         | 0x02   | Sign all inputs, outputs modifiable |
| SINGLE       | 0x03   | Sign all inputs and single output, other outputs modifiable |

**Sighash ALL**  

The illustration below illustrates the signature derivation for an input with the sighash set to `ALL`:

![SIGHASH ALL](https://ipfs.io/ipfs/QmbEYaLrNkCV8zHcvvHYPwHDA4f87Zp9w1pkrBMhxAQQWf)

Since the entire transaction is included in the sighash that is signed, we must first construct the complete set of inputs and outputs before the signatures are created.

*Note that for each input signing, we place the previous transaction's scriptPubKey or locking script in the scriptSig field of the respective input*

<!-- Example 2 -->
```c++
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
script::create_endorsement(sig_0, my_secret0, prev_script_0, tx, input0_index, 0x00);

//TX signature for input_1
endorsement sig_1;
uint8_t input1_index(1u);
script::create_endorsement(sig_1, my_secret1, prev_script_1, tx, input1_index, 0x00);

//Construct unlocking script 0
operation::list sig_script_0;
sig_script_0.push_back(operation(sig_0));
sig_script_0.push_back(operation(to_chunk(pubkey2)));
script my_unlocking_script_0(sig_script_0);

//Construct unlocking script 1
operation::list sig_script_1;
sig_script_1.push_back(operation(sig_1));
sig_script_1.push_back(operation(to_chunk(pubkey3)));
script my_unlocking_script_1(sig_script_1);

//Add unlockingscript to TX
tx.inputs()[0].set_script(my_unlocking_script_0);
tx.inputs()[1].set_script(my_unlocking_script_1);

//Print out
std::cout << encode_base16(tx.to_data()) << std::endl;
```

**Sighash NONE**  

Signing an input with the sighash marker set to `NONE` omits all outputs in the sighash which is signed.

![SIGHASH NONE](https://ipfs.io/ipfs/QmaEDF2XYYwMtHzd7jcqRcWbPnMXWFtfbsg9B2TzxAj3su)

We can therefore modify any output after the input signatures are created.
<!-- Example 3 -->
```c++
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

**Sighash SINGLE**  

A signature with the sighash marker set to `SINGLE` will only endorse or fix a single output with the same index as the input being signed. All other outputs can be modified later.

![SIGHASH SINGLE](https://ipfs.io/ipfs/QmNhHs2y7LhHx6b7xdCN2tJsB3j8WaZNXeuMqJiQjPEcz2)

In the following example, we will sign a transaction with 2 inputs and 2 outputs, but add further outputs later on.
```c++
//SIGHASH SINGLE example
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

//SINGLE: We can add additional outputs after signing
tx.outputs().push_back(output_2); //third output
                                  //...nth output

//Print out
std::cout << encode_base16(tx.to_data()) << std::endl;
```  

**Sighash Modifier: ANYONECANPAY**  

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

```c++
//We only sign a single output
tx.inputs().push_back(input_0);   //first input

//Construct previous locking script of input_0 & input_1
script prev_script_0 = script::to_pay_key_hash_pattern(my_address0.hash());

//TX signature for input_0
endorsement sig_0;
uint8_t input0_index(0u);
script::create_endorsement(sig_0, my_secret0, prev_script_0, tx, input0_index, 0x81);

//Construct unlocking script 0
operation::list sig_script_0;
sig_script_0.push_back(operation(sig_0));
sig_script_0.push_back(operation(to_chunk(pubkey2)));
script my_unlocking_script_0(sig_script_0);

//Add unlockingscript to TX
tx.inputs()[0].set_script(my_unlocking_script_0);

//Omitted: Construction of other inputs & outputs:
//input_1, output_0, output_1

//ANYONECANPAY: We can modify other inputs after signing
//Important: inputs added here must include valid scriptSig!
tx.inputs().push_back(input_1);   //second input
                                  //...nth input

//NONE: We can modify all outputs after signing
tx.outputs().push_back(output_0); //first output
tx.outputs().push_back(output_1); //second output
                                  //...nth output

//Print out final transaction
std::cout << encode_base16(tx.to_data()) << std::endl;
```  
[**Previous** -- Addresses & HD Wallets ](https://github.com/libbitcoin/libbitcoin/wiki/Addresses-&-HD-Wallets)  
[**Return to Index**](https://github.com/libbitcoin/libbitcoin/wiki)
