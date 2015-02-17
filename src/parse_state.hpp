#ifndef PARSE_STATE_HPP_
#define PARSE_STATE_HPP_

#include "source_location.hpp"
#include "error/parse_error.hpp"

#include <string>
#include <cassert>
#include <iostream>
#include <exception>


template <class Iterator>
class parse_state
{
private:
    Iterator pos;
    Iterator end;
    
    source_position source_pos;
    const char* file_name;
public:
    parse_state(Iterator begin, Iterator end)
      : pos(begin),
        end(end),
        source_pos(),
        file_name(0)
    {}
    parse_state(Iterator begin, Iterator end, const char* file_name)
      : parse_state(begin, end)
    {
        this->file_name = file_name;
    }
    
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
            source_pos.line_position = 0;
            ++source_pos.line;
        }
        else
        {
            ++source_pos.line_position;
        }
        
        ++pos;
    }
    
    const source_position& position() const
    {
        return source_pos;
    }
    const char* file() const
    {
        return file_name;
    }
};

template <class Iterator>
std::string remaining(parse_state<Iterator> state)
{
    std::string result;
    while(!state.empty())
    {
        result += state.front();
        state.pop_front();
    }
    return result;
}

#endif

