#include "printing.hpp"

#include "error/parse_error.hpp"
#include "error/import_export_error.hpp"
#include "error/evaluate_error.hpp"
#include "error/compile_type_error.hpp"
#include "error/core_misc_error.hpp"
#include "error/compile_function_error.hpp"
#include "error/macro_execution_error.hpp"

#include <llvm/Support/raw_os_ostream.h>
#include <llvm/IR/Type.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

using std::size_t;
using std::string;
using std::function;
using std::vector;
using std::stoul;
using std::ostringstream;
using std::ostream;
using std::unordered_map;

using boost::static_visitor;
using boost::apply_visitor;
using boost::blank;

using llvm::raw_os_ostream;
using llvm::Type;

struct print_location_visitor
  : static_visitor<>
{
    ostream& os;
    const function<string (size_t)>& file_id_to_name;
    
    print_location_visitor(ostream& os, const function<string (size_t)>& file_id_to_name)
      : os(os),
        file_id_to_name(file_id_to_name)
    {}

    void operator()(const blank&)
    {
        os << "<no location>";
    }
    void operator()(const code_location& loc)
    {
        os << file_id_to_name(loc.file_id) << ":" << (loc.pos.line + 1) << ":" << (loc.pos.line_pos + 1);
    }
};


ostream& print_error_location(ostream& os, const error_location& loc, const function<string (size_t)>& file_id_to_name)
{
    print_location_visitor visitor(os, file_id_to_name);
    apply_visitor(visitor, loc);
    return os;
}

struct print_error_visitor
  : static_visitor<>
{
    ostream& os;
    
    print_error_visitor(ostream& os)
      : os(os)
    {}
    
    void operator()(const blank&)
    {
        os << "<argument missing>";
    }

    void operator()(const Type& t)
    {
        raw_os_ostream llvm_os{os};
        t.print(llvm_os);
    }
    template<class T>
    void operator()(const T& obj)
    {
        os << obj;
    }
};


string format(const string& format_string, const vector<error_parameter>& parameters)
{
    ostringstream stream;
    auto it = format_string.begin();
    for( ; it != format_string.end(); ++it)
    {
        if(*it == '\\')
        {
            ++it;
            assert(it != format_string.end());
            assert(*it == '{');
            stream << *it;
        }
        else if(*it == '{')
        {
            ++it;
            auto number_begin = it;
            assert(it != format_string.end());
            while(*it != '}')
            {
                assert(it != format_string.end());
                ++it;
            }
            auto number_end = it;
            size_t after_number;
            string number_string{number_begin, number_end};
            unsigned long number = stoul(number_string, &after_number);
            assert(after_number == number_string.size());
            assert(number < parameters.size());

            print_error_visitor visitor{stream};
            apply_visitor(visitor, parameters[number]);
        }
        else
            stream << *it;
    }
    return stream.str();
}

void print(ostream& os, const compile_exception& exc, function<string (size_t)> file_id_to_name)
{
    const char* error_name;
    const char* error_message_template;
    switch(exc.kind)
    {
    case error_kind::PARSE:
    {
        assert(exc.error_id < size(parse_error::dictionary));
        const auto& entry = parse_error::dictionary[exc.error_id];
        error_name = entry.first.data();
        error_message_template = entry.second.data();
        break;
    }
    case error_kind::IMPORT_EXPORT:
    {
        assert(exc.error_id < size(import_export_error::dictionary));
        const auto& entry = import_export_error::dictionary[exc.error_id];
        error_name = entry.first.data();
        error_message_template = entry.second.data();
        break;
    }
    case error_kind::EVALUATE:
    {
        assert(exc.error_id < size(evaluate_error::dictionary));
        const auto& entry = evaluate_error::dictionary[exc.error_id];
        error_name = entry.first.data();
        error_message_template = entry.second.data();
        break;
    }
    case error_kind::COMPILE_TYPE:
    {
        assert(exc.error_id < size(compile_type_error::dictionary));
        const auto& entry = compile_type_error::dictionary[exc.error_id];
        error_name = entry.first.data();
        error_message_template = entry.second.data();
        break;
    }
    case error_kind::CORE_MISC:
    {
        assert(exc.error_id < size(core_misc_error::dictionary));
        const auto& entry = core_misc_error::dictionary[exc.error_id];
        error_name = entry.first.data();
        error_message_template = entry.second.data();
        break;
    }
    case error_kind::COMPILE_FUNCTION:
    {
        assert(exc.error_id < size(compile_function_error::dictionary));
        const auto& entry = compile_function_error::dictionary[exc.error_id];
        error_name = entry.first.data();
        error_message_template = entry.second.data();
        break;
    }
    case error_kind::MACRO_EXECUTION:
    {
        assert(exc.error_id < size(macro_execution_error::dictionary));
        const auto& entry = macro_execution_error::dictionary[exc.error_id];
        error_name = entry.first.data();
        error_message_template = entry.second.data();
        break;
    }
    }
    print_error_location(os, exc.location, file_id_to_name);
    if(error_message_template != string{""})
        os << ": " << format(error_message_template, exc.params) << "\n";
    else
        os << ": " << "<" << error_name << ">" << " (missing error message)\n";
}

ostream& operator<<(ostream& os, const compile_exception& exc)
{
    auto file_id_to_name = [](size_t){return "";};
    print(os, exc, file_id_to_name);

    return os;
}

#ifndef NDEBUG
const char* compile_exception::what() const noexcept
{
    static vector<string> error_strings;

    ostringstream oss;
    oss << *this << "\n";
    error_strings.push_back(oss.str());

    return error_strings.back().c_str();
}
#endif

struct print_node_visitor
{
    ostream& os;
    const string& indentation;

    void operator()(const id_node& id) const
    {
        os << indentation << "$" << id.id() << "\n";
    }
    void operator()(const lit_node& lit) const
    {
        os << indentation << "lit\"" << save<string>(rangeify(lit)) << "\"\n";
    }
    void operator()(const ref_node& ref) const
    {
        os << indentation << "ref\"" << save<string>(ref.identifier()) << "\"\n";
        if(ref.refered())
        {
            string new_indentation = indentation + "  ";
            ref.refered()->visit(print_node_visitor{os, new_indentation});
        }
    }
    void operator()(const list_node& list) const
    {
        os << indentation << "list" << "\n";
        string new_indentation = indentation + "  ";
        for(const node& s : list)
            s.visit(print_node_visitor{os, new_indentation});
    }
    void operator()(const macro_node&) const
    {
        os << indentation << "macro\n";
    }
};

ostream& operator<<(ostream& os, const node& s)
{
    string indent = "";
    s.visit(print_node_visitor{os, indent});
    return os;
}

