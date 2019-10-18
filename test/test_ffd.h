#ifndef TILER_TEST_FFD_H
#define TILER_TEST_FFD_H

#include <string>
#include <cppunit/extensions/HelperMacros.h>
#include "tesselate/ffd.h"

/// Test code for @ref VoxelVolume
class TestFFD : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestFFD);
    CPPUNIT_TEST(testCorners);
    CPPUNIT_TEST(testCenter);
    CPPUNIT_TEST(testIdentity);
    CPPUNIT_TEST_SUITE_END();

private:
    ffd * linearffd, * cubicffd, * mixffd;

public:

    /// Initialization before unit tests
    void setUp();

    /// Tidying up after unit tests
    void tearDown();

    /** 
     * Test that vertices at the corners of an FFD lattice track the control points
     */
    void testCorners();

    /**
     * Test that a vertex in the center of an FFD lattice is deformed correctly
     */
    void testCenter();

    /**
     * Place random points in an undeformed FFD lattice and make sure that they are return unchanged
     */
    void testIdentity();
};

#endif /* !TILER_TEST_FFD_H */
