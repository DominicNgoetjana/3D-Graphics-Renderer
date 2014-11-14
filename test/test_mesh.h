#ifndef TILER_TEST_MESH_H
#define TILER_TEST_MESH_H


#include <string>
#include <cppunit/extensions/HelperMacros.h>
#include "tesselate/mesh.h"

/// Test code for @ref Mesh
class TestMesh : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestMesh);
    CPPUNIT_TEST(testMeshing);
    CPPUNIT_TEST_SUITE_END();

private:
    Mesh * mesh;

public:

    void setUp();

    void tearDown();

    /// run standard validity tests on scene
    void testMeshing();
};

#endif /* !TILER_TEST_MESH_H */
