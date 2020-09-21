#include "iowave.h"
#include <limits>

namespace io {

basic_iwstream::basic_iwstream(const char *file):
	_f(file), F(0), N(0), M(0), B(0)
{
	word w; dword d, s;
	if(!get_element(d) || d != 0x46464952) // "RIFF"
		throw "Not a RIFF file";
	get_element(d); // chunk data size
	if(!get_element(d) || d != 0x45564157) // "WAVE"
		throw "Not a WAVE file";

	// go through chunks
	do {
		// chunk id && chunk size
		if(!get_element(d) || !get_element(s)) 
			throw "Can't read from file";
		if(d == 0x20746D66) { // "fmt "
			if(!get_element(w) || w != 1) // compression code
				throw "Don't support compressed wave-files";
			if(!get_element(M) // number of channels
			|| !get_element(F) // sample rate
			|| !get_element(d) // bytes per second
			|| !get_element(w) // block size
			|| !get_element(B))// bits per sample
				throw "Can't read from file";
			B = (B+7)/8; // bits per sample -> bytes per sample
		} else if(d == 0x61746164) { // "data"
			N = s; // chunk size
			_base = _f.pos(); // save sample section position
			break; // found data-chunk, go out
		} else {
			if(!get_element(d)) // chunk size
				throw "Can't read from file";
			_f.skip(d-8); // skip this chunk
		}
	} while(!_f.eos());

	if(!F || !N || !M || !B) 
		throw "Key fields of wave-file were not read";

}

} // namespace spl
