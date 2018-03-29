# Pedersen Commitments in EC Form

Using [EC operations](https://github.com/libbitcoin/libbitcoin/wiki), we can also generate Pedersen commitments in EC form, which can be expressed as the following:

> C =  r * H + a * G

The commitment `C` is binding, as neither the commited value `a` nor the random value `r` can be altered after the commitment has been made. It is also blinding, as the random value `r` ensures that no information about the commited value `a` can leak, even if two commitments are made to the same commited value `a`.

The committed value cannot be altered after commitment `C` is made, because `H` is a curve point with an unknown discrete log.

> H = q * G  
> Where q is unknown:  

If `q` were known, it could be factored out of `H`:  

> C = r * ( q * G ) + a * G = ( r * q + a ) * G  

In this case, multiple possible values of `r` and `a` can be determined for a given commitment `C`. Therefore, `r` and `a` can be changed after the commitment has been made, rendering the commitment non-binding.

The follow example demonstrates generating a pedersen commitment in EC form in Libbitcoin.
```c++
#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>

// Namespace
using namespace bc;
```
```c++
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
```

## Homomorphic Property of Pedersen Commitments

Homomorphism means that algebraic operations on commitments will also hold true for the binding and blinding factors they contain.

> C( r1 , a1 ) + C( r2 , a2 ) = C( r1 + r2 , a1 + a2 )

This is equality is demonstrated in the example below:

```c++
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
```
