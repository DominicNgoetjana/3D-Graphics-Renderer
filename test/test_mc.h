#ifndef TILER_TEST_MC_H
#define TILER_TEST_MC_H

#include <string>
#include <cppunit/extensions/HelperMacros.h>
#include "tesselate/csg.h"

/// Test code for @ref VoxelVolume
class TestMC : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestMC);
    CPPUNIT_TEST(testSimpleMC);
    CPPUNIT_TEST_SUITE_END();

private:
    Scene * csg;

public:

    /// Initialization before unit tests
    void setUp();

    /// Tidying up after unit tests
    void tearDown();

    /** 
     * Run simple set and get validity tests on marching cubes
     */
    void testSimpleMC();
};

#endif /* !TILER_TEST_MC_H */
