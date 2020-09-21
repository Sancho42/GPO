#ifndef _TEST_H_
#define _TEST_H_

#define _CRT_SECURE_NO_WARNINGS

#define NAMESPACE_TEST_BEGIN namespace test {
#define NAMESPACE_TEST_END   }

#include <cstdarg>
#include <stdio.h>
#include <cmath>
#include "../io/iofile.h"

NAMESPACE_TEST_BEGIN;

template<typename T>
double compare_streams(io::istream<T>& str1, io::istream<T>& str2) {
    double e = 0;
    const int BLOCK_SIZE = 1024;
    T block1[BLOCK_SIZE], block2[BLOCK_SIZE];
    size_t read1, read2;
    
    while (read1 = str1.read(block1, BLOCK_SIZE), 
           read2 = str2.read(block2, BLOCK_SIZE), 
           read1 == read2 && read1 > 0)
    {
        for (size_t i = 0; i < read1; i++) {
            e += fabs(block1[i] - block2[i]);
        }
    }
    if (read1 != read2 || str1.eos() != str2.eos()) {
        printf("compare_streams: warning: streams have different sizes\n");
    }
    return e;
}

template<typename T>
double compare_streams(const char *filename1, const char *filename2) {
    io::ifstream<T> str1(filename1), str2(filename2);
    return compare_streams(str1, str2);
}

typedef unsigned long time_t;

class test_t {
public:

    virtual const char *name() = 0;
    virtual void test() = 0;

    void execute();

    template<typename F>
    static inline void foreach(F func) {
        for (test_t *test = tests; test != nullptr; test = test->next) {
            func(*test);
        }
    }

protected:
    test_t() {
        next = tests;
        tests = this;
        execution_time = -1;
    }

    void fail(const char *message, va_list args);
    void fail(const char *message, ...) {
        va_list args;
        va_start(args, message);
        fail(message, args);
    }
    void assert(bool expression, const char *message, ...) {
        if (!expression) {
            va_list args;
            va_start(args, message);
            fail(message, args);
        }
    }
    
    time_t time;
    void tic();
    time_t toc();
    
    double execution_time;
    void set_execution_time(double t) {
        execution_time = t;
    }

    static test_t *tests;
    test_t *next;
};

class test_error_t: public test_t {
public:

    virtual double max_error() = 0;
    virtual double error() = 0;
    virtual void test() override {
        double e = error();
        double m = max_error();
        assert(e <= m, "error is too big: %lg > %lg", e, m);
    }

};

NAMESPACE_TEST_END;

#endif//_TEST_H_
