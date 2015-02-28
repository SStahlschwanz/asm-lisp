#include "core_module.hpp"

#include "compile_type.hpp"
#include "symbol.hpp"
#include "error/unique_exception.hpp"

#include <memory>

using std::unique_ptr;
using std::make_unique;
using namespace unique_exception;

module create_core_module(compilation_context& context)
{
    module m;
    
    auto int_func = [&](list_symbol::const_iterator begin, list_symbol::const_iterator end)
    {
        return compile_int(begin, end, context);
    };
    auto int_macro = make_unique<any_symbol>(macro_symbol{int_func});
    m.exports[identifier_ids::INT] = int_macro.get();
    m.evaluated_exports.push_back(move(int_macro));

    auto unique_func = [&](list_symbol::const_iterator begin, list_symbol::const_iterator end)
    {
        if(begin != end)
            throw invalid_argument_number{};
        return id_symbol{context.uuid()};
    };
    auto unique_macro = make_unique<any_symbol>(macro_symbol{unique_func});
    m.exports[identifier_ids::UNIQUE] = unique_macro.get();
    m.evaluated_exports.push_back(move(unique_macro));

    return m;
}

