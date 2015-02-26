#ifndef PARSE_STATE_HPP_
#define PARSE_STATE_HPP_

#include "symbol_source.hpp"
#include "compilation_context.hpp"

#include <string>
#include <cassert>
#include <iostream>
#include <exception>

template<class Iterator>
class parse_state
{
private:
    Iterator pos;
    Iterator end;
    
    file_position file_pos;
    size_t file_id;

    compilation_context& context;
public:
    parse_state(Iterator begin, Iterator end, size_t file_id, compilation_context& context)
      : pos{begin},
        end{end},
        file_pos{0, 0},
        file_id{file_id},
        context(context)
    {}
    
    bool empty() const
    {
        return pos == end;
    }

    char front() const
    {
        assert(!empty());
        return *pos;
    }
    void pop_front()
    {
        assert(!empty());
        
        if(front() == '\n')
        {
            file_pos.line_pos = 0;
            ++file_pos.line;
        }
        else
        {
            ++file_pos.line_pos;
        }
        
        ++pos;
    }
    
    const file_position& position() const
    {
        return file_pos;
    }
    size_t file() const
    {
        return file_id;
    }
    identifier_id_t identifier_id(const std::string& str)
    {
        return context.identifier_id(str);
    }
};

#endif

