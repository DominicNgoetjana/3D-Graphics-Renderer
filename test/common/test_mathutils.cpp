/**
 * @file
 *
 * Test code for @ref mathutils.h. Most of the tests are done with @c static_assert
 * rather than run-time tests.
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>
#include <common/mathutils.h>
#include <test/testutil.h>

static_assert(!isPower2(-2), "isPower2 is broken");
static_assert(!isPower2(0), "isPower2 is broken");
static_assert(isPower2(1), "isPower2 is broken");
static_assert(!isPower2(15), "isPower2 is broken");
static_assert(isPower2(16), "isPower2 is broken");
static_assert(!isPower2(17), "isPower2 is broken");
static_assert(isPower2(1 << 30), "isPower2 is broken");

static_assert(shrRound(3, 0) == 3, "shrRound is broken");
static_assert(shrRound(-3, 0) == -3, "shrRound is broken");
static_assert(shrRound(20, 0) == 20, "shrRound is broken");
static_assert(shrRound(-20, 0) == -20, "shrRound is broken");
static_assert(shrRound(20, 2) == 5, "shrRound is broken");
static_assert(shrRound(21, 2) == 5, "shrRound is broken");
static_assert(shrRound(22, 2) == 6, "shrRound is broken");
static_assert(shrRound(23, 2) == 6, "shrRound is broken");
static_assert(shrRound(24, 2) == 6, "shrRound is broken");
static_assert(shrRound(-20, 2) == -5, "shrRound is broken");
static_assert(shrRound(-21, 2) == -5, "shrRound is broken");
static_assert(shrRound(-22, 2) == -5, "shrRound is broken");
static_assert(shrRound(-23, 2) == -6, "shrRound is broken");
static_assert(shrRound(-24, 2) == -6, "shrRound is broken");
static_assert(shrRound(1073741823, 31) == 0, "shrRound is broken");

static_assert(shrDown(31, 0) == 31, "shrDown is broken");
static_assert(shrDown(-31, 0) == -31, "shrDown is broken");
static_assert(shrDown(INT_MIN, 0) == INT_MIN, "shrDown is broken");
static_assert(shrDown(95, 2) == 23, "shrDown is broken");
static_assert(shrDown(96, 2) == 24, "shrDown is broken");
static_assert(shrDown(-95, 2) == -24, "shrDown is broken");
static_assert(shrDown(-96, 2) == -24, "shrDown is broken");
static_assert(shrDown(-97, 2) == -25, "shrDown is broken");

static_assert(shrUp(31, 0) == 31, "shrUp is broken");
static_assert(shrUp(-31, 0) == -31, "shrUp is broken");
static_assert(shrUp(95, 2) == 24, "shrUp is broken");
static_assert(shrUp(96, 2) == 24, "shrUp is broken");
static_assert(shrUp(97, 2) == 25, "shrUp is broken");
static_assert(shrUp(-95, 2) == -23, "shrUp is broken");
static_assert(shrUp(-96, 2) == -24, "shrUp is broken");
static_assert(shrUp(-97, 2) == -24, "shrUp is broken");
static_assert(shrUp(1000000000, 20) == 954, "shrUp is broken");
static_assert(shrUp(-1000000000, 20) == -953, "shrUp is broken");

static_assert(isPower2Ratio(3.1f, 3.1f * 16), "isPower2Ratio is broken");
static_assert(isPower2Ratio(3.1f * 32, 3.1f), "isPower2Ratio is broken");
static_assert(isPower2Ratio(3.1f, 3.1f), "isPower2Ratio is broken");
static_assert(!isPower2Ratio(3.0f, 4.0f), "isPower2Ratio is broken");
static_assert(!isPower2Ratio(-1.0f, 1.0f), "isPower2Ratio is broken");
static_assert(!isPower2Ratio(1.0f, std::numeric_limits<float>::infinity()), "isPower2Ratio is broken");
// Large number of recursive calls
static_assert(isPower2Ratio(0x1p-300, 0x1p300), "isPower2Ratio is broken");
static_assert(isPower2Ratio(0x1p300, 0x1p-300), "isPower2Ratio is broken");

/// Run-time tests for @ref isPower2Ratio
class TestIsPower2Ratio : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestIsPower2Ratio);
    CPPUNIT_TEST(testNaN);
    CPPUNIT_TEST_SUITE_END();
private:
    /* GCC doesn't allow a NaN to be used in a comparison in a constant
     * expression.  I think that's probably a GCC bug, but to keep it happy we
     * test this at runtime.
     */
    void testNaN()
    {
        const float n = std::numeric_limits<float>::quiet_NaN();
        CPPUNIT_ASSERT(!isPower2Ratio(1.0f, n));
        CPPUNIT_ASSERT(!isPower2Ratio(n, 1.0f));
        CPPUNIT_ASSERT(!isPower2Ratio(n, n));
        // Put these in just to ensure coverage, even though it is redundant
        // with static assertions
        CPPUNIT_ASSERT(isPower2Ratio(0x1p300, 0x1p-300));
        CPPUNIT_ASSERT(!isPower2Ratio(1000000000.0, 1.0));
    }
};
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestIsPower2Ratio, TestSet::perBuild());

class TestRoundUp : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestRoundUp);
    CPPUNIT_TEST(testMultiple);
    CPPUNIT_TEST(testNonMultiple);
    CPPUNIT_TEST(testBig);
    CPPUNIT_TEST_SUITE_END();

private:
    void testMultiple()
    {
        CPPUNIT_ASSERT_EQUAL(0, roundUp(0, 1));
        CPPUNIT_ASSERT_EQUAL(0, roundUp(0, 5));
        CPPUNIT_ASSERT_EQUAL(0U, roundUp(0U, 4U));

        CPPUNIT_ASSERT_EQUAL(123, roundUp(123, 1));
        CPPUNIT_ASSERT_EQUAL(255, roundUp(255, 5));
        CPPUNIT_ASSERT_EQUAL(256U, roundUp(256U, 16U));
    }

    void testNonMultiple()
    {
        CPPUNIT_ASSERT_EQUAL(17, roundUp(1, 17));
        CPPUNIT_ASSERT_EQUAL(17, roundUp(16, 17));
        CPPUNIT_ASSERT_EQUAL(256, roundUp(250, 16));
        CPPUNIT_ASSERT_EQUAL(48U, roundUp(40U, 12U));
    }

    void testBig()
    {
        CPPUNIT_ASSERT_EQUAL(std::uint32_t(4294967295U),
                             roundUp(std::uint32_t(4294967294U),
                                     std::uint32_t(65537U)));
        CPPUNIT_ASSERT_EQUAL(std::uint32_t(4294967295U),
                             roundUp(std::uint32_t(4294967295U),
                                     std::uint32_t(65537U)));
        CPPUNIT_ASSERT_EQUAL(std::uint32_t(4294967292U),
                             roundUp(std::uint32_t(4294967290U),
                                     std::uint32_t(7U)));
    }
};
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestRoundUp, TestSet::perBuild());
