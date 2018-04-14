# Examples: Serialised Data

All examples from the data serialisation documentation chapter are shown here in full. The specific examples referenced in the subsections are wrapped in the functions listed below.

**Compressed public key**
* create_public_key();

**Data Chunk (Dynamic Length Data)**
* create_random_data_chunk();

**Byte Array (Fixed Length Data)**
* create_random_byte_array();

**Data Slice Parameters**
* hash_data_slice();  

**Libbitcoin API:** Version 3.

Script below is ready-to-compile: `g++ -std=c++11 -o serialised_data serialised_data_examples.cpp $(pkg-config --cflags libbitcoin --libs libbitcoin)`

```c++
#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>


using namespace bc;

void create_public_key() {

  // Declare a compressed public key.
  ec_compressed my_pubkey;
  // It will have a length of 33 single-byte elements.
  std::cout << my_pubkey.size() << std::endl; // Prints 33.

}

void create_random_data_chunk(){

  // Data chunks can be instantiated with arbitrary sizes
  data_chunk my_entropy_16(16);      //128bits allocated
  data_chunk my_entropy_32(32);      //256bits allocated

  // Fill data chunk object with entropy
  pseudo_random_fill(my_entropy_16); //128bit entropy
  pseudo_random_fill(my_entropy_32); //256bit entropy

}

void create_random_byte_array() {

  // Declare a byte array of length 16
  constexpr size_t my_array_size = 16u;
  byte_array<my_array_size> my_array;

  // The pseudo random fill function expects a data_chunk type input.
  // We cannot pass a parameter of type byte array.
  // pseudo_random_fill(my_array); // not ok.

  // So we first copy the byte data into a data chunk
  // ...with the to_chunk helper function.
  data_chunk my_chunk = to_chunk(my_array);
  pseudo_random_fill(my_chunk); // ok.

  // We can copy the data back into a byte array with the to_array helper function.
  byte_array<my_array_size> my_array2;
  my_array2 = to_array<my_array_size>(my_chunk);

}

void hash_data_slice() {

  byte_array<4u> my_array {{0, 1, 2, 3}};
  data_chunk my_chunk {0, 1, 2, 3};

  auto first_hash = bitcoin_short_hash(my_array);  //ok
  auto second_hash = bitcoin_short_hash(my_chunk); //ok

  std::cout << (first_hash == second_hash) << std::endl; //True

};

int main() {

  create_public_key();

  create_random_data_chunk();

  create_random_byte_array();

  hash_data_slice();

  return 0;

}
```
