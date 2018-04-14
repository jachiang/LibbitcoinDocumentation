#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>


using namespace bc;

ec_compressed generate_h_candidate() {

    // Generate 32bytes of my_entropy.
    data_chunk my_entropy(32u); //256bits
    pseudo_random_fill(my_entropy);

    // Add 02/03 prefix.
    uint8_t prefix(0x02);
    my_entropy.insert(my_entropy.begin(),prefix);

    // Convert to ec_compressed byte array
    constexpr size_t my_array_size = 33u;
    byte_array<my_array_size> my_array;
    my_array = to_array<my_array_size>(my_entropy);
    ec_compressed point_h_candidate(my_array);

    return point_h_candidate;

}


void point_h_generation() {

    ec_compressed h_candidate;
    bool test = false;

    while(test == false) {
        h_candidate = generate_h_candidate();
        test = verify(h_candidate);
    }

    std::cout << verify(h_candidate) << std::endl;
    std::cout << encode_base16(h_candidate) << std::endl;

}


void create_pedersen_commitments() {

    //********** Part 1 **********

    // Example value for point h:
    // h = q * G with unknown q.
    auto point_h = base16_literal(
        "02b2138500d3754cd3009d8cc0bd5e7b89b0eb158594eef21ae7e4224bc1ff1a76");

    // Verify point h is a valid EC point.
    std::cout << verify(point_h) << std::endl;

    // Generate Pedersen Commitment C;
    // C = r * H + a * G

    // Create random r.
    data_chunk entropy_r(ec_secret_size);
    pseudo_random_fill(entropy_r);
    auto scalar_r = to_array<ec_secret_size>(entropy_r);

    // r * H
    ec_compressed left_point(point_h);
    ec_multiply(left_point, scalar_r);

    // a * G
    ec_compressed right_point;
    auto committed_a = base16_literal(
        "1aee6572a3590637cd3eaa95212aefb8c029b2d982feef2d38e53d0da2b5bae3");
    secret_to_public(right_point, committed_a);

    // C = r * H + a * G
    point_list commitment_point_list = {left_point, right_point};
    ec_compressed commitment_point;
    ec_sum(commitment_point, commitment_point_list);

    // Commitment point C:
    std::cout << encode_base16(commitment_point) << std::endl;


    //********** Part 2 **********

    // Create a second commitment C(r2, a2).
    // C2 = r2 * H + a2 * G
    data_chunk entropy_r2(ec_secret_size);
    pseudo_random_fill(entropy_r2);
    auto scalar_r2 = to_array<ec_secret_size>(entropy_r2);
    // r2 * H
    ec_compressed left_point2(point_h);
    ec_multiply(left_point2, scalar_r2);
    // a2 * G
    ec_compressed right_point2;
    auto committed_a2 = base16_literal(
        "69f9e04fb736ab209fea2dcc97d70c8b0bfb778857517bee68a5eeda6d610a72");
    secret_to_public(right_point2, committed_a2);
    // C2 = r2 * H + a2 * G
    point_list commitment_point_list2 = {left_point2, right_point2};
    ec_compressed commitment_point2;
    ec_sum(commitment_point2, commitment_point_list2);

    // Create sum of two commitments.
    // C from previous example: commitment_point.
    // C + C2
    point_list commitment_list = {commitment_point, commitment_point2};
    ec_compressed commitment_sum;
    ec_sum(commitment_sum, commitment_list);

    // Now we generate a commitment from r+r2 and a+a2.
    // C(r + r2, a + a2)
    ec_secret scalar_sum_r(scalar_r);
    ec_add(scalar_sum_r, scalar_r2);

    ec_secret scalar_sum_a(committed_a);
    ec_add(scalar_sum_a, committed_a2);

    ec_compressed left_point_(point_h);
    ec_multiply(left_point_, scalar_sum_r);

    ec_compressed right_point_;
    secret_to_public(right_point_, scalar_sum_a);

    point_list commitment_list_ = {left_point_, right_point_};
    ec_compressed commitment_sum_;
    ec_sum(commitment_sum_, commitment_list_);

    // Homomorphism holds.
    // C + C2 = C(r + r1, a + a1)
    std::cout << (commitment_sum == commitment_sum_) << std::endl;

}


int main() {

  point_h_generation();

  create_pedersen_commitments();

  return 0;

}
