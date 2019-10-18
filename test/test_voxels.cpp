#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <test/testutil.h>
#include "test_voxels.h"
#include <stdio.h>
#include <cstdint>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

void TestVoxels::setUp()
{
    vox = new VoxelVolume();
}

void TestVoxels::tearDown()
{
    delete vox;
}

void TestVoxels::testVoxelSet()
{
    int v, i, j, k;

    vox->setDim(1024, 1024, 1024); // typical large voxel volume
    vox->setFrame(cgp::Point(0.0f, 0.0f, 0.0f), cgp::Vector(1.0f, 1.0f, 1.0f)); // unit cube

    srand (time(NULL));

    // check fill at random positions
    vox->fill(true);
    for(v = 0; v < 10; v++)
    {
        // check random positions in voxel volume
        CPPUNIT_ASSERT(vox->get(rand()%1024,rand()%1024,rand()%1024));
    }

    // check set and get for some corners and center
    vox->fill(false);
    vox->set(0,0,0,true);
    vox->set(0,1023,1023,true);
    vox->set(1023,1023,0,true);
    vox->set(1023,0,1023,true);
    vox->set(1023,1023,1023,true);
    CPPUNIT_ASSERT(vox->get(0,0,0));
    CPPUNIT_ASSERT(vox->get(0,1023,1023));
    CPPUNIT_ASSERT(vox->get(1023,1023,0));
    CPPUNIT_ASSERT(vox->get(1023,0,1023));
    CPPUNIT_ASSERT(vox->get(1023,1023,1023));

    // set and get outside volume bounds
    CPPUNIT_ASSERT(!vox->set(-1,-1,-1,true));
    CPPUNIT_ASSERT(!vox->set(1024,1024,1024,true));
    CPPUNIT_ASSERT(!vox->get(-1,-1,-1));
    CPPUNIT_ASSERT(!vox->get(1024,1024,1024));

    // check and set random positions in the volume
    vox->fill(true);
    for(v = 0; v < 10; v++)
    {
        // check random positions in voxel volume
        i = rand()%1024; j = rand()%1024; k = rand()%1024;
        CPPUNIT_ASSERT(vox->set(i,j,k, false));
        CPPUNIT_ASSERT(!vox->get(i,j,k));
    }
    cerr << "VOXEL CORRECTNESS PASSED" << endl;
}

void TestVoxels::testVoxelRegistration()
{
    vox->setDim(1024, 1024, 1024); // typical large voxel volume
    vox->setFrame(cgp::Point(0.0f, 0.0f, 0.0f), cgp::Vector(1.0f, 1.0f, 1.0f)); // unit cube

    // check some corner positions and center voxel
    CPPUNIT_ASSERT(vox->getVoxelPos(0,0,0) == cgp::Point(0.0f, 0.0f, 0.0f));
    CPPUNIT_ASSERT(vox->getVoxelPos(1023,0,0) == cgp::Point(1.0f, 0.0f, 0.0f));
    CPPUNIT_ASSERT(vox->getVoxelPos(0,1023,0) == cgp::Point(0.0f, 1.0f, 0.0f));
    CPPUNIT_ASSERT(vox->getVoxelPos(0,0,1023) == cgp::Point(0.0f, 0.0f, 1.0f));
    CPPUNIT_ASSERT(vox->getVoxelPos(1023,1023,1023) == cgp::Point(1.0f, 1.0f, 1.0f));
    CPPUNIT_ASSERT(vox->getVoxelPos(511,511,511) == cgp::Point(0.4995112f, 0.4995112f, 0.4995112f));
    cerr << "VOXEL REGISTRATION PASSED" << endl << endl;
}

//#if 0 /* Disabled since it crashes the whole test suite */
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestVoxels, TestSet::perBuild());
//#endif
