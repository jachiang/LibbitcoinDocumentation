#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>


using namespace bc;

void example1() {

  //Let us declare a compressed pubKey in Libbitcoin
  ec_compressed my_pubkey;
  std::cout << my_pubkey.size(); // 33

}

void example2(){

  //data_chunks can contain entropy of various sizes
  data_chunk my_entropy_16(16);      //128bits allocated
  data_chunk my_entropy_32(32);      //256bits allocated

  //we can now fill our data_chunk with random bits
  pseudo_random_fill(my_entropy_16); //128bit entropy
  pseudo_random_fill(my_entropy_32); //256bit entropy

}

void example3() {

  //Let's declare a byte_array of 16 bytes in memory
  constexpr size_t ArraySize = 16;
  byte_array<ArraySize> my_array;

  //pseudo_random_fill expects a data_chunk type input
  //yet byte_array isn't implicitly converted to data_chunk
  //pseudo_random_fill(my_array); //not ok

  //So we explicitly convert with our helper fct
  data_chunk my_chunk = to_chunk(my_array);
  pseudo_random_fill(my_chunk); //ok

  //We can transform my_chunk back to a byte_array
  byte_array<ArraySize> my_array2;
  my_array2 = to_array<ArraySize>(my_chunk);

}

void example4() {

    byte_array<4u> my_array {{0, 1, 2, 3}};
    data_chunk my_chunk {0, 1, 2, 3};

    auto first_hash = bitcoin_short_hash(my_array);  //ok
    auto second_hash = bitcoin_short_hash(my_chunk); //ok

    std::cout << (first_hash == second_hash); //True
};

int main() {

  std::cout << "Example 1: " << "\n";
  example1();
  std::cout << "\n";

  std::cout << "Example 2: " << "\n";
  example2();
  std::cout << "\n";

  std::cout << "Example 3: " << "\n";
  example3();
  std::cout << "\n";

  std::cout << "Example 4: " << "\n";
  example4();
  std::cout << "\n";

  return 0;

}
