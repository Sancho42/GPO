#ifndef _IO_WAVE_
#define _IO_WAVE_

#include "iofile.h"
#include <limits>

namespace io {

class basic_iwstream
{
protected:
    basic_iwstream(const char *file);

    typedef unsigned char byte;
    typedef unsigned short word;
    typedef unsigned long dword;
    io::ifstream<byte> _f;

    template<typename T>
    bool get_element(T& x) {
        return _f.read((byte*)&x, sizeof(T)) > 0;
    }

    // Function that gets sum sample across all channels (e.g. sum of two values for stereo).
    template<typename T>
    bool get_slice(T& x) {
        x = 0;
        T y;
        for (int i = 0; i < M; i++) {
            if (!get_element(y)) return false;
            x += y;
        }
        return true;
    }

    template<typename S, typename T>
    void convert_sample_to_float(const S& x, T& y) {
        S smin = std::numeric_limits<S>::min();
        S smax = std::numeric_limits<S>::max();
        y = (2 * T(x) - (T(smax) + 1 + smin)) / (T(smax) + 1 - smin);
    }

    template<typename S>
    void convert_sample(const S& x, float& y) {
        convert_sample_to_float(x, y);
    }

    template<typename S>
    void convert_sample(const S& x, double& y) {
        convert_sample_to_float(x, y);
    }

    template<typename S>
    void convert_sample(const S& x, long double& y) {
        convert_sample_to_float(x, y);
    }

    // WAVE FILE FORMAT
    size_t _base; ///< base of samples section
    dword N; ///< size of samples section
    word M; ///< number of channels
    word B; ///< number of bytes in one value
    dword F; ///< sampling frequency
};

/// 
/// Wave file stream.
/// Opens wave-file and provides access to it via io::istream<T> interface.
/// Has additional freq() function to get sampling frequency.
///

template<typename T>
class iwstream:
	public istream<T>,
    private basic_iwstream
{
public:
	/// Constructor. Get wav-file path.
    iwstream(const char *file): basic_iwstream(file) {}

	/// Get next sample.
	/// In multi-channel signals (e.g. stereo) this function gets sum of values for current sample.
	virtual bool get(T& x) {
        byte b; short s; long d;
        switch (B) {
        case 1: return get_slice<byte>(b)  ? (convert_sample<byte >(b, x), true) : false;
        case 2: return get_slice<short>(s) ? (convert_sample<short>(s, x), true) : false;
        case 4: return get_slice<long>(d)  ? (convert_sample<long >(d, x), true) : false;
        default: return false;
        }
    }

	virtual size_t pos() const { return (_f.pos() - _base) / M; }
	virtual size_t pos(size_t n) { return (_f.pos(_base + n*M) - _base) / M; }
	virtual size_t skip(size_t n) { return _f.skip(n*M), pos(); }
	virtual bool eos() const { return _f.pos() == _base + N; }
	virtual void close() { _f.close(); }

	/// Get sampling frequency.
	unsigned long freq() const { return F; }

};

} // namespace spl

#endif//_IO_WAVE_
