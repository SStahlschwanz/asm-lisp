#include "error_to_string.hpp"

#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

#include "error/type_compile_exception.hpp"
#include "symbol.hpp"

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

const unordered_map<string, string> default_errors =
{
    {"parse_unmatched_quote", "{0}: unmatched '\"'"},
    {"parse_unterminated_semicolon_list", "{0}: expected ';'"},
    {"parse_unmatchd_curly_brace", "{0}: unmatched '{'"},
    {"parse_unmatched_square_brace", "{0}: unmatched '['"},
    {"parse_unmatched_round_brace", "{0}: unmatched '('"},
    {"parse_invalid_character", "{0}: unexpected character '{1}'"},


    {"import_export_import_invalid_argument_number", "{0}: invalid number of arguments to \'import\': expected 3, got {1}"},
    {"import_export_invalid_import_list", "{0}: expected a list (of imports)"},
    {"import_export_invalid_from_token", "{0}: expected 'from'"},
    {"import_export_invalid_imported_module", "{0}: expected an identifier (module name)"},
    {"import_export_invalid_imported_identifier", "{0}: expected an identifier (name of symbol to import)"},
    {"import_export_import_after_header", "{0}: import statement after file header is not allowed"},
    {"import_export_export_after_header", "{0}: export statement after file header is not allowed"},
    {"import_export_module_not_found", "{0}: module not found: \'{1}\'"},


    {"evaluation_empty_top_level_statement", "{0}: empty top level statement is not allowed"},
    {"evaluation_invalid_command", "{0}: invalid command"},
    {"evaluation_def_invalid_argument_number", "{0}: too few arguments to def: expected at least 2"},
    {"evaluation_invalid_defined_symbol", "{0}: invalid symbol to defined: expected identifier"},
    {"evaluation_duplicate_definition", "{0}: duplicate definition"},


    {"type_compile_int_invalid_argument_number", "invalid number of arguments to 'int': expected 1"},
    {"type_compile_int_invalid_argument_symbol", "{0}: invalid argument: expected a literal (bit width)"},
    {"type_compile_int_invalid_argument_literal", "{0}: invalid bit width: expected a positive integer"},
    {"type_compile_int_out_of_range_bit_width", "{0}: invalid bit width: expected a positive integer"},


    {"unique_invalid_argument_number", "invalid number of arguments to ' unique': expected none"}
};

struct print_error_visitor
  : static_visitor<>
{
    ostringstream& oss;
    const function<string (size_t)>& file_id_to_name;
    
    print_error_visitor(ostringstream& s, const function<string (size_t)>& file_id_to_name)
      : oss(s),
        file_id_to_name(file_id_to_name)
    {}
    
    void operator()(const code_location& loc)
    {
        oss << file_id_to_name(loc.file_id) << ':' << (loc.pos.line + 1) << ':' << (loc.pos.line_pos + 1);
    }
    void operator()(const string& str)
    {
        oss << str;
    }
    void operator()(size_t s)
    {
        oss << s;
    }
};

string format(const string& format_string, const vector<error_parameter>& parameters, const function<string (size_t)> file_id_to_name)
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

            print_error_visitor visitor{stream, file_id_to_name};
            apply_visitor(visitor, parameters[number]);
        }
        else
            stream << *it;
    }
    return stream.str();
}

string default_error_to_string(const compile_exception& exc, function<string (size_t)> file_id_to_name)
{
    auto it = default_errors.find(exc.error_name);
    assert(it != default_errors.end());
    
    return format(it->second, exc.parameters, file_id_to_name);
}

struct print_symbol_visitor
{
    ostream& os;
    const string& indentation;
    compilation_context& context;

    void operator()(const id_symbol& id) const
    {
        os << indentation << id.id() << "\n";
    }
    void operator()(const lit_symbol& lit) const
    {
        os << indentation << "lit\"" << string{lit.begin(), lit.end()} << "\"\n";
    }
    void operator()(const owning_ref_symbol& ref) const
    {
        os << indentation << "owning_ref\"" << context.to_string(ref.identifier()) << "\"\n";
        if(ref.refered())
        {
            string new_indentation = indentation + "  ";
            ref.refered()->visit(print_symbol_visitor{os, new_indentation, context});
        }
    }
    void operator()(const ref_symbol& ref) const
    {
        os << indentation << "ref\"" << context.to_string(ref.identifier()) << "\"\n";
        if(ref.refered())
        {
            string new_indentation = indentation + "  ";
            ref.refered()->visit(print_symbol_visitor{os, new_indentation, context});
        }
    }
    void operator()(const list_symbol& list) const
    {
        os << indentation << "list" << "\n";
        string new_indentation = indentation + "  ";
        for(const symbol& s : list)
            s.visit(print_symbol_visitor{os, new_indentation, context});
    }
    void operator()(const macro_symbol&) const
    {
        os << indentation << "macro\n";
    }
    void operator()(const type_symbol&) const
    {
        os << indentation << "type\n";
    }

};

ostream& print_symbol(ostream& os, const symbol& s, compilation_context& context)
{
    s.visit(print_symbol_visitor{os, "", context});
    return os;
}
