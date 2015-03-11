#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE macro_execution
#include <boost/test/unit_test.hpp>

#include "../src/compile_function.hpp"

#include "state_utils.hpp"

#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

#include <memory>
#include <unordered_map>
#include <utility>
#include <iterator>
#include <string>

using std::pair;
using std::tie;
using std::ignore;
using std::unique_ptr;
using std::unordered_map;
using std::advance;
using std::string;

using llvm::Function;
using llvm::Value;
using llvm::IntegerType;
using llvm::Type;
using llvm::verifyFunction;

BOOST_AUTO_TEST_CASE(asdf_test)
{
}

