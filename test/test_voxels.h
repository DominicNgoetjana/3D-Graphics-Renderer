#ifndef TILER_TEST_VOXELS_H
#define TILER_TEST_VOXELS_H


#include <string>
#include <cppunit/extensions/HelperMacros.h>
#include "tesselate/voxels.h"

/*
namespace CppUnit
{

template<>
class assertion_traits<Region>
{
public:
    static bool equal(const Region &a, const Region &b);
    static std::string toString(const Region &region);
};

} // namespace CppUnit
*/

/// Test code for @ref VoxelVolume
class TestVoxels : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestVoxels);
    CPPUNIT_TEST(testDim);
    CPPUNIT_TEST(testFill);
    CPPUNIT_TEST(testBounds);
    CPPUNIT_TEST_SUITE_END();

private:
    VoxelVolume * vox;

public:

    void setUp();

    void tearDown();

    void testDim();

    void testFill();

    void testBounds();
};

#endif /* !TILER_TEST_VOXELS_H */
