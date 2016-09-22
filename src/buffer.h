//
//    This file is part of Fastmatch binary market data and order flow examples.
//
//    Fastmatch binary market data and order flow examples are free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Fastmatch binary market data and order flow examples are distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Fastmatch binary market data examples.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef FASTAPP_BUFFER_H__
#define	FASTAPP_BUFFER_H__

#include <vector>
#include <cassert>

namespace fastapp
{

class InputBuffer
{
public:

    InputBuffer() 
    : _expect(0)
    , _consumed(0)
    , _write_at(0) 
    {
    }
    
    InputBuffer(size_t size) 
    : _expect(0)
    , _consumed(0)
    , _write_at(0) 
    {
        _buf.resize(size);
        _write_at = &_buf.front();
    }
    
    void resize(size_t size)
    {
        if (size <= _buf.size())
        {
            return;
        }

        // size_t offset = _write_at - &_buf.front(); // std::distance(&_buf.front(), _write_at); // std::abs(_write_at - &_buf.front());
        _buf.resize(size);
        _write_at = &_buf[_consumed];
    }

    void expected_size(ssize_t size)
    {
        _expect = size;
        
        ssize_t new_size = _expect + _consumed; // _expect + (_write_at - &_buf.front()); //std::distance(&_buf.front(), _write_at);
        assert(new_size > 0);
        if (new_size  > (ssize_t)_buf.size() )
        {
            _buf.resize(new_size);
        }
    }
    
    size_t expected_size() const
    {
        return _expect;
    }
    
    ssize_t consumed(ssize_t size_read)
    {
        assert(size_read <= _expect);
        
        if (size_read > 0)
        {
            _write_at += size_read;
            _consumed += size_read;
            _expect   -= size_read;
        }
        return _consumed;
    }
    
    ssize_t consumed()
    {
        return _consumed;
    }    
    
    char * write_ptr()
    {
        return _write_at;
    }

    char * read_ptr()
    {
        return &_buf.front ();
    }
    
    void reset()
    {
        _expect   = 0;
        _consumed = 0;
        _write_at = &_buf.front();
    }
   
    ssize_t size() const
    {
        return (ssize_t)_buf.size();
    }
    
    ssize_t capciity() const
    {
        return (ssize_t)_buf.capacity();
    }
protected:

    std::vector<char>     _buf;
    ssize_t               _expect;
    ssize_t               _consumed;
    char *                _write_at;
    
};
    
} //fastapp


#endif	/* BUFFER_H */

