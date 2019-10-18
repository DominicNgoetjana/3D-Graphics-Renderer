#ifndef TILER_TEST_VOXEL_H
#define TILER_TEST_VOXEL_H


#include <string>
#include <cppunit/extensions/HelperMacros.h>
#include "tesselate/voxels.h"

/// Test code for @ref VoxelVolume
class TestVoxels : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestVoxels);
    CPPUNIT_TEST(testVoxelSet);
    CPPUNIT_TEST(testVoxelRegistration);
    CPPUNIT_TEST_SUITE_END();

private:
    VoxelVolume * vox;

public:

    /// Initialization before unit tests
    void setUp();

    /// Tidying up after unit tests
    void tearDown();

    /** 
     * Run simple set and get validity tests on voxels
     */
    void testVoxelSet();

    /**
     * Check correspondence of voxel elements to 3D position
     */
    void testVoxelRegistration();
};

#endif /* !TILER_TEST_VOXEL_H */
