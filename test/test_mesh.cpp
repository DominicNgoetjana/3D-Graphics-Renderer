#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <test/testutil.h>
#include "test_mesh.h"
#include <stdio.h>
#include <cstdint>
#include <sstream>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

void TestMesh::setUp()
{
    mesh = new Mesh();
}

void TestMesh::tearDown()
{
    delete mesh;
}

void TestMesh::testMeshing()
{
    mesh->readSTL("../bunny.stl");
    CPPUNIT_ASSERT(mesh->basicValidity());
    CPPUNIT_ASSERT(!mesh->manifoldValidity()); // bunny has known holes in the bottom
    CPPUNIT_ASSERT(mesh->connectionValidity());

    // test point in cylinder routine
    Cylinder cyl;
    cyl.s = vpPoint(1.0f, 0.0f, 0.0f);
    cyl.e = vpPoint(2.0f, 1.0f, 0.0f);
    cyl.r = 0.5f;
    CPPUNIT_ASSERT(!cyl.pointInCylinder(vpPoint(0.0f, -1.0f, 0.0f)));
    CPPUNIT_ASSERT(cyl.pointInCylinder(vpPoint(1.5f, 0.5f, 0.1f)));
}

//#if 0 /* Disabled since it crashes the whole test suite */
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMesh, TestSet::perCommit());
//#endif
