#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>


using namespace bc;

void example1() {
  //******** Example 1 ********

  //Let us declare a compressed pubKey in Libbitcoin
  ec_compressed myPubKey;

  //ec_compressed is a type alias of byte_array<33> type
  constexpr size_t ec_compressed_size = 33; //marker(1) + x-coord(32)
  using ec_compressed = byte_array<ec_compressed_size>;

  //byte_array<Size> is a type alias of a class template
  // template <size_t Size>
  // using byte_array = std::array<uint8_t, Size>;

  //For ec_compressed, this template compiles to:
  // template<size_t 33u>
  // using byte_array = std::array<uint8_t, 33u>;
}

void example2(){
  //******** Example 2 ********

  //data_chunks can contain entropy of various sizes
  data_chunk myEntropy16(16);      //128bits allocated
  data_chunk myEntropy32(32);      //256bits allocated

  //we can now fill our data_chunk with random bits
  pseudo_random_fill(myEntropy16); //128bit entropy
  pseudo_random_fill(myEntropy32); //256bit entropy

  //data_chunk is a type alias:
  using data_chunk = std::vector<uint8_t>;
}

void example3() {
  //******** Example 3 ********

  //Let's declare a byte_array of 16 bytes in memory
  constexpr size_t arraySize = 16;
  byte_array<arraySize> myArray;

  //pseudo_random_fill expects a data_chunk type input
  //yet byte_array isn't implicitly converted to data_chunk
  //pseudo_random_fill(myArray); //not ok

  //so we explicitly convert with our helper fct
  data_chunk myChunk = to_chunk(myArray);
  pseudo_random_fill(myChunk); //ok
}

void example4() {
    //******** Example 4 ********

    //bitcoin_short_hash function signature
    //short_hash bitcoin_short_hash(data_slice data);

    byte_array<4u> myArray {{0, 1, 2, 3}};
    data_chunk myChunk {0, 1, 2, 3};

    auto firstHash = bitcoin_short_hash(myArray);  //ok
    auto secondHash = bitcoin_short_hash(myChunk); //ok

    std::cout << (firstHash == secondHash); //True
};

int main() {

  example1();
  std::cout << "\n\n";

  example2();
  std::cout << "\n\n";

  example3();
  std::cout << "\n\n";

  example4();
  std::cout << "\n\n";

  return 0;

}
