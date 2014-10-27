/**
 * @file
 *
 * Helper code for testing various @ref Map subclasses.
 */

#ifndef UTS_TEST_COMMON_TEST_MAP_H
#define UTS_TEST_COMMON_TEST_MAP_H

#include <cppunit/extensions/HelperMacros.h>
#include <sstream>
#include <array>
#include <cstddef>
#include <string>
#include <cstring>
#include <type_traits>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <common/map.h>

/// Traits class of helper functions per tag.
template<typename Tag>
class TestMapTraits
{
};

template<>
class TestMapTraits<height_tag>
{
public:
    static MapTraits<height_tag>::type testValue(int x, int y);
};

template<>
class TestMapTraits<mask_tag>
{
public:
    static MapTraits<mask_tag>::type testValue(int x, int y);
};

template<>
class TestMapTraits<height_and_mask_tag>
{
public:
    static MapTraits<height_and_mask_tag>::type testValue(int x, int y);
};

template<>
class TestMapTraits<coords_tag>
{
public:
    static MapTraits<coords_tag>::type testValue(int x, int y);
};

template<>
class TestMapTraits<coords_offset_tag>
{
public:
    static MapTraits<coords_offset_tag>::type testValue(int x, int y);
};

template<>
class TestMapTraits<appearance_tag>
{
public:
    static MapTraits<appearance_tag>::type testValue(int x, int y);
};

template<>
class TestMapTraits<height_constraint_tag>
{
public:
    static MapTraits<height_constraint_tag>::type testValue(int x, int y);
};

namespace detail
{

template<>
class AssertionTraitsEqual<MapTraits<coords_tag>::type>
{
public:
    static bool equal(const MapTraits<coords_tag>::type &x,
                      const MapTraits<coords_tag>::type &y)
    {
        return AssertionTraitsEqual<decltype(x.flat)>::equal(x.flat, y.flat);
    }
};

template<>
class AssertionTraitsEqual<MapTraits<height_and_mask_tag>::type>
{
public:
    static bool equal(const MapTraits<height_and_mask_tag>::type &x,
                      const MapTraits<height_and_mask_tag>::type &y)
    {
        return AssertionTraitsEqual<decltype(x.height)>::equal(x.height, y.height)
            && AssertionTraitsEqual<decltype(x.mask)>::equal(x.mask, y.mask);
    }
};

template<>
class AssertionTraitsEqual<MapTraits<coords_offset_tag>::type>
{
public:
    static bool equal(const MapTraits<coords_offset_tag>::type &x,
                      const MapTraits<coords_offset_tag>::type &y)
    {
        return AssertionTraitsEqual<decltype(x.flat)>::equal(x.flat, y.flat)
            && AssertionTraitsEqual<decltype(x.offset)>::equal(x.offset, y.offset);
    }
};

template<>
class AssertionTraitsEqual<MapTraits<height_constraint_tag>::type>
{
public:
    static bool equal(const MapTraits<height_constraint_tag>::type &x,
                      const MapTraits<height_constraint_tag>::type &y)
    {
        return AssertionTraitsEqual<decltype(x.h)>::equal(x.h, y.h)
            && AssertionTraitsEqual<decltype(x.a)>::equal(x.a, y.a)
            && AssertionTraitsEqual<decltype(x.c)>::equal(x.c, y.c)
            && AssertionTraitsEqual<decltype(x.t)>::equal(x.t, y.t);
    }
};

template<typename Tag>
void assertMapsEqual(
    const MemMap<Tag> &expected, const MemMap<Tag> &actual,
    CppUnit::SourceLine sourceLine, const std::string &message)
{
    assertEquals(expected.region(), actual.region(), sourceLine, message, "regions differ");
    assertEquals(expected.step(), actual.step(), sourceLine, message, "step sizes differ");
    const Region r = expected.region();
    for (int y = r.y0; y < r.y1; y++)
        for (int x = r.x0; x < r.x1; x++)
        {
            assertEquals(expected[y][x], actual[y][x], sourceLine, message,
                         "equality assertion failed on element (" + std::to_string(x) + ", " + std::to_string(y) + ")");
        }
}

template<typename T1, typename T2>
void assertMapsEqual(
    const T1 &expected, const T2 &actual,
    CppUnit::SourceLine sourceLine, const std::string &message)
{
    assertMapsEqual(expected.toMemMap(expected.region()),
                    actual.toMemMap(actual.region()), sourceLine, message);
}

template<typename Tag>
void assertMapArraysEqual(
    const MemMapArray<Tag> &expected, const MemMapArray<Tag> &actual,
    CppUnit::SourceLine sourceLine, const std::string &message)
{
    assertEquals(expected.region(), actual.region(), sourceLine, message,
                 "regions differ");
    assertEquals(expected.arraySize(), actual.arraySize(), sourceLine, message,
                 "array sizes differ");
    for (int i = 0; i < expected.arraySize(); i++)
    {
        assertEquals(expected.step(i), actual.step(i), sourceLine, message,
                     "equality assertion failed: step sizes differ at " + std::to_string(i));
    }
    const Region r = expected.region();
    for (int i = 0; i < expected.arraySize(); i++)
        for (int y = r.y0; y < r.y1; y++)
            for (int x = r.x0; x < r.x1; x++)
            {
                assertEquals(
                    expected[i][y][x], actual[i][y][x], sourceLine, message,
                    "equality assertion failed at element ("
                    + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(i) + ")");
            }
}

template<typename T1, typename T2>
void assertMapArraysEqual(
    const T1 &expected, const T2 &actual,
    CppUnit::SourceLine sourceLine, const std::string &message)
{
    assertMapArraysEqual(expected.toMemMapArray(expected.region()),
                         actual.toMemMapArray(actual.region()), sourceLine, message);
}

#define UTS_ASSERT_MAPS_EQUAL(expected, actual) \
    (detail::assertMapsEqual((expected), (actual), CPPUNIT_SOURCELINE(), ""))
#define UTS_ASSERT_MAPS_EQUAL_MESSAGE(message, expected, actual) \
    (detail::assertMapsEqual((expected), (actual), CPPUNIT_SOURCELINE(), (message)))
#define UTS_ASSERT_MAP_ARRAYS_EQUAL(expected, actual) \
    (detail::assertMapArraysEqual((expected), (actual), CPPUNIT_SOURCELINE(), ""))
#define UTS_ASSERT_MAP_ARRAYS_EQUAL_MESSAGE(message, expected, actual) \
    (detail::assertMapArraysEqual((expected), (actual), CPPUNIT_SOURCELINE(), (message)))

} // namespace detail


namespace CppUnit
{

#define ASSERTION_PRINTER(T) \
    template<> class assertion_traits<T> : public detail::AssertionTraitsEqual<T> \
    { \
    public: \
        static std::string toString(const T &x); \
    }

ASSERTION_PRINTER(MapTraits<height_and_mask_tag>::type);
ASSERTION_PRINTER(MapTraits<coords_tag>::type);
ASSERTION_PRINTER(MapTraits<coords_offset_tag>::type);
ASSERTION_PRINTER(MapTraits<height_constraint_tag>::type);

#undef ASSERTION_PRINTER

} // namespace CppUnit

/// Tests for @ref MemMap
template<typename Tag>
class TestMemMap : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(TestMemMap);
    CPPUNIT_TEST(testConstruct);
    CPPUNIT_TEST(testAllocate);
    CPPUNIT_TEST(testTranslateTo);
    CPPUNIT_TEST(testWriteRead);
    CPPUNIT_TEST(testSerialize);
    CPPUNIT_TEST(testStep);
    CPPUNIT_TEST(testMoveConstruct);
    CPPUNIT_TEST(testMoveAssign);
    CPPUNIT_TEST(testFill);
    CPPUNIT_TEST(testIndex);
    CPPUNIT_TEST_SUITE_END_ABSTRACT();

protected:
    void testConstruct()
    {
        MemMap<Tag> empty;
        CPPUNIT_ASSERT_EQUAL(0, empty.width());
        CPPUNIT_ASSERT_EQUAL(0, empty.height());
        CPPUNIT_ASSERT_EQUAL(Region(), empty.region());
        CPPUNIT_ASSERT_EQUAL(0.0f, empty.step());
        CPPUNIT_ASSERT(nullptr == empty.get());

        Region r(-4, -5, 6, 3);
        MemMap<Tag> m(Region(-4, -5, 6, 3));
        CPPUNIT_ASSERT_EQUAL(10, m.width());
        CPPUNIT_ASSERT_EQUAL(8, m.height());
        CPPUNIT_ASSERT_EQUAL(r, m.region());
        CPPUNIT_ASSERT_EQUAL(0.0f, m.step());
        CPPUNIT_ASSERT(nullptr != m.get());
    }

    void testAllocate()
    {
        MemMap<Tag> m;

        m.allocate(Region(-1, 10, 6, 14));
        CPPUNIT_ASSERT_EQUAL(7, m.width());
        CPPUNIT_ASSERT_EQUAL(4, m.height());
        CPPUNIT_ASSERT_EQUAL(Region(-1, 10, 6, 14), m.region());
        auto ptr = m.get();

        // Reallocate same size: should not change the pointer
        m.allocate(Region(1, 5, 8, 9));
        CPPUNIT_ASSERT_EQUAL(7, m.width());
        CPPUNIT_ASSERT_EQUAL(4, m.height());
        CPPUNIT_ASSERT_EQUAL(Region(1, 5, 8, 9), m.region());
        CPPUNIT_ASSERT_EQUAL(ptr, m.get());

        // Change the size: should reallocate the pointer
        m.allocate(Region(-10, -10, 15, -5));
        CPPUNIT_ASSERT_EQUAL(25, m.width());
        CPPUNIT_ASSERT_EQUAL(5, m.height());
        CPPUNIT_ASSERT_EQUAL(Region(-10, -10, 15, -5), m.region());
        CPPUNIT_ASSERT(ptr != m.get());

        // Free the memory
        m.allocate(Region());
        CPPUNIT_ASSERT_EQUAL(0, m.width());
        CPPUNIT_ASSERT_EQUAL(0, m.height());
        CPPUNIT_ASSERT(nullptr == m.get());
    }

    void testTranslateTo()
    {
        Region r(-5, 3, 10, 20);
        Region r2(-7, 2, 8, 19);
        MemMap<Tag> m(r);
        MemMap<Tag> ref(r2);
        for (int y = r.y0; y < r.y1; y++)
            for (int x = r.x0; x < r.x1; x++)
                m[y][x] = TestMapTraits<Tag>::testValue(x, y);
        for (int y = r2.y0; y < r2.y1; y++)
            for (int x = r2.x0; x < r2.x1; x++)
                ref[y][x] = TestMapTraits<Tag>::testValue(x + 2, y + 1);

        m.translateTo(-7, 2);
        UTS_ASSERT_MAPS_EQUAL(ref, m);
    }

    template<typename Tag2>
    void testWriteReadImpl(typename std::enable_if<detail::Serializable<Tag2>::value>::type *)
    {
        TempDirectory tmpDir("test_map");
        MemMap<Tag> m;

        Region r(-5, 3, 10, 20);
        m.allocate(r);
        for (int y = r.y0; y < r.y1; y++)
            for (int x = r.x0; x < r.x1; x++)
                m[y][x] = TestMapTraits<Tag>::testValue(x, y);

        m.write("test_map/test_map.exr");

        MemMap<Tag> t;
        t.setStep(1.0f); // should be reset back to 0
        t.read("test_map/test_map.exr");
        UTS_ASSERT_MAPS_EQUAL(m, t);
    }

    template<typename Tag2>
    void testWriteReadImpl(typename std::enable_if<!detail::Serializable<Tag2>::value>::type *)
    {
    }

    void testWriteRead()
    {
        testWriteReadImpl<Tag>(nullptr);
    }

    void testSerialize()
    {
        Region r(-5, 3, 10, 20);
        MemMap<Tag> m(r);
        for (int y = r.y0; y < r.y1; y++)
            for (int x = r.x0; x < r.x1; x++)
                m[y][x] = TestMapTraits<Tag>::testValue(x, y);
        m.setStep(1.5f);

        std::stringstream store;
        boost::archive::text_oarchive oa(store);
        oa << m;

        boost::archive::text_iarchive ia(store);
        MemMap<Tag> t;
        ia >> t;
        UTS_ASSERT_MAPS_EQUAL(m, t);
    }

    void testStep()
    {
        TempDirectory tmpDir("test_map");

        Region r(3, -5, 20, 10);
        MemMap<Tag> m(r);
        CPPUNIT_ASSERT_EQUAL(0.0f, m.step());
        m.setStep(1.25f);
        CPPUNIT_ASSERT_EQUAL(1.25f, m.step());

        // Initialise the values just to avoid valgrind warnings
        for (int y = r.y0; y < r.y1; y++)
            for (int x = r.x0; x < r.x1; x++)
                m[y][x] = TestMapTraits<Tag>::testValue(x, y);
        m.write("test_map/test_map.exr");

        MemMap<Tag> t;
        t.read("test_map/test_map.exr");
        CPPUNIT_ASSERT_EQUAL(1.25f, t.step());
    }

    void testMoveConstruct()
    {
        Region r(-1, -2, 5, 4);
        MemMap<Tag> m1(r);
        m1.setStep(1.25f);
        auto ptr = m1.get();

        MemMap<Tag> m2(std::move(m1));
        CPPUNIT_ASSERT_EQUAL(1.25f, m2.step());
        CPPUNIT_ASSERT_EQUAL(r, m2.region());
        CPPUNIT_ASSERT(ptr == m2.get());
        CPPUNIT_ASSERT(nullptr == m1.get());
    }

    void testMoveAssign()
    {
        Region r1(-1, -3, 5, 4);
        MemMap<Tag> m1(r1);
        m1.setStep(1.25f);
        auto ptr = m1.get();

        Region r2(-4, 4, 6, 7);
        MemMap<Tag> m2(r2);
        m2.setStep(0.5f);

        m2 = std::move(m1);
        CPPUNIT_ASSERT_EQUAL(1.25f, m2.step());
        CPPUNIT_ASSERT_EQUAL(r1, m2.region());
        CPPUNIT_ASSERT(ptr == m2.get());
    }

    void testFill()
    {
        MemMap<Tag> m;
        m.fill({}); // must not crash

        Region r(-3, -5, 2, 3);
        m.allocate(r);

        auto testValue = TestMapTraits<Tag>::testValue(50, 50);
        m.fill(testValue);
        for (int y = r.y0; y < r.y1; y++)
            for (int x = r.x0; x < r.x1; x++)
                CPPUNIT_ASSERT_EQUAL(testValue, m[y][x]);
    }

    void testIndex()
    {
        Region r(-1, -4, 5, 4);
        MemMap<Tag> m(r);
        for (int y = r.y0; y < r.y1; y++)
            for (int x = r.x0; x < r.x1; x++)
                m[y][x] = TestMapTraits<Tag>::testValue(x, y);

        int offset = 0;
        for (int y = r.y0; y < r.y1; y++)
            for (int x = r.x0; x < r.x1; x++, offset++)
            {
                auto expected = TestMapTraits<Tag>::testValue(x, y);
                CPPUNIT_ASSERT_EQUAL(expected, m.get()[offset]);
            }

        // Test with const version
        const MemMap<Tag> &cm = m;
        for (int y = r.y0; y < r.y1; y++)
            for (int x = r.x0; x < r.x1; x++)
            {
                CPPUNIT_ASSERT_EQUAL(m[y][x], cm[y][x]);
            }
    }
};

/// Tests for @ref MemMapArray
template<typename Tag>
class TestMemMapArray : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestMemMapArray);
    CPPUNIT_TEST(testConstruct);
    CPPUNIT_TEST(testAllocate);
    CPPUNIT_TEST(testWriteRead);
    CPPUNIT_TEST(testSerialize);
    CPPUNIT_TEST(testCopyStep);
    CPPUNIT_TEST(testMoveConstruct);
    CPPUNIT_TEST(testMoveAssign);
    CPPUNIT_TEST(testFill);
    CPPUNIT_TEST(testReadSlice);
    CPPUNIT_TEST(testWriteSlice);
    CPPUNIT_TEST(testIndex);
    CPPUNIT_TEST_SUITE_END_ABSTRACT();
public:
    void testConstruct()
    {
        MemMapArray<Tag> empty;
        CPPUNIT_ASSERT_EQUAL(0, empty.width());
        CPPUNIT_ASSERT_EQUAL(0, empty.height());
        CPPUNIT_ASSERT_EQUAL(Region(), empty.region());
        CPPUNIT_ASSERT_EQUAL(0, empty.arraySize());
        CPPUNIT_ASSERT(nullptr == empty.get());

        Region r(-1, -3, 5, 4);
        MemMapArray<Tag> array(r, 3);
        CPPUNIT_ASSERT_EQUAL(6, array.width());
        CPPUNIT_ASSERT_EQUAL(7, array.height());
        CPPUNIT_ASSERT_EQUAL(r, array.region());
        CPPUNIT_ASSERT_EQUAL(3, array.arraySize());
        CPPUNIT_ASSERT(nullptr != array.get());
        for (int i = 0; i < array.arraySize(); i++)
            CPPUNIT_ASSERT_EQUAL(0.0f, array.step(i));
    }

    void testAllocate()
    {
        MemMapArray<Tag> array;

        array.allocate(Region(-1, -3, 7, 4), 3);
        CPPUNIT_ASSERT_EQUAL(8, array.width());
        CPPUNIT_ASSERT_EQUAL(7, array.height());
        CPPUNIT_ASSERT_EQUAL(Region(-1, -3, 7, 4), array.region());
        CPPUNIT_ASSERT_EQUAL(3, array.arraySize());
        auto ptr = array.get();

        // Reallocate same size: should not change the pointer
        array.allocate(Region(-3, 4, 5, 11), 3);
        CPPUNIT_ASSERT_EQUAL(8, array.width());
        CPPUNIT_ASSERT_EQUAL(7, array.height());
        CPPUNIT_ASSERT_EQUAL(Region(-3, 4, 5, 11), array.region());
        CPPUNIT_ASSERT_EQUAL(3, array.arraySize());
        CPPUNIT_ASSERT(ptr == array.get());
        array.setStep(0, 0.25f);
        array.setStep(1, 1.0f);
        array.setStep(2, 2.0f);

        // Change the size: should reallocate the pointer, but preserve
        // existing steps
        array.allocate(Region(1, -3, 8, 9), 5);
        CPPUNIT_ASSERT_EQUAL(7, array.width());
        CPPUNIT_ASSERT_EQUAL(12, array.height());
        CPPUNIT_ASSERT_EQUAL(Region(1, -3, 8, 9), array.region());
        CPPUNIT_ASSERT_EQUAL(5, array.arraySize());
        CPPUNIT_ASSERT(ptr != array.get());
        CPPUNIT_ASSERT_EQUAL(0.25f, array.step(0));
        CPPUNIT_ASSERT_EQUAL(1.0f, array.step(1));
        CPPUNIT_ASSERT_EQUAL(2.0f, array.step(2));

        // Free the memory
        array.allocate(Region(), 0);
        CPPUNIT_ASSERT_EQUAL(0, array.width());
        CPPUNIT_ASSERT_EQUAL(0, array.height());
        CPPUNIT_ASSERT(array.region().empty());
        CPPUNIT_ASSERT_EQUAL(0, array.arraySize());
        CPPUNIT_ASSERT(nullptr == array.get());
    }

    void testWriteRead()
    {
        TempDirectory tmpDir("test_map_array");
        MemMapArray<Tag> m;

        Region r(-23, 5, -3, 17);
        m.allocate(r, 3);
        for (int z = 0; z < m.arraySize(); z++)
        {
            for (int y = r.y0; y < r.y1; y++)
                for (int x = r.x0; x < r.x1; x++)
                    m[z][y][x] = TestMapTraits<Tag>::testValue(x, y + 20 * z);
            m.setStep(z, z + 0.5f);
        }

        m.write("test_map_array/test_map_array");

        MemMapArray<Tag> t;
        t.read("test_map_array/test_map_array");
        UTS_ASSERT_MAP_ARRAYS_EQUAL(m, t);
    }

    void testSerialize()
    {
        MemMapArray<Tag> m;
        Region r(-23, 5, -3, 17);
        m.allocate(r, 3);
        for (int z = 0; z < m.arraySize(); z++)
        {
            for (int y = r.y0; y < r.y1; y++)
                for (int x = r.x0; x < r.x1; x++)
                    m[z][y][x] = TestMapTraits<Tag>::testValue(x, y + 20 * z);
            m.setStep(z, z + 0.5f);
        }

        std::stringstream store;
        boost::archive::text_oarchive oa(store);
        oa << m;

        boost::archive::text_iarchive ia(store);
        MemMapArray<Tag> t;
        ia >> t;
        UTS_ASSERT_MAP_ARRAYS_EQUAL(m, t);
    }

    void testCopyStep()
    {
        MemMapArray<Tag> a1(Region(-1, -2, 7, 4), 3), a2(Region(-10, 2, 5, 5), 3);
        a1.setStep(0, 0.25f);
        a1.setStep(1, 0.5f);
        a1.setStep(2, 3.0f);
        a2.setStep(a1);
        CPPUNIT_ASSERT_EQUAL(0.25f, a2.step(0));
        CPPUNIT_ASSERT_EQUAL(0.5f, a2.step(1));
        CPPUNIT_ASSERT_EQUAL(3.0f, a2.step(2));
    }

    void testMoveConstruct()
    {
        Region r(-1, -5, 5, 4);
        MemMapArray<Tag> a1(r, 3);
        a1.setStep(1, 1.25f);
        auto ptr = a1.get();

        MemMapArray<Tag> a2(std::move(a1));
        CPPUNIT_ASSERT_EQUAL(1.25f, a2.step(1));
        CPPUNIT_ASSERT_EQUAL(6, a2.width());
        CPPUNIT_ASSERT_EQUAL(9, a2.height());
        CPPUNIT_ASSERT_EQUAL(r, a2.region());
        CPPUNIT_ASSERT_EQUAL(3, a2.arraySize());
        CPPUNIT_ASSERT(ptr == a2.get());
        CPPUNIT_ASSERT(nullptr == a1.get());
    }

    void testMoveAssign()
    {
        Region r1(-1, -5, 5, 4);
        MemMapArray<Tag> a1(r1, 3);
        a1.setStep(1, 1.25f);
        auto ptr = a1.get();

        MemMapArray<Tag> a2(Region(0, 0, 6, 3), 4);
        a2.setStep(1, 0.5f);

        a2 = std::move(a1);
        CPPUNIT_ASSERT_EQUAL(1.25f, a2.step(1));
        CPPUNIT_ASSERT_EQUAL(6, a2.width());
        CPPUNIT_ASSERT_EQUAL(9, a2.height());
        CPPUNIT_ASSERT_EQUAL(r1, a2.region());
        CPPUNIT_ASSERT_EQUAL(3, a2.arraySize());
        CPPUNIT_ASSERT(ptr == a2.get());
    }

    void testFill()
    {
        MemMapArray<Tag> m;
        m.fill({}); // must not crash

        Region r(-3, -5, 2, 3);
        m.allocate(r, 4);

        auto testValue = TestMapTraits<Tag>::testValue(50, 50);
        m.fill(testValue);
        for (int z = 0; z < m.arraySize(); z++)
            for (int y = r.y0; y < r.y1; y++)
                for (int x = r.x0; x < r.x1; x++)
                    CPPUNIT_ASSERT_EQUAL(testValue, m[z][y][x]);
    }

    void testReadSlice()
    {
        MemMapArray<Tag> m(Region(-5, -5, 10, 10), 3);
        for (int z = 0; z < 3; z++)
            for (int y = m.region().y0; y < m.region().y1; y++)
                for (int x = m.region().x0; x < m.region().x1; x++)
                    m[z][y][x] = TestMapTraits<Tag>::testValue(x, y + 10 * (z + 1));
        m.setStep(uts::vector<float>{1.5f, 2.5f, 3.5f});

        MemMap<Tag> slice(Region(-4, -6, 9, 9));
        for (int y = slice.region().y0; y < slice.region().y1; y++)
            for (int x = slice.region().x0; x < slice.region().x1; x++)
                slice[y][x] = TestMapTraits<Tag>::testValue(x, y);
        slice.setStep(2.5f);

        Region r(-3, -1, 7, 5);
        m.read(1, slice, r);
        for (int z = 0; z < m.arraySize(); z++)
            for (int y = m.region().y0; y < m.region().y1; y++)
                for (int x = m.region().x0; x < m.region().x1; x++)
                {
                    typename MapTraits<Tag>::type expected;
                    if (z == 1 && y >= r.y0 && y < r.y1 && x >= r.x0 && x < r.x1)
                        expected = TestMapTraits<Tag>::testValue(x, y);
                    else
                        expected = TestMapTraits<Tag>::testValue(x, y + 10 * (z + 1));
                    CPPUNIT_ASSERT_EQUAL(expected, m[z][y][x]);
                }
    }

    void testWriteSlice()
    {
        Region all(-5, -7, 10, 9);
        MemMapArray<Tag> m(all, 3);
        for (int z = 0; z < 3; z++)
            for (int y = m.region().y0; y < m.region().y1; y++)
                for (int x = m.region().x0; x < m.region().x1; x++)
                    m[z][y][x] = TestMapTraits<Tag>::testValue(x, y + 100 * (z + 1));
        m.setStep(uts::vector<float>{1.5f, 1.5f, 3.5f});

        Region sub(-3, 1, 5, 4);
        MemMap<Tag> slice;
        m.write(0, slice); // resizes and resteps slice
        m.write(1, slice, sub);

        CPPUNIT_ASSERT_EQUAL(all, slice.region());
        CPPUNIT_ASSERT_EQUAL(1.5f, slice.step());
        for (int y = all.y0; y < all.y1; y++)
            for (int x = all.x0; x < all.x1; x++)
            {
                int s = (x >= sub.x0 && x < sub.x1 && y >= sub.y0 && y < sub.y1) ? 1 : 0;
                CPPUNIT_ASSERT_EQUAL(slice[y][x], m[s][y][x]);
            }
    }

    void testIndex()
    {
        Region r(-1, -4, 5, 4);
        MemMapArray<Tag> array(r, 3);
        for (int z = 0; z < array.arraySize(); z++)
            for (int y = r.y0; y < r.y1; y++)
                for (int x = r.x0; x < r.x1; x++)
                    array[z][y][x] = TestMapTraits<Tag>::testValue(x, y + 20 * z);

        int offset = 0;
        for (int z = 0; z < array.arraySize(); z++)
            for (int y = r.y0; y < r.y1; y++)
                for (int x = r.x0; x < r.x1; x++, offset++)
                {
                    auto expected = TestMapTraits<Tag>::testValue(x, y + 20 * z);
                    CPPUNIT_ASSERT_EQUAL(expected, array.get()[offset]);
                }

        // Test with const version
        const MemMapArray<Tag> &carray = array;
        for (int z = 0; z < array.arraySize(); z++)
            for (int y = r.y0; y < r.y1; y++)
                for (int x = r.x0; x < r.x1; x++)
                {
                    CPPUNIT_ASSERT_EQUAL(array[z][y][x], carray[z][y][x]);
                }
    }
};

#endif /* !UTS_TEST_COMMON_TEST_MAP_H */
