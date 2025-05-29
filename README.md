# soa

C++ 20 macros that make SoA ([structure of arrays](https://en.wikipedia.org/wiki/AoS_and_SoA)) data layout easier to work with.

## Example usage

```cpp
// Aos struct
struct AosStructExample {
    int a{};
    std::string b;
};

std::vector<AosStructExample> aos_vec;
AosStructExample aos_struct;
aos_vec.push_back(aos_struct);
int aos_first_value = aos_vec[0].a;

// The equivelent Soa struct declaration as the above Aos struct would be:
struct SoaStructExample {
    FixedSizeSOA(
        SoaStructExample, 2,
        int, a,
        std::string, b
    )
};

SoaStructExample soa_struct;
soa_struct.init(1) // allocate memory for 1 element and setup sub vectors.

soa_struct.set_a(0, 999);
// OR
soa_struct.a[0] = 999;

int soa_first_value = soa_struct.get_a(0);
// OR
int soa_first_value = soa_struct.a[0];
```

For more examples check out the code in the `tests` directory.

## Implementation Details

Doing Soa in C++ is a pain because the language just isn't made for it, generally it's designed to do the exact opposite in most cases. Some languages like [Zig](https://ziglang.org/documentation/master/std/#std.multi_array_list.MultiArrayList) and [Odin](https://odin-lang.org/docs/overview/#soa-data-types) have built in ways to make working with it much easier...C++ is not one of those languages.

There seem to be 4 possible ways to go about solving this in C++:
1. [Disgusting template stuff with tuples](https://github.com/crosetto/SoAvsAoS)
2. [Generating the code outside of C++](https://marzer.github.io/soagen/)
3. Using macros (this is what I do here)
4. [In C++ 26 with reflection](https://brevzin.github.io/c++/2025/05/02/soa/#a-working-implementation)

Unfortunately all of these solutions kind of suck but I think I've come up with a nice solution here that sucks the least (it still kinda sucks though Zig and Odin's solutions are much better).

The core idea behind the macros is to send the type and name of every class variable member to a bunch of [recursive FOR_EACH macros](https://www.scs.stanford.edu/~dm/blog/va-opt.html) to generate all the boilerplate required. The generated boilerplate code includes getters and setters for each member, push for dynamic vectors, initilization, and deinitilization code.


One of the big problems with using Soa data layout in general is that since each member is a vector to do the same thing as with Aos you have to do a lot more memory allocations as each member would get allocated seperately. To prevent this each Soa struct you declare will manage the memory of all of it's members, this is done by having each member stored in a specialized [SoaVector](https://github.com/dementive/soa/blob/main/src/SoaVector.hpp) container that does mostly the same things as std::vector but it does not manage it's own memory. This way only 1 allocation is ever made each time initilization or reallocation happens instead of 1 for each vector.


There are 2 different macros you can use to create your Soa structs: FixedSizeSOA and DynamicSOA.

The FixedSizeSOA is...fixed size, it is for things that the size is not known at compile time but it is known that it will not grow after initilization. After calling the `init(size)` function, use the `set_X(index, elem)` function to add new items.

DynamicSOA is, you guessed it, dynamically sized. Unlike FixedSizeSOA it can allocate more memory for itself it it runs out. Using DynamicSOA will generate a `push_X(index, elem)` function for each of your members, these must be used to add new elements so the SoaVector can keep track of it's size.

The SoaVector that each member is stored in satisfies the `std::ranges::contiguous_range` concept, meaning they can be used with almost all the `<ranges>` and `<algorithm>` methods. In particular [ranges](https://en.cppreference.com/w/cpp/ranges.html) has some nice methods that help make Soa layout easier by giving a way to query rows joined together using C++23 `views::zip` and `ranges::to`:
```cpp
struct SoaStruct {
FixedSizeSOA(
  SoaStruct, 3,
  int, a,
  int, b,
  int, c
)
};

SoaStruct soa_struct;
soa_struct.init(5);
static_assert(std::ranges::contiguous_range<decltype(soa_struct.a)>);

for (int i = 0; i < 5; ++i) {
  soa_struct.set_a(i, i);
  soa_struct.set_b(i, i + i);
}

auto zip_test = views::zip(soa_struct.a, soa_struct.b);
std::tuple<int, int> fifth_row = zip_test[4];
TEST("Zip: ", std::get<0>(fifth_row) == 4 and std::get<1>(fifth_row) == 8)

// have to use ranges::to to sort (or modify in any way) a subrange otherwise it will modify the original data.
auto subrange_test = ranges::subrange(zip_test.begin(), zip_test.end()) | ranges::to<std::vector<std::tuple<int, int>>>();
ranges::sort(subrange_test, std::greater());
auto first_sorted_pair = subrange_test[0];
TEST("Sorting subrange: ", std::get<0>(first_sorted_pair) == 4 and std::get<1>(first_sorted_pair) == 8)
```

Unfortunately (currently) there is no way to remove from either of these containers as ensuring that externally held IDs (indexes) don't get invalidated was very important to me. There is probably a way to do this by just storing a ID->index mapping on top of a DynamicSOA but I haven't gotten to it yet.

## Benchmark

To make sure I didn't waste my time implementing a slow piece of garbage I made some simple [benchmarks](https://github.com/dementive/soa/blob/main/tests/AoSvsSoA_test.hpp) that test filling a struct with Aos layout vs Soa layout and then the time to set 1 member and also the time to sum 1 member for each one:

```
---------------------------------------------------------------
Benchmark                     Time       Iterations
---------------------------------------------------------------
SoaFillAllMembers          0.135377  ms     1000
AosFillAllMembers          0.021005  ms     1000
SoaSetOneMember            0.001142  ms     1000
AosSetOneMember            0.015990  ms     1000
SoaSumOneMember            0.002102  ms     1000
AosSumOneMember            0.006647  ms     1000

SoaFillAllMembers          7.467560  ms     1,000,000
AosFillAllMembers          5.700850  ms     1,000,000
SoaSetOneMember            0.463828  ms     1,000,000
AosSetOneMember            4.104640  ms     1,000,000
SoaSumOneMember            0.278665  ms     1,000,000
AosSumOneMember            2.049370  ms     1,000,000
```

This shows that using all members with Soa layout is slower than Aos but accessing individual members is faster with Soa.
