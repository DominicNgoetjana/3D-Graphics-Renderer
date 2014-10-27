/**
 * @file
 *
 * Test code for @ref region.cpp.
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <cstdint>
#include <sstream>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>
#include <common/region.h>
#include <test/testutil.h>
#include "test_region.h"

namespace CppUnit
{

bool assertion_traits<Region>::equal(const Region &a, const Region &b)
{
    return a == b;
}

std::string assertion_traits<Region>::toString(const Region &region)
{
    std::ostringstream o;
    o << "Region(" << region.x0 << ", " << region.y0 << ", " << region.x1 << ", " << region.y1 << ")";
    return o.str();
}

} // namespace CppUnit

/// Test code for @ref Region
class TestRegion : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestRegion);
    CPPUNIT_TEST(testWidthHeight);
    CPPUNIT_TEST(testPixels);
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testUnion);
    CPPUNIT_TEST(testIntersection);
    CPPUNIT_TEST(testIntersectionEmpty);
    CPPUNIT_TEST(testUnionWithEmpty);
    CPPUNIT_TEST(testContains);
    CPPUNIT_TEST(testDilate);
    CPPUNIT_TEST_SUITE_END();

private:
    void testWidthHeight()
    {
        Region r(5, 3, 12, 20);
        CPPUNIT_ASSERT_EQUAL(7, r.width());
        CPPUNIT_ASSERT_EQUAL(17, r.height());
    }

    void testPixels()
    {
        // Empty region with crossover
        Region r;
        r.x0 = 5;
        r.y0 = 3;
        CPPUNIT_ASSERT_EQUAL(std::size_t(0), r.pixels());

        // Normal region
        r = Region(2, -20, 9, -11);
        CPPUNIT_ASSERT_EQUAL(std::size_t(63), r.pixels());
#if SIZE_MAX >= UINT64_MAX
        // Large region
        r = Region(1, 2, 100000001, 1000000002);
        CPPUNIT_ASSERT_EQUAL(std::size_t(100000000000000000LL), r.pixels());
#endif
    }

    void testEmpty()
    {
        Region e1;
        Region e2(5, 10, 5, 20);
        Region e3(5, 10, 12, 10);
        Region n1(5, 5, 20, 20);
        CPPUNIT_ASSERT(e1.empty());
        CPPUNIT_ASSERT(e2.empty());
        CPPUNIT_ASSERT(e3.empty());
        CPPUNIT_ASSERT(!n1.empty());
    }

    void testUnionHelper(const Region &c)
    {
        CPPUNIT_ASSERT_EQUAL(5, c.x0);
        CPPUNIT_ASSERT_EQUAL(4, c.y0);
        CPPUNIT_ASSERT_EQUAL(15, c.x1);
        CPPUNIT_ASSERT_EQUAL(17, c.y1);
    }

    void testUnion()
    {
        Region a(5, 10, 12, 17);
        Region b(7, 4, 15, 7);
        testUnionHelper(a | b);
        testUnionHelper(b | a);
        Region c = a;
        c |= b;
        testUnionHelper(c);
        c = b;
        c |= a;
        testUnionHelper(c);
    }

    void testIntersectionHelper(const Region &c)
    {
        CPPUNIT_ASSERT_EQUAL(7, c.x0);
        CPPUNIT_ASSERT_EQUAL(10, c.y0);
        CPPUNIT_ASSERT_EQUAL(12, c.x1);
        CPPUNIT_ASSERT_EQUAL(11, c.y1);
    }

    void testIntersection()
    {
        Region a(5, 10, 12, 17);
        Region b(7, 4, 15, 11);
        testIntersectionHelper(a & b);
        testIntersectionHelper(b & a);
        Region c = a;
        c &= b;
        testIntersectionHelper(c);
        c = b;
        c &= a;
        testIntersectionHelper(c);
    }

    void testIntersectionEmptyHelper(const Region &c)
    {
        CPPUNIT_ASSERT(c.empty());
        CPPUNIT_ASSERT_EQUAL(0, c.x0);
        CPPUNIT_ASSERT_EQUAL(0, c.y0);
        CPPUNIT_ASSERT_EQUAL(0, c.x1);
        CPPUNIT_ASSERT_EQUAL(0, c.y1);
    }

    void testIntersectionEmpty()
    {
        Region a(5, 10, 12, 17);
        Region b(7, 4, 15, 7);
        testIntersectionEmptyHelper(a & b);
        Region c = a;
        c &= b;
        testIntersectionEmptyHelper(c);
    }

    void testUnionWithEmpty()
    {
        Region a(5, 4, 15, 17);
        Region b;
        testUnionHelper(a | b);
        testUnionHelper(b | a);
        Region c = a;
        c |= b;
        testUnionHelper(c);
        c = b;
        c |= a;
        testUnionHelper(c);
    }

    void testContains()
    {
        // Empty, empty
        CPPUNIT_ASSERT(Region(5, 5, 5, 5).contains({2, 6, 2, 6}));
        // Empty, non-empty
        CPPUNIT_ASSERT(!Region(5, 10, 5, 10).contains({5, 10, 6, 11}));
        // Non-empty, non-empty
        CPPUNIT_ASSERT(Region(5, 10, 12, 20).contains({5, 10, 12, 20}));
        CPPUNIT_ASSERT(Region(5, 10, 12, 20).contains({6, 15, 8, 18}));
        CPPUNIT_ASSERT(!Region(5, 10, 12, 20).contains({4, 15, 8, 18}));
        CPPUNIT_ASSERT(!Region(5, 10, 12, 20).contains({6, 5, 8, 18}));
        CPPUNIT_ASSERT(!Region(5, 10, 12, 20).contains({6, 15, 13, 18}));
        CPPUNIT_ASSERT(!Region(5, 10, 12, 20).contains({6, 15, 8, 21}));
    }

    void testDilate()
    {
        // Empty region does nothing
        CPPUNIT_ASSERT_EQUAL(Region(), Region(5, 7, 6, 7).dilate(4));
        CPPUNIT_ASSERT_EQUAL(Region(), Region(5, 7, 12, 7).dilate(-1));
        // Normal dilation
        CPPUNIT_ASSERT_EQUAL(Region(2, 4, 13, 23), Region(5, 7, 10, 20).dilate(3));
        // Normal erosion
        CPPUNIT_ASSERT_EQUAL(Region(5, 7, 10, 20), Region(2, 4, 13, 23).dilate(-3));
        // Erosion to nothing
        CPPUNIT_ASSERT_EQUAL(Region(), Region(5, 7, 10, 20).dilate(-3));
    }
};
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestRegion, TestSet::perBuild());
