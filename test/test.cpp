//#include "test.h"
#include <stdio.h>
#include <string>
//#include <map>
//#include <exception>
//#include "../io/iomem.h"
#include "../io/iofile.h"
//#include "../io/iobit.h"
//#include "../io/iobuf.h"
//#include "../io/iowave.h"
//#include <cmath>
//#include <iostream>
//#include <fstream>
//#include <string>
#include "../core/scale.h"
//#include "../core/spectrum.h"
//#include "../io/iowave.h"
#include "../spl_c/spl_c.h"
#include "../core/model.h"



using namespace std;


using namespace spl;

//static const char *TEST_STATUS_FORMAT = "\rTest %-40s %s\n";
//static const char *TEST_STATUS_TIME_FORMAT = "\rTest %-40s %s (%lf ms)\n";
//static const char *TEST_MESSAGE_FORMAT = "\t%s\n";

const char *signal_wav_std = "E:/testdata/signal.wav";
const char* scale_test = "E:/testdata/test/test2-scale-model-freq.bin";

int main(int argc, const char *argv[]) {

    //iwstream<double>(*signal_wav_std); :basic_iwstream(signal_wav_std);
    try {
        //double byak[1024];

        freq_t Fr;
        int K = 256;
        freq_t F1 = 50.0, F2 = 4000.0;
        

        spl_freq_scale_generate(K, &Fr, freq_scale_model, F1, F2);

        spl_freq_scale_save(scale_test, &Fr, K);




        //iwstream<double> baka(signal_wav_std);
        //freq_scale_t sc = freq_scale_t::generate(spl_params_t::);
        //sc.save(scale_test);

    }
    catch (const char* message) {
        printf("CRASHED!");
        printf(message);
    }
    catch (const string & message) {
        printf("CRASHED!");
        printf(message.c_str());
    }


    /*using test::test_t;

    auto help = []() {
        printf("Available tests: ");
        test_t::foreach([](test_t& test) {
            printf("%s ", test.name());
        });
        printf("\n");
    };

    if (argc < 2) {
        help();
        return -1;
    }

    map<string, test_t *> tests_by_name;
    test_t::foreach([&tests_by_name](test_t& test) {
        tests_by_name[test.name()] = &test;
    });

    for (int i = 1; i < argc; i++) {
        if (0 == strcmp(argv[i], "help")) {
            help();
        }
        else if (0 == strcmp(argv[i], "all")) {
            test_t::foreach([&tests_by_name](test_t& test) {
                test.execute();
            });
        }
        else {
            auto t = tests_by_name.find(argv[i]);
            if (t != tests_by_name.end()) {
                t->second->execute();
            }
            else {
                printf(TEST_STATUS_FORMAT, argv[i], "NOT FOUND!");
            }
        }
    }*/

    return 0;
}

//NAMESPACE_TEST_BEGIN;
//
//test_t *test_t::tests = nullptr;
//
//class test_failure: public exception {
//public:
//    test_failure(const char *message) :
//        exception(message) {}
//};
//
//void test_t::fail(const char *message, va_list args) {
//    char buffer[1000];
//    vsprintf(buffer, message, args);
//    throw test_failure(buffer);
//}
//
//void test_t::execute() {
//    try {
//        printf("\t");
//        test();
//        if (execution_time < 0)
//            printf(TEST_STATUS_FORMAT, name(), "PASSED");
//        else
//            printf(TEST_STATUS_TIME_FORMAT, name(), "PASSED", execution_time);
//    }
//    catch (const test_failure& fail) {
//        printf(TEST_STATUS_FORMAT, name(), "FAILED!");
//        printf(TEST_MESSAGE_FORMAT, fail.what());
//    }
//    catch (const exception& error) {
//        printf(TEST_STATUS_FORMAT, name(), "CRASHED!");
//        printf(TEST_MESSAGE_FORMAT, error.what());
//    }
//    catch (const char *message) {
//        printf(TEST_STATUS_FORMAT, name(), "CRASHED!");
//        printf(TEST_MESSAGE_FORMAT, message);
//    }
//    catch (const string& message) {
//        printf(TEST_STATUS_FORMAT, name(), "CRASHED!");
//        printf(TEST_MESSAGE_FORMAT, message.c_str());
//    }
//    catch (...) {
//        printf(TEST_STATUS_FORMAT, name(), "CRASHED!");
//    }
//}
//
//#ifdef _WIN32
//#include <windows.h>
//static time_t get_time() { return GetTickCount(); }
//void test_t::tic() { time = get_time(); }
//time_t test_t::toc() { return get_time() - time; }
//#else
//static long long test_time = 0;
//long long get_time() { time_t t; ctime(&t); return t; }
//void tic() { test_time = get_time(); }
//long long toc() { return get_time() - test_time; }
//#endif
//
//
//NAMESPACE_TEST_END;
