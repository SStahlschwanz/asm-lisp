#include "../src/error/error_utils.hpp"

#include <limits>

using std::numeric_limits;

template<bool>
struct is_true;
template<>
struct is_true<true>
{};

constexpr std::pair<conststr, conststr> dict1[] =
{
    {"aa", "aa"},
    {"bb", "bb"},
    {"cc", "cc"}
};
is_true<!has_duplicates(dict1)> test11;
is_true<index_of("aa", dict1) == 0> test12;
is_true<index_of("bb", dict1) == 1> test13;
is_true<index_of("cc", dict1) == 2> test14;
is_true<index_of("asdfasdf", dict1) == numeric_limits<size_t>::max()> test15;
 
constexpr std::pair<conststr, conststr> dict2[] =
{
    {"asdf", "asdf"},
    {"asdf", "sdf"} // duplicate
};
is_true<has_duplicates(dict2)> test21;

int main()
{}
