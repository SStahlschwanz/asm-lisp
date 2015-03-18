#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE compile_instruction
#include <boost/test/unit_test.hpp>

#include "../src/compile_instruction.hpp"

using llvm::Value;
using llvm::IRBuilder;

BOOST_AUTO_TEST_CASE(first_test)
{
    if(false)
    {
        auto define_variable = [&](const ref_node& name, named_value_info)
        {
        };
        auto lookup_variable = [&](const ref_node& name) -> named_value_info
        {
        };
        auto add_statement = [&](const node& statement_node, instruction_data data)
        {
        };
        IRBuilder<>& builder = *static_cast<IRBuilder<>*>(nullptr);
        auto st_context = make_statement_context(builder, define_variable, lookup_variable, add_statement);
        const node& n = *static_cast<const node*>(nullptr);
        compile_statement(n, st_context);
    }
}
