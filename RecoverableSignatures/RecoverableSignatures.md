# Recoverable Signatures

An interesting property of ECSDA signatures is that together with the signed message, it is possible to derive a set of 4 possible pubkey's without any knowledge of the private key.

Given `r,s` and an index `i` ranging from 0 to 3, it is therefore possible to uniquely identify the public key used in the signature. The signature and recovery index are represented in the Libbitcoin type `recoverable_signature`.

```c++
recoverable_signature my_recoverable_sig;
sign_recoverable(my_recoverable_sig, my_secret, my_hash);
//Conversion: uint8_t->unsigned int for visible ASCII output
std::cout << unsigned(my_recoverable_sig.recovery_id);

//recover public key from recoverable signature
ec_compressed recovered_sig;
recover_public(recovered_sig, my_recoverable_sig, my_hash);

std::cout << (recovered_sig == my_pubkey);
```

Recoverable signatures are used in message signing by Bitcoin wallets to prove control over a given Bitcoin address.
