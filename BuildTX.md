# Building Transactions

Libbitcoin provides the `chain::transactions` type to capture all necessary components to build a Bitcoin transaction in its serialised format for transmission on the Bitcoin network.

<!-- Image of Libbitcoin Transaction classes and subclasses -->
![Libbitcoin TX](https://ipfs.io/ipfs/Qmf1HPdedXhxTfKy2gYChhXSNvZydaaPdMQbPWpX8tfnd1)

The below table explicitly juxtaposes Bitcoin transaction components and the respective Libbitcoin types used in `chain::transaction`.

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
#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>

//Namespace
using namespace bc;
using namespace wallet;   
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
std::string prev_tx_string_0 = "ca05e6c14fe816c93b91dd4c8f00e60e4a205da85741f26326d6f21f9a5ac5e9";
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
```
**Sequence**  
In the example above, the sequence of the uxto is set to the maximum value of `0xffffffff`.
If transaction `locktime` or the opcode `checklocktimeverify` is required, the sequence is set to a lower value, usually to `0xFFFFFFFE`. Sequence values lower than `0XF0000000` are interpreted as having a locktime relative to the age of the input.

### Transaction Output(s)

The transaction outputs require only two pieces of information: the Satoshi amount of the output and the locking script or *scriptPubKey* of the output.

<!-- Example 1 (Part 4) -->
```c++
//Destination Address
auto my_address_raw = "mmbmNXo7QZWU2WgWwvrtnyQrwffngWScFe";
payment_address my_address1(my_address_raw);

//Create Output locking script/scriptPubKey from template:
operation::list locking_script_0=script::to_pay_key_hash_pattern(my_address1.hash());

//Define Output amount
std::string btc_amount_string_0 = "1.295";
uint64_t satoshi_amount_0;
decode_base10(satoshi_amount_0, btc_amount_string_0, btc_decimal_places); // btc_decimal_places = 8

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

### Endorsing the Transaction

The transaction in our examples has now been constructed. However, the input has yet to be signed: the scriptSig field in the input object is still empty.

We illustrate the signing of the entire transaction below, where the Sighash marker is set to ALL.

Once the unlocking script or scriptSig is constructed, it is inserted into the `transaction` object input, and can then be serialised for broadcasting.

<!-- Example 1 (Part 6) -->
```c++
//Signer: Secret > Pubkey > Address
auto my_secret0 = base16_literal("3eec08386d08321cd7143859e9bf4d6f65a71d24f37536d76b4224fdea48009f");
ec_private my_private0(my_secret0, ec_private::testnet, true);
ec_compressed pubkey0= my_private0.to_public().point();
payment_address my_address0 = my_private0.to_payment_address();

//Signature
endorsement sig_0;
script prev_script_0 = script::to_pay_key_hash_pattern(my_address0.hash());
uint8_t input0_index(0u);
script::create_endorsement(sig_0, my_secret0, prev_script_0, tx, input0_index, 0x01);

//Create sigScript
operation::list sig_script_0;
sig_script_0.push_back(operation(sig_0));
sig_script_0.push_back(operation(to_chunk(pubkey0)));
script my_unlocking_script_0(sig_script_0);

//Add sigScript to first input in transaction
tx.inputs()[0].set_script(my_unlocking_script_0);

//Print serialised transaction
std::cout << encode_base16(tx.to_data()) << std::endl;
```

[**Next** -- Sighash: Partial TX Signing](https://github.com/libbitcoin/libbitcoin/wiki)  
[**Previous** -- Addresses & HD Wallets](https://github.com/libbitcoin/libbitcoin/wiki/Addresses-&-HD-Wallets)  
[**Return to Index**](https://github.com/libbitcoin/libbitcoin/wiki)
