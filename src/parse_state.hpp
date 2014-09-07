#ifndef PARSE_STATE_HPP_
#define PARSE_STATE_HPP_

#include "symbol.hpp"

template <class Iterator>
class parse_state
{
private:
    Iterator pos;
    Iterator end;
    
    source_location loc;
public:
    parse_state(Iterator begin, Iterator end)
      : pos(begin),
        end(end),
        loc()
    {}
    parse_state(Iterator begin, Iterator end, const char* file_name)
      : parse_state(begin, end)
    {
        loc.file_name = file_name;
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
            loc.pos = 0;
            ++loc.line;
        }
        else
        {
            ++loc.pos;
        }
        
        ++pos;
    }
    
    const source_location& location() const
    {
        return loc;
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

