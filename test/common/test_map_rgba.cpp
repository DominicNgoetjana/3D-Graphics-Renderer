/**
 * @file
 *
 * Test code for @ref rgba_tag.
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>
#include <array>
#include <cmath>
#include <common/map.h>
#include <common/map_rgba.h>
#include <test/testutil.h>
#include <test/common/test_map.h>
#include <test/common/test_region.h>

template<>
class TestMapTraits<gray_tag>
{
public:
    static MapTraits<gray_tag>::type testValue(int x, int y)
    {
        return x * 1024.0f + y;
    }
};

template<>
class TestMapTraits<rgba_tag>
{
public:
    static MapTraits<rgba_tag>::type testValue(int x, int y)
    {
        return {{x + 0.0f, y + 1.0f, 0.001f * x, 0.002f * y}};
    }
};

/** Grayscale conversion using the coefficients from ImageMagick */
static float colorToGray(const std::array<float, 4> &v)
{
    return 0.212656 * v[0] + 0.715158 * v[1] + 0.072186 * v[2];
}

static float linearize(int v)
{
    double s = v / 255.0;
    if (s < 0.04045)
        return s / 12.92;
    else
        return std::pow((s + 0.055) / 1.055, 2.4);
}

static std::array<float, 4> linearize(const std::array<int, 4> &v)
{
    std::array<float, 4> out;
    for (int i = 0; i < 3; i++)
        out[i] = linearize(v[i]);
    // alpha is always linear
    out[3] = v[3] / 255.0f;
    return out;
}

/**
 * Raw pixel values in test PNG image.
 */
static const std::array<int, 4> pixels[2][2] =
{
    { {{255, 0, 0, 255}}, {{63, 127, 255, 255}} },
    { {{0, 0, 255, 255}}, {{0, 158, 0, 147}} }
};

/**
 * Tests for custom image read/write with @ref gray_tag.
 */
class TestMemMapGray : public TestMemMap<gray_tag>
{
    CPPUNIT_TEST_SUB_SUITE(TestMemMapGray, TestMemMap<gray_tag>);
    CPPUNIT_TEST(testReadPNG);
    CPPUNIT_TEST(testWritePNG);
    CPPUNIT_TEST_SUITE_END();

private:
    void testReadPNG()
    {
        MemMap<gray_tag> map;
        map.read("../test/common/test2x2.png");
        CPPUNIT_ASSERT_EQUAL(Region(0, 0, 2, 2), map.region());
        for (int y = 0; y < 2; y++)
            for (int x = 0; x < 2; x++)
            {
                std::array<float, 4> pixel = linearize(pixels[y][x]);
                float expected = colorToGray(pixel);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, map[y][x], 1e-5);
            }
    }

    void testWritePNG()
    {
        TempDirectory tmpDir("test_gray");

        MemMap<gray_tag> map;
        map.read("../test/common/test2x2.png");

        map.write("test_gray/test2x2write.png");
        MemMap<gray_tag> map2("test_gray/test2x2write.png");

        CPPUNIT_ASSERT_EQUAL(map.region(), map2.region());
        CPPUNIT_ASSERT_EQUAL(map.step(), map2.step());
        const Region r = map.region();
        for (int y = r.y0; y < r.y1; y++)
            for (int x = r.x0; x < r.x1; x++)
                CPPUNIT_ASSERT_EQUAL(map[y][x], map2[y][x]);
    }
};
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMemMapGray, TestSet::perCommit());

/**
 * Tests for custom image read/write with @ref rgba_tag.
 */
class TestMemMapRGBA : public TestMemMap<rgba_tag>
{
    CPPUNIT_TEST_SUB_SUITE(TestMemMapRGBA, TestMemMap<rgba_tag>);
    CPPUNIT_TEST(testReadPNG);
    CPPUNIT_TEST(testWritePNG);
    CPPUNIT_TEST_SUITE_END();

private:
    void testReadPNG()
    {
        MemMap<rgba_tag> map;
        map.read("../test/common/test2x2.png");
        CPPUNIT_ASSERT_EQUAL(Region(0, 0, 2, 2), map.region());
        for (int y = 0; y < 2; y++)
            for (int x = 0; x < 2; x++)
            {
                std::array<float, 4> pixel = linearize(pixels[y][x]);
                for (int i = 0; i < 4; i++)
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(pixel[i], map[y][x][i], 1e-5);
            }
    }

    void testWritePNG()
    {
        TempDirectory tmpDir("test_rgba");

        MemMap<rgba_tag> map;
        map.read("../test/common/test2x2.png");

        map.write("test_rgba/test2x2write.png");
        MemMap<rgba_tag> map2("test_rgba/test2x2write.png");

        CPPUNIT_ASSERT_EQUAL(map.region(), map2.region());
        CPPUNIT_ASSERT_EQUAL(map.step(), map2.step());
        const Region r = map.region();
        for (int y = r.y0; y < r.y1; y++)
            for (int x = r.x0; x < r.x1; x++)
                CPPUNIT_ASSERT_EQUAL(map[y][x], map2[y][x]);
    }
};
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMemMapRGBA, TestSet::perCommit());
