/**
 * @file
 *
 * Test code for @ref MemMap and related classes
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>
#include <memory>
#include <iostream>
#include <sstream>
#include <array>
#include <common/map.h>
#include <cstdint>
#include <type_traits>
#include <test/testutil.h>
#include "test_map.h"
#include "test_region.h"

template<typename T>
static inline std::string toString(const T &x)
{
    return CppUnit::assertion_traits<T>::toString(x);
}

namespace CppUnit
{

std::string assertion_traits<MapTraits<height_and_mask_tag>::type>::toString(
    const MapTraits<height_and_mask_tag>::type &x)
{
    std::ostringstream o;
    o << "(" << x.height << ", " << x.mask << ")";
    return o.str();
}

std::string assertion_traits<MapTraits<coords_tag>::type>::toString(
    const MapTraits<coords_tag>::type &x)
{
    return ::toString(x.flat);
}

std::string assertion_traits<MapTraits<coords_offset_tag>::type>::toString(
    const MapTraits<coords_offset_tag>::type &x)
{
    std::ostringstream o;
    o << "(" << ::toString(x.flat) << ", " << x.offset << ")";
    return o.str();
}

std::string assertion_traits<MapTraits<height_constraint_tag>::type>::toString(
    const MapTraits<height_constraint_tag>::type &x)
{
    std::ostringstream o;
    o << "(" << x.h << ", " << x.a << ", " << x.c << ", " << x.t << ")";
    return o.str();
}

} // namespace CppUnit

MapTraits<height_tag>::type TestMapTraits<height_tag>::testValue(int x, int y)
{
    return x * 1000.0f + y;
}

MapTraits<mask_tag>::type TestMapTraits<mask_tag>::testValue(int x, int y)
{
    return ((std::uint32_t) x * 0x9e3779b9) ^ (std::uint32_t) y;
}

MapTraits<height_and_mask_tag>::type TestMapTraits<height_and_mask_tag>::testValue(int x, int y)
{
    return {x * 1000.0f + y, ((std::uint32_t) x * 0x9e3779b9) ^ (std::uint32_t) y};
}

MapTraits<appearance_tag>::type TestMapTraits<appearance_tag>::testValue(int x, int y)
{
    static_assert(MapTraits<appearance_tag>::type().size() == 4, "Need to update for new APPEARANCE_MODES");
    return {{
        std::uint16_t(x * 10),
        std::uint16_t(y * 13),
        std::uint16_t(x * 27),
        std::uint16_t(y * 19)
    }};
}

MapTraits<coords_tag>::type TestMapTraits<coords_tag>::testValue(int x, int y)
{
    static_assert(MapTraits<coords_tag>::type().flat.size() == 2, "need to adjust for new coords_t");
    return
    {
        {{
            x * 1.25f + 1.75f,
            y * 2.75f + 1.5f
        }},
    };
}

MapTraits<coords_offset_tag>::type TestMapTraits<coords_offset_tag>::testValue(int x, int y)
{
    return {
        TestMapTraits<coords_tag>::testValue(x, y),
        1.25f + x - 2 * y
    };
}

MapTraits<height_constraint_tag>::type TestMapTraits<height_constraint_tag>::testValue(int x, int y)
{
    return {
        x * 1.75f,
        x * 2.25f,
        y * 0.0001f,
        (float) x + y
    };
}

/// Tests for @ref Map base class
class TestMap : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestMap);
    CPPUNIT_TEST(testFileDimensions);
    CPPUNIT_TEST_EXCEPTION(testFileDimensionsNoFile, Iex::BaseExc);
    CPPUNIT_TEST_SUITE_END();

private:
    void testFileDimensions()
    {
        TempDirectory tmpDir("test_map");

        Region r(1, 2, 5, 10);
        float step = 1.75f;
        MemMap<height_tag> m(r);
        m.setStep(step);
        m.write("test_map/file_dimensions.exr");

        Region r2;
        float step2;
        std::tie(r2, step2) = Map::fileDimensions("test_map/file_dimensions.exr");
        CPPUNIT_ASSERT_EQUAL(r, r2);
        CPPUNIT_ASSERT_EQUAL(step, step2);
    }

    void testFileDimensionsNoFile()
    {
        Map::fileDimensions("test_map/no_such_file.exr");
    }
};
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMap, TestSet::perBuild());

/// Tests for @ref MemMap specialized for @ref height_tag
class TestMemMapHeight : public TestMemMap<height_tag>
{
    CPPUNIT_TEST_SUB_SUITE(TestMemMapHeight, TestMemMap<height_tag>);
    CPPUNIT_TEST(testWriteReadTerragen);
    CPPUNIT_TEST_SUITE_END();

private:
    /**
     * Test that data can be written to Terragen format and reimported.
     *
     * @todo This is
     * a fairly weak test since it does not check that the Terragen file
     * conforms to the standard. This would require having a Terragen file with
     * known contents to import.
     */
    void testWriteReadTerragen()
    {
        TempDirectory tmpDir("test_map");
        MemMap<height_tag> m;

        Region r(0, 0, 15, 17); // Terragen doesn't encode origin
        m.allocate(r);
        m.setStep(10.0f);
        for (int y = r.y0; y < r.y1; y++)
            for (int x = r.x0; x < r.x1; x++)
                m[y][x] = TestMapTraits<height_tag>::testValue(x, y);

        m.write("test_map/test_map.ter");

        MemMap<height_tag> t;
        t.setStep(1.0f); // should be reset by read
        t.read("test_map/test_map.ter");

        // Terragen uses 16-bit heights, so we won't have an exact match
        // and hence can't use UTS_ASSERT_MAPS_EQUAL
        CPPUNIT_ASSERT_EQUAL(m.region(), t.region());
        CPPUNIT_ASSERT_EQUAL(m.step(), t.step());
        for (int y = r.y0; y < r.y1; y++)
            for (int x = r.x0; x < r.x1; x++)
            {
                CPPUNIT_ASSERT_DOUBLES_EQUAL(m[y][x], t[y][x], 0.5);
            }
    }
};
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMemMapHeight, TestSet::perBuild());

/// Tests for @ref MemMap specialized for @ref mask_tag
class TestMemMapMask : public TestMemMap<mask_tag>
{
    CPPUNIT_TEST_SUB_SUITE(TestMemMapMask, TestMemMap<mask_tag>);
    CPPUNIT_TEST_SUITE_END();
};
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMemMapMask, TestSet::perBuild());

/// Tests for @ref MemMap specialized for @ref height_and_mask_tag
class TestMemMapHeightAndMask : public TestMemMap<height_and_mask_tag>
{
    CPPUNIT_TEST_SUB_SUITE(TestMemMapHeightAndMask, TestMemMap<height_and_mask_tag>);
    CPPUNIT_TEST_SUITE_END();
};
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMemMapHeightAndMask, TestSet::perBuild());

/// Tests for @ref MemMap specialized for @ref appearance_tag
class TestMemMapAppearance : public TestMemMap<appearance_tag>
{
    CPPUNIT_TEST_SUB_SUITE(TestMemMapAppearance, TestMemMap<appearance_tag>);
    CPPUNIT_TEST_SUITE_END();
};
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMemMapAppearance, TestSet::perBuild());

/// Tests for @ref MemMap specialized for @ref coords_tag
class TestMemMapCoords : public TestMemMap<coords_tag>
{
    CPPUNIT_TEST_SUB_SUITE(TestMemMapCoords, TestMemMap<coords_tag>);
    CPPUNIT_TEST_SUITE_END();
};
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMemMapCoords, TestSet::perBuild());

/// Tests for @ref MemMap specialized for @ref coords_offset_tag
class TestMemMapCoordsOffset : public TestMemMap<coords_offset_tag>
{
    CPPUNIT_TEST_SUB_SUITE(TestMemMapCoordsOffset, TestMemMap<coords_offset_tag>);
    CPPUNIT_TEST_SUITE_END();
};
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMemMapCoordsOffset, TestSet::perBuild());

/// Tests for @ref MemMapArray specialized for @ref height_tag
class TestMemMapArrayHeight : public TestMemMapArray<height_tag>
{
    CPPUNIT_TEST_SUB_SUITE(TestMemMapArrayHeight, TestMemMapArray<height_tag>);
    CPPUNIT_TEST_SUITE_END();
};
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMemMapArrayHeight, TestSet::perBuild());

/// Tests for @ref MemMapArray specialized for @ref appearance_tag
class TestMemMapArrayAppearance : public TestMemMapArray<appearance_tag>
{
    CPPUNIT_TEST_SUB_SUITE(TestMemMapArrayAppearance, TestMemMapArray<appearance_tag>);
    CPPUNIT_TEST_SUITE_END();
};
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMemMapArrayAppearance, TestSet::perBuild());

/// Tests for @ref MemMapArray specialized for @ref coords_tag
class TestMemMapArrayCoords : public TestMemMapArray<coords_tag>
{
    CPPUNIT_TEST_SUB_SUITE(TestMemMapArrayCoords, TestMemMapArray<coords_tag>);
    CPPUNIT_TEST_SUITE_END();
};
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMemMapArrayCoords, TestSet::perBuild());

/// Tests for @ref MemMapArray specialized for @ref coords_tag
class TestMemMapArrayCoordsOffset : public TestMemMapArray<coords_offset_tag>
{
    CPPUNIT_TEST_SUB_SUITE(TestMemMapArrayCoordsOffset, TestMemMapArray<coords_offset_tag>);
    CPPUNIT_TEST_SUITE_END();
};
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMemMapArrayCoordsOffset, TestSet::perBuild());
