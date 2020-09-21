#ifndef _IO_BUF_
#define _IO_BUF_

#include "io.h"
#include "iowrap.h"

namespace io {

class iobuf_impl
{
public:
	typedef unsigned char byte;
	static iobuf_impl *create();
	static void destroy(iobuf_impl *impl);
	virtual istream<byte>& input() = 0;
	virtual ostream<byte>& output() = 0;
};

template<typename T>
class memory_buffer:
	public buffer<T>
{
public:
    memory_buffer():
		impl(iobuf_impl::create()),
		_input(impl->input()),
		_output(impl->output())
	{
	}

	~memory_buffer() {
		iobuf_impl::destroy(impl);
	}

    virtual istream<T>& input() { return _input; }
    virtual ostream<T>& output() { return _output; }

private:
	typedef unsigned char byte;
	iobuf_impl *impl;
	iwrapblock<T,byte> _input;
	owrapblock<T,byte> _output;
};

} // namespace io


#endif//_IO_BUF_