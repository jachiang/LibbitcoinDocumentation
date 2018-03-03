#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>


using namespace bc;

void example1() {

  //Let us declare a compressed pubKey in Libbitcoin
  ec_compressed myPubKey;
  std::cout << myPubKey.size(); // 33

}

void example2(){

  //data_chunks can contain entropy of various sizes
  data_chunk myEntropy16(16);      //128bits allocated
  data_chunk myEntropy32(32);      //256bits allocated

  //we can now fill our data_chunk with random bits
  pseudo_random_fill(myEntropy16); //128bit entropy
  pseudo_random_fill(myEntropy32); //256bit entropy

}

void example3() {

  //Let's declare a byte_array of 16 bytes in memory
  constexpr size_t arraySize = 16;
  byte_array<arraySize> myArray;

  //pseudo_random_fill expects a data_chunk type input
  //yet byte_array isn't implicitly converted to data_chunk
  //pseudo_random_fill(myArray); //not ok

  //So we explicitly convert with our helper fct
  data_chunk myChunk = to_chunk(myArray);
  pseudo_random_fill(myChunk); //ok

  //We can transform myChunk back to a byte_array
  byte_array<arraySize> myArray2;
  myArray2 = to_array<arraySize>(myChunk);

}

void example4() {

    byte_array<4u> myArray {{0, 1, 2, 3}};
    data_chunk myChunk {0, 1, 2, 3};

    auto firstHash = bitcoin_short_hash(myArray);  //ok
    auto secondHash = bitcoin_short_hash(myChunk); //ok

    std::cout << (firstHash == secondHash); //True
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
