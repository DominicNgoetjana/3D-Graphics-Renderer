#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <test/testutil.h>
#include "test_mc.h"
#include <stdio.h>
#include <cstdint>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

void TestMC::setUp()
{
    csg = new Scene();
}

void TestMC::tearDown()
{
    delete csg;
}

void TestMC::testSimpleMC()
{
    // set a single voxel in the volume and make sure that an octahedron
    // surrounding that voxel is produced by marching cubes

    int dx, dy, dz;

    csg->voxelise(1.0f); // empty scene
    csg->getVox()->getDim(dx, dy, dz);
    csg->getVox()->set(dx/2,dy/2,dz/2,true); // set single voxel at volume center
    csg->isoextract(); // run marching cubes

    // check that octahedron results
    CPPUNIT_ASSERT(csg->getMesh()->getNumVerts() == 6);
    CPPUNIT_ASSERT(csg->getMesh()->getNumFaces() == 8);
    cgp::Point p = csg->getVox()->getVoxelPos(dx/2,dy/2,dz/2); // this must be inside the mesh because it is located exactly at the set voxel
    CPPUNIT_ASSERT(csg->getMesh()->pointContainment(p));
    CPPUNIT_ASSERT(csg->getMesh()->manifoldValidity());
    cerr << "MARCHING CUBES SIMPLE TEST PASSED" << endl << endl;
}

//#if 0 /* Disabled since it crashes the whole test suite */
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMC, TestSet::perBuild());
//#endif
