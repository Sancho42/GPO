#include "test.h"
#include "../io/iomem.h"
#include "../io/iofile.h"
#include "../io/iobit.h"
#include "../io/iobuf.h"
#include "../io/iowave.h"
#include <stdio.h>
#include <cmath>

NAMESPACE_TEST_BEGIN;

using namespace io;

const char *signal_std = "E:/testdata/signal.bin";
const char *signal_wav_std = "E:/testdata/signal.wav";
const char *numbers_std = "E:/testdata/256.bi";
const char *numbers_test = "E:/testdata/test-256.bi";

// streams comparison

class test_iwstream_t : public test_error_t {
public:

    const char *name() override { return "iwstream"; }
    double max_error() override { return 1E-10; }
    double error() override {
        return compare_streams(iwstream<double>(signal_wav_std), iwstream<double>(signal_wav_std));
    }

} test_iwstream;

class test_iobit_t : public test_t {

    const char *name() override { return "iobit"; }
    void test() override {
        typedef unsigned short ushort;
        typedef unsigned long ulong;
        typedef io::imstream<ushort> imstream16;
        typedef io::omstream<ushort> omstream16;
        using io::ibitwrap16;
        using io::obitwrap16;

        ushort x[4] = { 0x3210, 0x7654, 0xBA98, 0xFEDC };
        ushort y[4] = { 0, 0, 0, 0 };
        imstream16 input_byte(x, sizeof(x) / sizeof(ushort));
        omstream16 output_byte(y, sizeof(y) / sizeof(ushort));
        ibitwrap16 input_bit(input_byte);
        obitwrap16 output_bit(output_byte);

        for (int i = 0; i < 17; i++) {
            for (int j = 0; j < 4; j++) {
                bool b;
                if (input_bit.get(b)) {
                    output_bit.put(b);
                }
            }
        }

        assert(y[0] == x[0] && y[1] == x[1] && y[2] == x[2] && y[3] == x[3], 
            "ibitwrap::get and obitwrap::put: %X %X %X %X ", y[0], y[1], y[2], y[3]);
        
        input_byte.pos(0);
        output_byte.pos(0);
        y[0] = y[1] = y[2] = y[3] = 0;

        while (!input_bit.eos() && !output_bit.eos()) {
            bool b[19];
            size_t r, w;
            r = input_bit.read(b, 19); // read by 19 bits
            w = output_bit.write(b, 19); // write by 19 bits
        }
        assert(y[0] == x[0] && y[1] == x[1] && y[2] == x[2] && y[3] == x[3],
            "ibitwrap::read and obitwrap::write: %X %X %X %X ", y[0], y[1], y[2], y[3]);
    }

} test_iobit;

class test_iobuf_t : public test_t {

    const char *name() override { return "iobuf"; }
    void test() override {
        typedef short elem_t;

        ifstream<elem_t> input(numbers_std);
        ofstream<elem_t> output(numbers_test);
        memory_buffer<elem_t> buffer;

        elem_t block[25];
        size_t size1 = 20,
            size2 = 15,
            size3 = 25,
            read, written;
        while (!input.eos()) {
            read = input.read(block, size1);

            written = buffer.output().write(block, read);
            assert(written == read, "written != read: %d != %d\n", written, read);

            read = buffer.input().read(block, size2);
            assert(read == size2, "should have read %d elements, but read %d elements\n", size2, read);

            output.write(block, read);
        }
        buffer.output().close();

        // write the remainder
        while (!buffer.input().eos()) {
            read = buffer.input().read(block, size3);
            output.write(block, read);
        }

        input.close();
        output.close();

        double e = compare_streams<elem_t>(numbers_std, numbers_test);
        assert(e == 0, "non-zero error: %d", int(e));
    }
} test_iobuf;

NAMESPACE_TEST_END;