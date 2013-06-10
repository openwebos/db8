/****************************************************************
 * @@@LICENSE
 *
 * Copyright (c) 2013 LG Electronics, Inc.
 *
 * LICENSE@@@
 ****************************************************************/

/****************************************************************
 *  @file AtomicIntTest.cpp
 ****************************************************************/

#include <array>

#include <thread>
#include <mutex>

#include <cstdlib>

#include <core/MojAtomicInt.h>

#include "Runner.h"

TEST(AtomicInt, logic)
{
    for(size_t n = 0; n < 100; ++n)
    {
        MojAtomicInt i1;
        MojAtomicInt i2(25);
        MojAtomicInt i3(i2);

        EXPECT_EQ( 0, i1.value() );
        EXPECT_EQ( 25, i2.value() );
        EXPECT_EQ( 25, i3.value() );
        EXPECT_TRUE( i2 == i3 );
        EXPECT_TRUE( i1 != i2 );
        i1 = i3;
        i3 = 0;
        EXPECT_EQ( 0, i3.value() );
        EXPECT_EQ( 25, i1.value() );
        EXPECT_EQ( 26, i1.increment() );
        EXPECT_EQ( 26, i1.value() );
        EXPECT_EQ( 1, ++i3 );
        EXPECT_EQ( 1, i3.value() );
        EXPECT_EQ( 25, i1.decrement() );
        EXPECT_EQ( 25, i1.value() );
        EXPECT_EQ( 0, --i3 );
        EXPECT_EQ( 0, i3.value() );
    }
}

TEST(AtomicInt, threadsafety)
{
    const size_t nthreads = 8, nsteps = 10000;

    MojAtomicInt sum = 0;
    std::mutex barrier;
    auto f = [&sum, &barrier]() {
        barrier.lock(); // align to barrier unlock
        barrier.unlock(); // unlock next thread

        for(size_t n = 0; n < nsteps; ++n)
        {
            sum.increment();
        }
    };

    barrier.lock(); // lock barrier

    std::array<std::thread, nthreads> threads;
    for(auto &thread : threads) thread = std::thread(f);

    barrier.unlock(); // give a way to threads

    for(auto &thread : threads) thread.join();
    EXPECT_EQ( (MojInt32)(nthreads * nsteps), sum.value() );
}
