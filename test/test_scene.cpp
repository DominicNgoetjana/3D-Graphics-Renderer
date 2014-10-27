#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <test/testutil.h>
#include "test_scene.h"
#include <stdio.h>
#include <cstdint>
#include <sstream>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

void TestScene::setUp()
{
    scene = new Scene();
}

void TestScene::tearDown()
{
    delete scene;
}

void TestScene::testTesselate()
{
    scene->setTessLength(0.5f);
    CPPUNIT_ASSERT(scene->duplicateValidity());
    CPPUNIT_ASSERT(scene->danglingVertValidity());
    CPPUNIT_ASSERT(scene->edgeBoundValidity());
    CPPUNIT_ASSERT(scene->connectionValidity());
}

void TestScene::testValidity()
{
    Scene vscene;

    vscene.clear();
    vscene.addCube(vpPoint(0.0f, 0.0f, 0.0f), 1.0f);
    vscene.addCube(vpPoint(10.0f, 10.0f, 10.0f), 1.0f);
    CPPUNIT_ASSERT(!vscene.connectionValidity());

    vscene.addVertex(vpPoint(0.0f, 0.0f, 0.0f));
    vscene.addVertex(vpPoint(0.0f, 0.0f, 0.0f));
    CPPUNIT_ASSERT(!vscene.danglingVertValidity());
    CPPUNIT_ASSERT(!vscene.duplicateValidity());

    vscene.addEdge(100, 1000);
    CPPUNIT_ASSERT(!vscene.edgeBoundValidity());
}

//#if 0 /* Disabled since it crashes the whole test suite */
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestScene, TestSet::perBuild());
//#endif
