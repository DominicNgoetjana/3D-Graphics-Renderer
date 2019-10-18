//
// csg
//

#include "csg.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <limits>
#include <stack>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace std;
// using namespace cgp;

GLfloat defaultCol[] = {0.243f, 0.176f, 0.75f, 1.0f};

bool Scene::genVizRender(View * view, ShapeDrawData &sdd)
{
    std::vector<ShapeNode *> leaves;
    std::stack<SceneNode *> nodes;
    SceneNode * currnode;
    OpNode * currop;
    ShapeNode * currshape;
    int i;

    geom.clear();
    geom.setColour(defaultCol);

    // gather vector of leaf nodes
    if(csgroot != NULL)
    {
        if(dynamic_cast<ShapeNode*>( csgroot ) == 0) // OpNode
        {
            nodes.push(csgroot);
        }
        else // ShapeNode
        {
            currshape = dynamic_cast<ShapeNode*>( csgroot );
            leaves.push_back((ShapeNode *) csgroot);
        }

        // explicit walk of csg tree using stack
        while(!nodes.empty())
        {
            currnode = nodes.top();
            currop = dynamic_cast<OpNode*> (currnode);
            nodes.pop();

            // left subtree
            if(dynamic_cast<ShapeNode*>( currop->left ) == 0) // cast fails so it is an OpNode
            {
                nodes.push(currop->left);
            }
            else // ShapeNode, so treat as leaf
            {
                currshape = dynamic_cast<ShapeNode*>( currop->left );
                leaves.push_back(currshape);
            }

            // right subtree
            if(dynamic_cast<ShapeNode*>( currop->right ) == 0) // cast fails so it is an OpNode
            {
                nodes.push(currop->right);
            }
            else // ShapeNode, so treat as leaf
            {
                currshape = dynamic_cast<ShapeNode*>( currop->right );
                leaves.push_back(currshape);
            }
        }
    }

    // traverse leaf shapes generating geometry
    for(i = 0; i < (int) leaves.size(); i++)
    {
        leaves[i]->shape->genGeometry(&geom, view);
    }

    // bind geometry to buffers and return drawing parameters, if possible
    if(geom.bindBuffers(view))
    {
        sdd = geom.getDrawParameters();
        return true;
    }
    else
        return false;
}

bool Scene::genVoxRender(View * view, ShapeDrawData &sdd)
{
    int x, y, z, xdim, ydim, zdim;
    glm::mat4 tfm, idt;
    glm::vec3 trs;
    cgp::Point pnt;

    geom.clear();
    geom.setColour(defaultCol);

    if(rep == SceneRep::VOXELS)
    {
        idt = glm::mat4(1.0f); // identity matrix

        vox.getDim(xdim, ydim, zdim);

        // place a sphere at filled voxels but subsample to avoid generating too many spheres
        for(x = 0; x < xdim; x+=10)
            for(y = 0; y < ydim; y+=10)
                for(z = 0; z < zdim; z+=10)
                {
                    if(vox.get(x, y, z))
                    {
                        pnt = vox.getVoxelPos(x, y, z); // convert from voxel space to world coordinates
                        trs = glm::vec3(pnt.x, pnt.y, pnt.z);
                        tfm = glm::translate(idt, trs);
                        geom.genSphere(voxsidelen * 5.0f, 3, 3, tfm);
                    }
                }
    }

    // bind geometry to buffers and return drawing parameters, if possible
    if(geom.bindBuffers(view))
    {
        sdd = geom.getDrawParameters();
        return true;
    }
    else
        return false;
}

Scene::Scene()
{
    csgroot = NULL;
    col = defaultCol;
    voldiag = cgp::Vector(20.0f, 20.0f, 20.0f);
    voxsidelen = 0.0f;
    rep = SceneRep::TREE;
}

Scene::~Scene()
{
    clear();
}

void Scene::clear()
{
    std::stack<SceneNode *> nodes;
    SceneNode * currnode;
    OpNode * currop;
    ShapeNode * currleaf;

    geom.clear();
    vox.clear();

    // walk csg tree and deallocate nodes
    if(csgroot != NULL)
    {
        nodes.push(csgroot);

        // explicit walk of csg tree using stack
        while(!nodes.empty())
        {
            currnode = nodes.top();
            nodes.pop(); // calls destructor

            if(dynamic_cast<OpNode*> (currnode))
            {
                currop = dynamic_cast<OpNode*> (currnode);
                nodes.push(currop->right);
                nodes.push(currop->left);
                // delete currop;
            }
            else
            {
                if(dynamic_cast<ShapeNode*> (currnode))
                {
                    currleaf = dynamic_cast<ShapeNode*> (currnode);
                    // delete currleaf;
                }
                else
                {
                    cerr << "Error Scene::clear(): CSG tree is not properly formed" << endl;
                }
            }
        }
    }
}

bool Scene::bindGeometry(View * view, ShapeDrawData &sdd)
{
    bool pass;

    // call different geometric construction methods depending on current representation
    switch(rep)
    {
        case SceneRep::TREE:
            pass = genVizRender(view, sdd);
            break;
        case SceneRep::VOXELS:
            pass = genVoxRender(view, sdd);
            break;
        case SceneRep::ISOSURFACE:
            pass = voxmesh.bindGeometry(view, sdd);
            break;
        default:
            pass = false;
            break;
    }
    return pass;
}

void Scene::voxSetOp(SetOp op, VoxelVolume *leftarg, VoxelVolume *rightarg)
{
   /*
    switch based on op
    UNION: wherever voxel is set in rightarg copy to leftarg
    INTERSECTION: if voxel is set in leftarg, check to see if it is also set in rightarg, otherwise switch it off
    DIFFERENCE: wherever voxel is set in rightarg turn it off in leftarg
    */

    int x, y, z, dx1, dx2, dy1, dy2, dz1, dz2;

    // check that volume sizes match
    leftarg->getDim(dx1, dy1, dz1);
    rightarg->getDim(dx2, dy2, dz2);
    if(dx1 != dx2 || dy1 != dy2 || dz1 != dz2)
    {
    }
    else
    {
        switch(op)
        {
            case SetOp::UNION: // wherever voxel is set in rightarg copy to leftarg
                for(x = 0; x < dx1; x++)
                    for(y = 0; y < dy1; y++)
                        for(z = 0; z < dz1; z++)
                            if(rightarg->get(x,y,z))
                                leftarg->set(x,y,z,true);
                break;
            case SetOp::INTERSECTION: // if voxel is set in leftarg, check to see if it is also set in rightarg, otherwise switch it off
                for(x = 0; x < dx1; x++)
                    for(y = 0; y < dy1; y++)
                        for(z = 0; z < dz1; z++)
                             if(leftarg->get(x,y,z))
                                 if(!rightarg->get(x,y,z))
                                     leftarg->set(x,y,z,false);
                break;
            case SetOp::DIFFERENCE: // wherever voxel is set in rightarg turn it off in leftarg
                for(x = 0; x < dx1; x++)
                    for(y = 0; y < dy1; y++)
                        for(z = 0; z < dz1; z++)
                            if(rightarg->get(x,y,z))
                                leftarg->set(x,y,z,false);
                break;
            default:
                break;
        }
    } 
}

void Scene::voxWalk(SceneNode *root, VoxelVolume *voxels)
{
    // traverse csg tree by depth first recursive walk
    /*
    if(root is leaf)
        convert to voxel rep, store in voxels
    else
        voxWalk left tree (leftnode, voxels)
        allocate rightvoxels, copy parameters from voxels
        voxWalk right tree (rightnode, rightvoxels)
        apply opp to voxels and rightvoxels store results in voxels
        deallocate rightvoxels
    */

    VoxelVolume * rightvoxels;
    ShapeNode * shapenode;
    OpNode * opnode;
    int dx, dy, dz;
    cgp::Point o;
    cgp::Vector d;

    if(dynamic_cast<ShapeNode*>( root )) // SceneNode
    {
        shapenode = dynamic_cast<ShapeNode*>( root );
        // needs to be improved by only considering bounding box area
        voxels->getDim(dx, dy, dz);

        cerr << "xdim = " << dx << endl;
        for(int x = 0; x < dx; x++)
        {
            #pragma omp parallel for
            for(int y = 0; y < dy; y++)
                for(int z = 0; z < dz; z++)
                    voxels->set(x,y,z, shapenode->shape->pointContainment(voxels->getVoxelPos(x,y,z)));
            int temp = ((float) x/dx)*100;
            if (temp > percentDone && (temp%10==0))
            {
                percentDone = temp;
                cerr << "Percent Complete: " << percentDone << "%" << endl;
            }
        }
    }
    else // OpNode
    {
        opnode = dynamic_cast<OpNode*>( root );
        voxWalk(opnode->left, voxels);
        voxels->getDim(dx, dy, dz);
        voxels->getFrame(o, d);
        rightvoxels = new VoxelVolume(dx, dy, dz, o, d);
        voxWalk(opnode->right, rightvoxels);
        voxSetOp(opnode->op, voxels, rightvoxels);
        delete rightvoxels;
    }

    /* convert to sizeable voxel grid */
    int xspan = voxels->getXSpan();
    int len = dx/xspan;
    vector<int> voxel1d(len, 0);
    vector<vector<int>> voxel2d(len, voxel1d);
    vector<vector<vector<int>>> voxelgrid(len, voxel2d);
    int xcount = 0, ycount = 0, zcount = 0;
    for (int z = 0; z < dz; z++)
    {
        if ((z+1)%xspan == 0) zcount++;   /// if at next xspan, switch to next z voxel
        for (int y = 0; y < dy; y++)
        {
            if ((y+1)%xspan == 0) ycount++;   /// if at next xspan, switch to next y voxel
            for (int x = 0; x < dx; x++)
            {
                int xPos = x/xspan, yPos = y/xspan, zPos = z/xspan;
                if (voxelgrid[xPos][yPos][zPos] == 1) continue;  /// if already identified as a voxel, skip
                voxelgrid[xPos][yPos][zPos] = voxels->get(x,y,z);
                if ((x+1)%xspan == 0) xcount++;   /// if at next xspan, switch to next x voxel
            }
        }
    }
    cout << "writing to voxelisedgrid" << endl;
    getMesh()->writeGrid(voxelgrid, "meshes/voxel/voxelisedgrid", len);
}

void Scene::voxelise(float voxlen)
{
    int xdim, ydim, zdim;

    /// calculate voxel volume dimensions based on voxlen
    xdim = ceil(voldiag.i / voxlen)+2; // needs a 1 voxel border to ensure a closed mesh
    ydim = ceil(voldiag.j / voxlen)+2;
    zdim = ceil(voldiag.k / voxlen)+2;

    voxsidelen = voxlen;
    vox.setDim(xdim, ydim, zdim);

    /// account for expansion in x-dimension by adjusting origin and diagonal
    vox.getDim(xdim, ydim, zdim);
    cgp::Vector voxdiag = cgp::Vector((float) xdim * voxlen, (float) ydim * voxlen, (float) zdim * voxlen);
    cgp::Point voxorigin = cgp::Point(-0.5f*voxdiag.i, -0.5f*voxdiag.j, -0.5f*voxdiag.k);
    vox.setFrame(voxorigin, voxdiag);

    /// map dimensions to closest power of 2

   cerr << "Voxel volume dimensions = " << xdim << " x " << ydim << " x " << zdim << endl;

   percentDone = 0;
    // actual recursive depth-first walk of csg tree
    if(csgroot != NULL)
        voxWalk(csgroot, &vox);

    rep = SceneRep::VOXELS;
}

void Scene::isoextract()
{
    voxmesh.marchingCubes(&vox);
    rep = SceneRep::ISOSURFACE;
}

void Scene::smooth()
{
    voxmesh.laplacianSmooth(6, 1.0f);
}

void Scene::deform(ffd * def)
{
    voxmesh.applyFFD(def);
}

void Scene::sampleScene()
{
    ShapeNode * sph = new ShapeNode();
    sph->shape = new Sphere(cgp::Point(0.0f, 0.0f, 0.0f), 4.0f);

    ShapeNode * cyl1 = new ShapeNode();
    cyl1->shape = new Cylinder(cgp::Point(-7.0f, -7.0f, 0.0f), cgp::Point(7.0f, 7.0f, 0.0f), 2.0f);

    ShapeNode * cyl2 = new ShapeNode();
    cyl2->shape = new Cylinder(cgp::Point(0.0f, -7.0f, 0.0f), cgp::Point(0.0f, 7.0f, 0.0f), 2.5f);

    OpNode * combine = new OpNode();
    combine->op = SetOp::UNION;
    combine->left = sph;
    combine->right = cyl1;

    OpNode * diff = new OpNode();
    diff->op = SetOp::DIFFERENCE;
    diff->left = combine;
    diff->right = cyl2;

    csgroot = diff;
}

void Scene::intersectScene()
{
    ShapeNode * sph = new ShapeNode();
    sph->shape = new Sphere(cgp::Point(0.0f, 0.0f, 0.0f), 5.0f);

    ShapeNode * cyl1 = new ShapeNode();
    cyl1->shape = new Cylinder(cgp::Point(0.0f, 0.0f, -1.0f), cgp::Point(0.0f, 0.0f, 7.0f), 1.0f);

    OpNode * combine = new OpNode();
    combine->op = SetOp::INTERSECTION;
    combine->left = sph;
    combine->right = cyl1;

    csgroot = combine;
}

void Scene::loadSTLScene(string filename)
{
    ShapeNode * mesh = new ShapeNode();
    Mesh * object = new Mesh();
    object->readSTL(filename);
    object->boxFit(30.0f);
    mesh->shape = object;
    csgroot = mesh;
}

void Scene::sphereScene()
{
    ShapeNode * mesh = new ShapeNode();
    Mesh * spheremesh = new Mesh();
    spheremesh->readSTL("meshes/triangle/sphere.stl");
    spheremesh->boxFit(10.0f);
    mesh->shape = spheremesh;
    csgroot = mesh;
}

void Scene::expensiveScene(string filename)
{
    ShapeNode * sph = new ShapeNode();
    sph->shape = new Sphere(cgp::Point(0.0f, 0.0f, 0.0f), 3.0f);

    ShapeNode * cyl = new ShapeNode();
    cyl->shape = new Cylinder(cgp::Point(-11.0f, -11.0f, 0.0f), cgp::Point(11.0f, 11.0f, 0.0f), 2.0f);

    ShapeNode * mesh = new ShapeNode();
    Mesh * loadedModel = new Mesh();
    loadedModel->readSTL(filename);
    loadedModel->boxFit(10.0f);
    mesh->shape = loadedModel;

    OpNode * combine = new OpNode();
    combine->op = SetOp::UNION;
    combine->left = mesh;
    combine->right = cyl;
    /*
     OpNode * diff = new OpNode();
     diff->op = SetOp::DIFFERENCE;
     diff->left = combine;
     diff->right = sph;
     */
    csgroot = mesh;
    rep = SceneRep::TREE;
}

void Scene::readGridVV(std::string filename, int len)
{
    ifstream infile;
    infile.open(filename, ios::in);
    string line;
    vox.clear();
    vox.setDim(len, len, len);
    for (int z = 0; z < len; z++) {
        getline(infile, line);
        for (int y = 0; y < len; y++) {
            vector<string> yValues;
            for (int x = 0; x < len; x++) {
                if (stoi(line.substr(x, 1)) == 1)
                    vox.set(z,y,x, true);
                else
                    vox.set(z,y,x, false);
            }
            if (y < (len-1))
                line = line.substr(line.find(",")+1);
        }
    }

    infile.close();
}

void Scene::displayVoxelScene(string filename)
{
    /// load grid
    readGridVV(filename, 32);

    /// load cube mesh
    ShapeNode * mesh = new ShapeNode();
    Mesh * loadedModel = new Mesh();
    loadedModel->readSTL("meshes/triangle/cube5mm.stl");
    loadedModel->boxFit(2.0f);

    OpNode * combine = new OpNode();
    combine->op = SetOp::UNION;

    vector<Mesh*> loadedModels;
    vector<OpNode*> shapeList;
    vector<ShapeNode*> shapeNodes;

    float offset = 2.0f;
    cgp::Vector pos(-32.0f, -32.0f, -32.0f);
    int entryCount = 0, length = 32;

    /// go through voxel grid and render a cube for each voxel value == 1
    for (int z = 0; z < length; ++z) {
        pos = cgp::Vector(pos.i, pos.j, pos.k + offset); // update z position
        for (int y = 0; y < length; ++y) {
            pos = cgp::Vector(pos.i, pos.j + offset, pos.k); // update y position
            for (int x = 0; x < length; ++x) {
                pos = cgp::Vector(pos.i + offset, pos.j, pos.k); // update x position
                if (vox.get(z, y, x) == 1) { // voxel found, so render cube based on pos
                    loadedModels.push_back(new Mesh());
                    *loadedModels[entryCount] = *loadedModel; // copy cube mesh
                    loadedModels[entryCount]->setTranslation(pos); // place cube at new pos
                    shapeNodes.push_back(new ShapeNode());
                    shapeNodes[entryCount]->shape = loadedModels[entryCount];

                    if (entryCount == 0) {
                        loadedModel->setTranslation(cgp::Vector(pos.i, pos.j, pos.k));
                        mesh->shape = loadedModel;
                        combine->left = mesh;
                        combine->right = mesh;
                        shapeList.push_back(combine);
                    } else {
                        shapeList.push_back(new OpNode());
                        shapeList[entryCount]->op = SetOp::UNION;
                        shapeList[entryCount]->left = shapeList[entryCount-1];
                        shapeList[entryCount]->right = shapeNodes[entryCount];
                    }
                    entryCount++;
                }
            }
            pos = cgp::Vector(-34.0f, pos.j, pos.k); /// reset x position
        }
        pos = cgp::Vector(pos.i + offset, -32.0f, pos.k); /// reset y position
    }

    csgroot = shapeList[shapeList.size()-1];
    rep = SceneRep::TREE;
}

void Scene::voxelScene(string filename)
{
    /// load grid
    readGridVV(filename, 32);

    /// global voxel rep object
    VoxelRep * voxelRep = new VoxelRep ();

    /// test merge mesh
    ShapeNode * finalMesh = new ShapeNode();
    Mesh * testLoadedModel = new Mesh();
    testLoadedModel->readSTL("meshes/triangle/10mm_test_cube.stl");
    Mesh * testLoadedModel2 = new Mesh();
    testLoadedModel2->readSTL("meshes/triangle/10mm_test_cube.stl");
    Mesh * testLoadedModel3 = new Mesh();
    testLoadedModel3->readSTL("meshes/triangle/10mm_test_cube.stl");
    Mesh * testLoadedModel4 = new Mesh();
    testLoadedModel4->readSTL("meshes/triangle/10mm_test_cube.stl");

    /// translate vertices (2nd cube)
    vector<cgp::Point> * vts = testLoadedModel2->getVerts();
    cgp::Point pointToAdd = cgp::Point(20.0f, 0.0f, 0.0f);
    for (std::vector<cgp::Point>::iterator i = vts->begin() ; i != vts->end(); ++i) {
        (*i).add(pointToAdd);
    }

    /// translate vertices (3rd cube)
    vts = testLoadedModel3->getVerts();
    pointToAdd = cgp::Point(0.0f, 0.0f, 20.0f);
    for (std::vector<cgp::Point>::iterator i = vts->begin() ; i != vts->end(); ++i) {
        (*i).add(pointToAdd);
    }

    /// translate vertices (4th cube)
    vts = testLoadedModel4->getVerts();
    pointToAdd = cgp::Point(0.0f, 20.0f, 0.0f);
    for (std::vector<cgp::Point>::iterator i = vts->begin() ; i != vts->end(); ++i) {
        (*i).add(pointToAdd);
    }

    /// re-index triangles (2nd cube)
    vector<Triangle> * trs = testLoadedModel2->getCubeTriangles();
    for (std::vector<Triangle>::iterator i = trs->begin() ; i != trs->end(); ++i) {
        (*i).v[0] += 8;
        (*i).v[1] += 8;
        (*i).v[2] += 8;
    }

    /// re-index triangles (3rd cube)
    vector<Triangle> * trs3 = testLoadedModel3->getCubeTriangles();
    for (std::vector<Triangle>::iterator i = trs3->begin() ; i != trs3->end(); ++i) {
        (*i).v[0] += 16;
        (*i).v[1] += 16;
        (*i).v[2] += 16;
    }

    /// re-index triangles (4th cube)
    vector<Triangle> * trs4 = testLoadedModel4->getCubeTriangles();
    for (std::vector<Triangle>::iterator i = trs4->begin() ; i != trs4->end(); ++i) {
        (*i).v[0] += 24;
        (*i).v[1] += 24;
        (*i).v[2] += 24;
    }

//    cerr << "testLoade/*dModel triangles: " << testLoadedModel->getCubeTriangles()->size() << endl;
//    vector<Triangle> * triangles = testLoadedModel->getCubeTriangles();
//    for (vector<Triangle>::iterator i = triangles->begin(); i != triangles->end(); ++i)
//        i->printVec();

//    scerr << "testLoadedModel verts: " << testLoadedModel->getVerts()->size() << endl;
//    vector<cgp::Point> * vertices = testLoadedModel->getVerts();
//    for (vector<cgp::Point>::iterator i = vertices->begin(); i != vertices->end(); ++i)
//        cerr << i->printPoint() << endl;
//    cerr << endl;

    voxelRep->add(0, {0, 0, 0});
    voxelRep->add(0, {2, 0, 0});
    voxelRep->add(0, {0, 0, 2});
    voxelRep->add(0, {0, 2, 0});

    ///    merge cube 0 and 1
    testLoadedModel->mergeMesh(testLoadedModel2);

    /// delete cube 0 triangles
    cerr << "deleting cube 0 triangles" << endl;
    vector<Triangle> * tris = testLoadedModel->getCubeTriangles();
    cerr << "start pos = " << voxelRep->getStartPos(0) + voxelRep->getSideCode("R", {0, 0, 0}) << endl;
    tris->erase(tris->begin() + voxelRep->getStartPos(0) + voxelRep->getSideCode("R", {0, 0, 0}), tris->begin() + voxelRep->getStartPos(0) + voxelRep->getSideCode("R", {0, 0, 0}) + 2);
    voxelRep->addRemovedFaceCode(0, {0, 0, 0});
    cerr << "updating cube 0" << endl;
    voxelRep->update(2, {0, 0, 0});
    cerr << "done" << endl;

    /// delete cube 1 triangles
    cerr << "deleting cube 1 triangles" << endl;
    tris->erase(tris->begin() + voxelRep->getStartPos(1) + voxelRep->getSideCode("L", {2, 0, 0}), tris->begin() + voxelRep->getStartPos(1) + voxelRep->getSideCode("L", {2, 0, 0}) + 2);
    voxelRep->addRemovedFaceCode(1, {2, 0, 0});
    cerr << "updating cube 1" << endl;
    voxelRep->update(2, {2, 0, 0});
    cerr << "done" << endl;

    ///    merge cube 0 and 2
    testLoadedModel->mergeMesh(testLoadedModel3);

    /// delete cube 0 triangles
    cerr << "deleting cube 0 triangles" << endl;
    cerr << "start pos = " << voxelRep->getStartPos(0) + voxelRep->getSideCode("F", {0, 0, 0}) << endl;
    tris->erase(tris->begin() + voxelRep->getStartPos(0) + voxelRep->getSideCode("F", {0, 0, 0}), tris->begin() + voxelRep->getStartPos(0) + voxelRep->getSideCode("F", {0, 0, 0}) + 2);
    voxelRep->addRemovedFaceCode(0, {0, 0, 0});
    cerr << "updating cube 0" << endl;
    voxelRep->update(2, {0, 0, 0});
    cerr << "done" << endl;

    /// delete cube 2 triangles
    cerr << "deleting cube 2 triangles" << endl;
    cerr << "start pos = " << voxelRep->getStartPos(2) + voxelRep->getSideCode("Bk", {0, 0, 2}) << endl;
    tris->erase(tris->begin() + voxelRep->getStartPos(2) + voxelRep->getSideCode("Bk", {0, 0, 2}), tris->begin() + voxelRep->getStartPos(2) + voxelRep->getSideCode("Bk", {0, 0, 2}) + 2);
    voxelRep->addRemovedFaceCode(2, {0, 0, 2});
    cerr << "updating cube 2" << endl;
    voxelRep->update(2, {0, 0, 2});
    cerr << "done" << endl;

    ///    merge cube 0 and 3
    testLoadedModel->mergeMesh(testLoadedModel4, true);

    /// delete cube 0 triangles
    cerr << "deleting cube 0 triangles" << endl;
    cerr << "start pos = " << voxelRep->getStartPos(0) + voxelRep->getSideCode("T", {0, 0, 0}) << endl;
    tris->erase(tris->begin() + voxelRep->getStartPos(0) + voxelRep->getSideCode("T", {0, 0, 0}), tris->begin() + voxelRep->getStartPos(0) + voxelRep->getSideCode("T", {0, 0, 0}) + 2);
    voxelRep->addRemovedFaceCode(0, {0, 0, 0});
    cerr << "updating cube 0" << endl;
    voxelRep->update(2, {0, 0, 0});
    cerr << "done" << endl;

    /// delete cube 3 triangles
    cerr << "deleting cube 3 triangles" << endl;
    cerr << "start pos = " << voxelRep->getStartPos(3) + voxelRep->getSideCode("Bm", {0, 2, 0}) << endl;
    tris->erase(tris->begin() + voxelRep->getStartPos(3) + voxelRep->getSideCode("Bm", {0, 2, 0}), tris->begin() + voxelRep->getStartPos(3) + voxelRep->getSideCode("Bm", {0, 2, 0}) + 2);
    voxelRep->addRemovedFaceCode(3, {0, 2, 0});
    cerr << "updating cube 3" << endl;
    voxelRep->update(2, {0, 2, 0});
    cerr << "done" << endl;

    testLoadedModel->boxFit(10.0f);
    finalMesh->shape = testLoadedModel;

    csgroot = finalMesh;
    rep = SceneRep::TREE;
}

void Scene::newTestScene(string filename)
{
    /// load grid
    readGridVV(filename, 32);

    /// global voxel rep object
    VoxelRep * voxelRep = new VoxelRep ();

    /// test merge mesh
    ShapeNode * finalMesh = new ShapeNode();
    Mesh * accCube = new Mesh();
    accCube->readSTL("meshes/triangle/10mm_test_cube.stl");

    int entryCount = 0, length = 32;
    cgp::Point pointToAdd = cgp::Point(0.0f, 0.0f, 0.0f);

    /// go through voxel grid and render a cube for each voxel value == 1
    for (int z = 0; z < length; ++z) {
        for (int y = 0; y < length; ++y) {
            for (int x = 0; x < length; ++x) {
                if (vox.get(x, y, z) == 1) { /// voxel found, so render cube

                    voxelRep->add(0, {x, y, z});
                    pointToAdd = cgp::Point(x*10, y*10, z*10);
                    if (entryCount == 0) {
                        /// translate vertices
                        vector<cgp::Point> * vts = accCube->getVerts();

                        for (std::vector<cgp::Point>::iterator i = vts->begin() ; i != vts->end(); ++i) {
                            (*i).add(pointToAdd);
                        }
                    } else {
                        Mesh * newCube = new Mesh();
                        newCube->readSTL("meshes/triangle/10mm_test_cube.stl");

                        /// translate vertices of new cube
                        vector<cgp::Point> * vts = newCube->getVerts();

                        for (std::vector<cgp::Point>::iterator i = vts->begin() ; i != vts->end(); ++i) {
                            (*i).add(pointToAdd);
                        }

                        /// change triangle indices in new cube
                        vector<Triangle> * trs = newCube->getCubeTriangles();
                        int sizeToAdd = accCube->getNumVerts();
                        Triangle vecToAdd;
                        for (std::vector<Triangle>::iterator i = trs->begin() ; i != trs->end(); ++i) {
                            (*i).v[0] += sizeToAdd;
                            (*i).v[1] += sizeToAdd;
                            (*i).v[2] += sizeToAdd;
                        }

                        /// merge new cube to accCube
                        accCube->mergeMesh(newCube);


                        ///< delete surrounding cubes' triangles

                        vector<Triangle> * tris = accCube->getCubeTriangles();
                        int eCount;

                        /// left
                        if (x != 0  && (vox.get(x-1, y, z) == 1))
                        {
                            /// existing cube
                            eCount = voxelRep->getEntryCount({x-1, y, z});
                            tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("R", {x-1, y, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("R", {x-1, y, z}) + 2);
                            voxelRep->addRemovedFaceCode(eCount, "R");
                            voxelRep->update(2, {x-1, y, z});

                            /// newly added cube
                            eCount = voxelRep->getEntryCount({x, y, z});
                            tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("L", {x, y, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("L", {x, y, z}) + 2);
                            voxelRep->addRemovedFaceCode(eCount, "L");
                            voxelRep->update(2, {x, y, z});
                        }

                        /// back
                        if (z != 0  && (vox.get(x, y, z-1) == 1))
                        {
                            /// existing cube
                            eCount = voxelRep->getEntryCount({x, y, z-1});
                            cerr << "x, y, z-1 : " << x << ", " << y << ", " << z-1 << endl;
                            tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("F", {x, y, z-1}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("F", {x, y, z-1}) + 2);
                            voxelRep->addRemovedFaceCode(eCount, "F");
                            voxelRep->update(2, {x, y, z-1});

                            /// newly added cube
                            eCount = voxelRep->getEntryCount({x, y, z});
                            tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("Bk", {x, y, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("Bk", {x, y, z}) + 2);
                            voxelRep->addRemovedFaceCode(eCount, "Bk");
                            voxelRep->update(2, {x, y, z});
                        }


                        /// bottom
                        if (y != 0 && (vox.get(x, y-1, z) == 1))
                        {
                            /// existing cube
                            eCount = voxelRep->getEntryCount({x, y-1, z});
                            tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("T", {x, y-1, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("T", {x, y-1, z}) + 2);
                            voxelRep->addRemovedFaceCode(eCount, "T");
                            voxelRep->update(2, {x, y-1, z});

                            /// newly added cube
                            eCount = voxelRep->getEntryCount({x, y, z});
                            tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("Bm", {x, y, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("Bm", {x, y, z}) + 2);
                            voxelRep->addRemovedFaceCode(eCount, "Bm");
                            voxelRep->update(2, {x, y, z});
                        }
                    }
                    entryCount++;
                }
            }
        }
    }

    accCube->mergeAllVerts();

    for (vector<Triangle>::iterator triangle = accCube->getCubeTriangles()->begin(); triangle != accCube->getCubeTriangles()->end(); ++triangle)
    {
        triangle->printVec();
    }

    accCube->boxFit(10.0f);
    finalMesh->shape = accCube;

    csgroot = finalMesh;
    rep = SceneRep::TREE;
}

void Scene::anotherVoxelScene(string filename)
{
    /// load grid
    readGridVV(filename, 32);

    /// global voxel rep object
    VoxelRep * voxelRep = new VoxelRep ();

    /// test merge mesh
    ShapeNode * finalMesh = new ShapeNode();
    Mesh * accCube = new Mesh();
    accCube->readSTL("meshes/triangle/10mm_test_cube.stl");

    int entryCount = 0, length = 5;
    cgp::Point pointToAdd = cgp::Point(0.0f, 0.0f, 0.0f);

    /// go through voxel grid and render a cube for each voxel value == 1
    for (int z = 0; z < length; ++z) {
        for (int y = 0; y < length; ++y) {
            for (int x = 0; x < length; ++x) {
                if (/*vox.get(z, y,x) == 1 x%2==0 && y%2==08*/ true ) { // voxel found, so render cube

                    voxelRep->add(0, {x, y, z});
                    pointToAdd = cgp::Point(x*10, y*10, z*10);
                    if (entryCount == 0) {
                        /// translate vertices
                        vector<cgp::Point> * vts = accCube->getVerts();

                        for (std::vector<cgp::Point>::iterator i = vts->begin() ; i != vts->end(); ++i) {
                            (*i).add(pointToAdd);
                        }
                    } else {
                        Mesh * newCube = new Mesh();
                        newCube->readSTL("meshes/triangle/10mm_test_cube.stl");

                        /// translate vertices of new cube
                        vector<cgp::Point> * vts = newCube->getVerts();

                        for (std::vector<cgp::Point>::iterator i = vts->begin() ; i != vts->end(); ++i) {
                            (*i).add(pointToAdd);
                        }

                        /// change triangle indices in new cube
                        vector<Triangle> * trs = newCube->getCubeTriangles();
                        int sizeToAdd = accCube->getNumVerts();
                        Triangle vecToAdd;
                        for (std::vector<Triangle>::iterator i = trs->begin() ; i != trs->end(); ++i) {
                            (*i).v[0] += sizeToAdd;
                            (*i).v[1] += sizeToAdd;
                            (*i).v[2] += sizeToAdd;
                        }

                        /// merge new cube to accCube
                        accCube->mergeMesh(newCube);
                    }
                    entryCount++;
                }
            }
        }
    }

    accCube->mergeAllVerts();
    accCube->boxFit(10.0f);
    finalMesh->shape = accCube;

    csgroot = finalMesh;
    rep = SceneRep::TREE;
}

void Scene::voxelMeshScene(string filename)
{
    /// load grid
    readGridVV(filename, 32);

    /// global voxel rep object
    VoxelRep * voxelRep = new VoxelRep ();

    /// test merge mesh

    accCube->readSTL("meshes/triangle/10mm_test_cube.stl");

    int entryCount = 0, length = 32;
    cgp::Point pointToAdd = cgp::Point(0.0f, 0.0f, 0.0f);

    /// go through voxel grid and render a cube for each voxel value == 1
    for (int z = 0; z < length; ++z) {
        for (int y = 0; y < length; ++y) {
            for (int x = 0; x < length; ++x) {
                if (vox.get(x, y, z) == 1) { /// voxel found, so render cube

                    voxelRep->add(0, {x, y, z});
                    pointToAdd = cgp::Point(x*10, y*10, z*10);
                    if (entryCount == 0) {
                        /// translate vertices
                        vector<cgp::Point> * vts = accCube->getVerts();

                        for (std::vector<cgp::Point>::iterator i = vts->begin() ; i != vts->end(); ++i) {
                            (*i).add(pointToAdd);
                        }
                    } else {
                        Mesh * newCube = new Mesh();
                        newCube->readSTL("meshes/triangle/10mm_test_cube.stl");

                        /// translate vertices of new cube
                        vector<cgp::Point> * vts = newCube->getVerts();

                        for (std::vector<cgp::Point>::iterator i = vts->begin() ; i != vts->end(); ++i) {
                            (*i).add(pointToAdd);
                        }

                        /// change triangle indices in new cube
                        vector<Triangle> * trs = newCube->getCubeTriangles();
                        int sizeToAdd = accCube->getNumVerts();
                        Triangle vecToAdd;
                        for (std::vector<Triangle>::iterator i = trs->begin() ; i != trs->end(); ++i) {
                            (*i).v[0] += sizeToAdd;
                            (*i).v[1] += sizeToAdd;
                            (*i).v[2] += sizeToAdd;
                        }

                        /// merge new cube to accCube
                        accCube->mergeMesh(newCube);

                        ///< delete surrounding cubes' triangles

                        vector<Triangle> * tris = accCube->getCubeTriangles();
                        int eCount;

                        // left
                        if (x != 0  && (vox.get(x-1, y, z) == 1))
                        {
                            // existing cube
                            eCount = voxelRep->getEntryCount({x-1, y, z});
                            tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("R", {x-1, y, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("R", {x-1, y, z}) + 2);
                            voxelRep->addRemovedFaceCode(eCount, "R");
                            voxelRep->update(2, {x-1, y, z});

                            // newly added cube
                            eCount = voxelRep->getEntryCount({x, y, z});
                            tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("L", {x, y, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("L", {x, y, z}) + 2);
                            voxelRep->addRemovedFaceCode(eCount, "L");
                            voxelRep->update(2, {x, y, z});
                        }

                        // back
                        if (z != 0  && (vox.get(x, y, z-1) == 1))
                        {
                            // existing cube
                            eCount = voxelRep->getEntryCount({x, y, z-1});
                            cerr << "x, y, z-1 : " << x << ", " << y << ", " << z-1 << endl;
                            tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("F", {x, y, z-1}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("F", {x, y, z-1}) + 2);
                            voxelRep->addRemovedFaceCode(eCount, "F");
                            voxelRep->update(2, {x, y, z-1});

                            // newly added cube
                            eCount = voxelRep->getEntryCount({x, y, z});
                            tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("Bk", {x, y, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("Bk", {x, y, z}) + 2);
                            voxelRep->addRemovedFaceCode(eCount, "Bk");
                            voxelRep->update(2, {x, y, z});
                        }


                        // bottom
                        if (y != 0 && (vox.get(x, y-1, z) == 1))
                        {
                            // existing cube
                            eCount = voxelRep->getEntryCount({x, y-1, z});
                            tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("T", {x, y-1, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("T", {x, y-1, z}) + 2);
                            voxelRep->addRemovedFaceCode(eCount, "T");
                            voxelRep->update(2, {x, y-1, z});

                            // newly added cube
                            eCount = voxelRep->getEntryCount({x, y, z});
                            tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("Bm", {x, y, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("Bm", {x, y, z}) + 2);
                            voxelRep->addRemovedFaceCode(eCount, "Bm");
                            voxelRep->update(2, {x, y, z});
                        }
                    }
                    entryCount++;
                }
            }
        }
    }

    accCube->mergeAllVerts();

//    cerr << "tris after merge:" << endl;
//    for (vector<Triangle>::iterator triangle = accCube->getCubeTriangles()->begin(); triangle != accCube->getCubeTriangles()->end(); ++triangle)
//    {
//        triangle->printVec();
//    }

    accCube->boxFit(10.0f);
    finalMesh->shape = accCube;

    csgroot = finalMesh;
    rep = SceneRep::TREE;
}

void Scene::testShrinkScene(bool normalSize)
{
    /*
     * if normalSize then render normal sample mesh
     * else render shrunken mesh
    */

    if (normalSize)
    {
        accCube->readSTL("meshes/triangle/10mm_test_cube.stl");

        /// second cube
        Mesh * newCube = new Mesh();
        newCube->readSTL("meshes/triangle/10mm_test_cube.stl");
        cgp::Point pointToAdd = cgp::Point(10, 0, 0);

        ///< translate vertices of new cube
        vector<cgp::Point> * vts = newCube->getVerts();

        for (std::vector<cgp::Point>::iterator i = vts->begin() ; i != vts->end(); ++i) {
            (*i).add(pointToAdd);
        }

        ///< change triangle indices in new cube
        vector<Triangle> * trs = newCube->getCubeTriangles();
        int sizeToAdd = accCube->getNumVerts();
        for (std::vector<Triangle>::iterator i = trs->begin() ; i != trs->end(); ++i) {
            (*i).v[0] += sizeToAdd;
            (*i).v[1] += sizeToAdd;
            (*i).v[2] += sizeToAdd;
        }

        ///< merge second cube to accCube
        accCube->mergeMesh(newCube, true);

        ///< global voxel rep object
        VoxelRep * voxelRep = new VoxelRep ();
        voxelRep->add(0, {0, 0, 0});
        voxelRep->add(0, {1, 0, 0});

        ///< delete surrounding cubes' triangles

        vector<Triangle> * tris = accCube->getCubeTriangles();
        int eCount;

        /// left of cube 2
        if (true)
        {
            /// existing cube
            eCount = voxelRep->getEntryCount({0, 0, 0});
            tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("R", {0, 0, 0}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("R", {0, 0, 0}) + 2);
            voxelRep->addRemovedFaceCode(eCount, "R");
            voxelRep->update(2, {0, 0, 0});

            /// newly added cube
            eCount = voxelRep->getEntryCount({1, 0, 0});
            tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("L", {1, 0, 0}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("L", {1, 0, 0}) + 2);
            voxelRep->addRemovedFaceCode(eCount, "L");
            voxelRep->update(2, {1, 0, 0});
        }

//        cerr << "triangle size after face removal: " << tris->size() << endl;
//        for (vector<Triangle>::iterator iter = tris->begin(); iter != tris->end(); iter++) {
//            (*iter).printVec();
//        }

        accCube->boxFit(10.0f);
    }
    else
    {
        vector<cgp::Point> * vts = accCube->getVerts();
        vector<Triangle> * trs = accCube->getCubeTriangles();
        vector<cgp::Vector> netVectors;

        int index = 0;

        cerr << "##### Shrink Test #####" << endl;

        for (vector<cgp::Point>::iterator vertex = vts->begin(); vertex != vts->end(); ++vertex) {
            vector<cgp::Vector> normals;
            vector<Triangle> adjacentTris;

            ///< find all adjacent faces
            for (vector<Triangle>::iterator triangle = trs->begin(); triangle != trs->end(); ++triangle) {
                if (triangle->vertexFound(index))
                    adjacentTris.push_back( *triangle );
            }

            //cerr << "index " << index << " | triangles found: " << adjacentTris.size() << endl;
            //cerr << "normals size: " << normals.size() << endl;

            ///< calculate normals
            for (vector<Triangle>::iterator triangle = adjacentTris.begin(); triangle != adjacentTris.end(); ++triangle) {
                triangle->deriveNormal(normals, *vts);
            }

            ///< average normals
            cgp::Vector avgNormal = normals[0];
            int n = normals.size();
            for (int i = 1; i < n; ++i) {
                avgNormal.i += normals[i].i;
                avgNormal.j += normals[i].j;
                avgNormal.k += normals[i].k;
            }

//            cerr << "index " << index << " | normals found: " << normals.size() << endl;
//            if (true)
//                for (int i = 0; i < normals.size(); ++i) {
//                    cerr << normals[i].printVec() << endl;
//                }

            avgNormal = cgp::Vector(avgNormal.i/n, avgNormal.j/n, avgNormal.k/n);
            //cerr << "avg normal: " << avgNormal.i << " " << avgNormal.j << " " << avgNormal.k << endl;

            /// normalize
            avgNormal.normalize();

            netVectors.push_back(avgNormal);

            index++;
        }

        /// snap net vectors
        float shrinkBy = 0.5f;
        vector<cgp::Point>::iterator vertex = vts->begin();
        for (int i = 0; i < vts->size(); ++i) {
            netVectors[i].snapVector();
            netVectors[i].normalize();
            netVectors[i].specificSnap();
            cgp::Point translateBy = cgp::Point(shrinkBy*netVectors[i].i, shrinkBy*netVectors[i].j, shrinkBy*netVectors[i].k);
            vertex->add(translateBy);
            ++vertex;
        }
    }

    finalMesh->shape = accCube;

    csgroot = finalMesh;
    rep = SceneRep::TREE;
}

void Scene::pieceA1Scene(bool shrunk)
{
    /// global voxel rep object
    VoxelRep * voxelRep = new VoxelRep ();

    int entryCount = 0, length = 32;
    cgp::Point pointToAdd = cgp::Point(0.0f, 0.0f, 0.0f);
    int count = 0;
    bool renderX = true, renderZ = true;

    if (!shrunk)
    {
        accCube->readSTL("meshes/triangle/10mm_test_cube.stl");

        // go through voxel grid and render a cube for each voxel value == 1
        for (int z = 0; z < length; ++z) {
            for (int y = 0; y < length; ++y) {
                for (int x = 0; x < length; ++x) {
                    if (((x == 15 || x == 17) && (z == 14 || z == 18)) || x == 14 || x == 16 || x == 18) renderX = true; else renderX = false;
                    if (((z == 15 || z == 17) && (x == 14 || x == 18)) || z == 14 || z == 16 || z == 18) renderZ = true; else renderZ = false;

                    if (((y >= 14 && y<16) && (x >= 14 && x <= 18) && (z >= 14 && z <= 18)) || (renderX && renderZ && (y >= 14 && y <= 18)))
                    {
                        vox.set(x, y, z, true);
                        count++;
                        voxelRep->add(0, {x, y, z});
                        pointToAdd = cgp::Point(x*10, y*10, z*10);
                        if (entryCount == 0) {
                            /// translate vertices
                            vector<cgp::Point> * vts = accCube->getVerts();

                            for (std::vector<cgp::Point>::iterator i = vts->begin() ; i != vts->end(); ++i) {
                                (*i).add(pointToAdd);
                            }
                        } else {
                            Mesh * newCube = new Mesh();
                            newCube->readSTL("meshes/triangle/10mm_test_cube.stl");

                            /// translate vertices of new cube
                            vector<cgp::Point> * vts = newCube->getVerts();

                            for (std::vector<cgp::Point>::iterator i = vts->begin() ; i != vts->end(); ++i) {
                                (*i).add(pointToAdd);
                            }

                            /// change triangle indices in new cube
                            vector<Triangle> * trs = newCube->getCubeTriangles();
                            int sizeToAdd = accCube->getNumVerts();
                            Triangle vecToAdd;
                            for (std::vector<Triangle>::iterator i = trs->begin() ; i != trs->end(); ++i) {
                                (*i).v[0] += sizeToAdd;
                                (*i).v[1] += sizeToAdd;
                                (*i).v[2] += sizeToAdd;
                            }

                            /// merge new cube to accCube
                            accCube->mergeMesh(newCube);

                            ///< delete surrounding cubes' triangles

                            vector<Triangle> * tris = accCube->getCubeTriangles();
                            int eCount;

                            /// left
                            if (x != 0  && (vox.get(x-1, y, z) == 1))
                            {
                                /// existing cube
                                eCount = voxelRep->getEntryCount({x-1, y, z});
                                tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("R", {x-1, y, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("R", {x-1, y, z}) + 2);
                                voxelRep->addRemovedFaceCode(eCount, "R");
                                voxelRep->update(2, {x-1, y, z});

                                /// newly added cube
                                eCount = voxelRep->getEntryCount({x, y, z});
                                tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("L", {x, y, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("L", {x, y, z}) + 2);
                                voxelRep->addRemovedFaceCode(eCount, "L");
                                voxelRep->update(2, {x, y, z});
                            }

                            /// back
                            if (z != 0  && (vox.get(x, y, z-1) == 1))
                            {
                                /// existing cube
                                eCount = voxelRep->getEntryCount({x, y, z-1});
                                cerr << "x, y, z-1 : " << x << ", " << y << ", " << z-1 << endl;
                                tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("F", {x, y, z-1}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("F", {x, y, z-1}) + 2);
                                voxelRep->addRemovedFaceCode(eCount, "F");
                                voxelRep->update(2, {x, y, z-1});

                                /// newly added cube
                                eCount = voxelRep->getEntryCount({x, y, z});
                                tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("Bk", {x, y, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("Bk", {x, y, z}) + 2);
                                voxelRep->addRemovedFaceCode(eCount, "Bk");
                                voxelRep->update(2, {x, y, z});
                            }


                            /// bottom
                            if (y != 0 && (vox.get(x, y-1, z) == 1))
                            {
                                /// existing cube
                                eCount = voxelRep->getEntryCount({x, y-1, z});
                                tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("T", {x, y-1, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("T", {x, y-1, z}) + 2);
                                voxelRep->addRemovedFaceCode(eCount, "T");
                                voxelRep->update(2, {x, y-1, z});

                                /// newly added cube
                                eCount = voxelRep->getEntryCount({x, y, z});
                                tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("Bm", {x, y, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("Bm", {x, y, z}) + 2);
                                voxelRep->addRemovedFaceCode(eCount, "Bm");
                                voxelRep->update(2, {x, y, z});
                            }
                        }
                        entryCount++;
                    }
                }
            }
        }

        accCube->mergeAllVerts();

        //        cerr << "tris after merge:" << endl;
        //        for (vector<Triangle>::iterator triangle = accCube->getCubeTriangles()->begin(); triangle != accCube->getCubeTriangles()->end(); ++triangle)
        //        {
        //            triangle->printVec();
        //        }

        accCube->boxFit(10.0f);
    }
    else
    {
        vector<cgp::Point> * vts = accCube->getVerts();
        vector<Triangle> * trs = accCube->getCubeTriangles();
        vector<cgp::Vector> netVectors;

        int index = 0;

        cerr << "##### Shrink Test #####" << endl;

        for (vector<cgp::Point>::iterator vertex = vts->begin(); vertex != vts->end(); ++vertex) {
            vector<cgp::Vector> normals;
            vector<Triangle> adjacentTris;

            ///< find all adjacent faces
            for (vector<Triangle>::iterator triangle = trs->begin(); triangle != trs->end(); ++triangle) {
                if (triangle->vertexFound(index))
                    adjacentTris.push_back( *triangle );
            }

            ///< calculate normals
            for (vector<Triangle>::iterator triangle = adjacentTris.begin(); triangle != adjacentTris.end(); ++triangle) {
                triangle->deriveNormal(normals, *vts);
            }

            ///< average normals
            cgp::Vector avgNormal = normals[0];
            int n = normals.size();
            for (int i = 1; i < n; ++i) {
                avgNormal.i += normals[i].i;
                avgNormal.j += normals[i].j;
                avgNormal.k += normals[i].k;
            }

//            cerr << "index " << index << " | normals found: " << normals.size() << endl;
//            if (true)
//                for (int i = 0; i < normals.size(); ++i) {
//                    cerr << normals[i].printVec() << endl;
//                }

            avgNormal = cgp::Vector(avgNormal.i/n, avgNormal.j/n, avgNormal.k/n);

            /// normalize
            avgNormal.normalize();

            netVectors.push_back(avgNormal);

            index++;
        }

        /// snap net vectors
        float shrinkBy = 0.5f;
        vector<cgp::Point>::iterator vertex = vts->begin();
        for (int i = 0; i < vts->size(); ++i) {
            netVectors[i].snapVector();
            netVectors[i].normalize();
            netVectors[i].specificSnap();
            cgp::Point translateBy = cgp::Point(shrinkBy*netVectors[i].i, shrinkBy*netVectors[i].j, shrinkBy*netVectors[i].k);
            vertex->add(translateBy);
            ++vertex;
        }
    }

    finalMesh->shape = accCube;

    csgroot = finalMesh;
    rep = SceneRep::TREE;
}

void Scene::pieceA2Scene(bool shrunk)
{
    /// global voxel rep object
    VoxelRep * voxelRep = new VoxelRep ();

    int entryCount = 0, length = 32;
    cgp::Point pointToAdd = cgp::Point(0.0f, 0.0f, 0.0f);
    int count = 0;
    bool renderX = true, renderZ = true;

    if (!shrunk)
    {
        accCube->readSTL("meshes/triangle/10mm_test_cube.stl");

        /// go through voxel grid and render a cube for each voxel value == 1
        for (int z = 0; z < length; ++z) {
            for (int y = 0; y < length; ++y) {
                for (int x = 0; x < length; ++x) {
                    if (x == 15 || x == 17 || (x == 16 && z != 16)) renderX = true; else renderX = false;
                    if (z == 15 || z == 17 || (z == 16 && x != 16)) renderZ = true; else renderZ = false;

                    if (((y >= 14 && y<16) && (x >= 14 && x <= 18) && (z >= 14 && z <= 18)) || ((y >= 16 && y <= 18) && renderX && renderZ)) {
                        vox.set(x, y, z, true);
                        count++;
                        voxelRep->add(0, {x, y, z});
                        pointToAdd = cgp::Point(x*10, y*10, z*10);
                        if (entryCount == 0) {
                            /// translate vertices
                            vector<cgp::Point> * vts = accCube->getVerts();

                            for (std::vector<cgp::Point>::iterator i = vts->begin() ; i != vts->end(); ++i) {
                                (*i).add(pointToAdd);
                            }
                        } else {
                            Mesh * newCube = new Mesh();
                            newCube->readSTL("meshes/triangle/10mm_test_cube.stl");

                            /// translate vertices of new cube
                            vector<cgp::Point> * vts = newCube->getVerts();

                            for (std::vector<cgp::Point>::iterator i = vts->begin() ; i != vts->end(); ++i) {
                                (*i).add(pointToAdd);
                            }

                            /// change triangle indices in new cube
                            vector<Triangle> * trs = newCube->getCubeTriangles();
                            int sizeToAdd = accCube->getNumVerts();
                            Triangle vecToAdd;
                            for (std::vector<Triangle>::iterator i = trs->begin() ; i != trs->end(); ++i) {
                                (*i).v[0] += sizeToAdd;
                                (*i).v[1] += sizeToAdd;
                                (*i).v[2] += sizeToAdd;
                            }

                            /// merge new cube to accCube
                            accCube->mergeMesh(newCube);


                            ///< delete surrounding cubes' triangles

                            vector<Triangle> * tris = accCube->getCubeTriangles();
                            int eCount;

                            /// left
                            if (x != 0  && (vox.get(x-1, y, z) == 1))
                            {
                                /// existing cube
                                eCount = voxelRep->getEntryCount({x-1, y, z});
                                tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("R", {x-1, y, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("R", {x-1, y, z}) + 2);
                                voxelRep->addRemovedFaceCode(eCount, "R");
                                voxelRep->update(2, {x-1, y, z});

                                /// newly added cube
                                eCount = voxelRep->getEntryCount({x, y, z});
                                tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("L", {x, y, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("L", {x, y, z}) + 2);
                                voxelRep->addRemovedFaceCode(eCount, "L");
                                voxelRep->update(2, {x, y, z});
                            }

                            /// back
                            if (z != 0  && (vox.get(x, y, z-1) == 1))
                            {
                                /// existing cube
                                eCount = voxelRep->getEntryCount({x, y, z-1});
                                cerr << "x, y, z-1 : " << x << ", " << y << ", " << z-1 << endl;
                                tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("F", {x, y, z-1}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("F", {x, y, z-1}) + 2);
                                voxelRep->addRemovedFaceCode(eCount, "F");
                                voxelRep->update(2, {x, y, z-1});

                                /// newly added cube
                                eCount = voxelRep->getEntryCount({x, y, z});
                                tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("Bk", {x, y, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("Bk", {x, y, z}) + 2);
                                voxelRep->addRemovedFaceCode(eCount, "Bk");
                                voxelRep->update(2, {x, y, z});
                            }


                            /// bottom
                            if (y != 0 && (vox.get(x, y-1, z) == 1))
                            {
                                /// existing cube
                                eCount = voxelRep->getEntryCount({x, y-1, z});
                                tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("T", {x, y-1, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("T", {x, y-1, z}) + 2);
                                voxelRep->addRemovedFaceCode(eCount, "T");
                                voxelRep->update(2, {x, y-1, z});

                                /// newly added cube
                                eCount = voxelRep->getEntryCount({x, y, z});
                                tris->erase(tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("Bm", {x, y, z}), tris->begin() + voxelRep->getStartPos(eCount) + voxelRep->getSideCode("Bm", {x, y, z}) + 2);
                                voxelRep->addRemovedFaceCode(eCount, "Bm");
                                voxelRep->update(2, {x, y, z});
                            }
                        }
                        entryCount++;
                    }
                }
            }
        }

        accCube->mergeAllVerts();

//        cerr << "tris after merge:" << endl;
//        for (vector<Triangle>::iterator triangle = accCube->getCubeTriangles()->begin(); triangle != accCube->getCubeTriangles()->end(); ++triangle)
//        {
//            triangle->printVec();
//        }

        accCube->boxFit(10.0f);
    }
    else
    {
        vector<cgp::Point> * vts = accCube->getVerts();
        vector<Triangle> * trs = accCube->getCubeTriangles();
        vector<cgp::Vector> netVectors;

        int index = 0;

        cerr << "##### Shrink Test #####" << endl;

        for (vector<cgp::Point>::iterator vertex = vts->begin(); vertex != vts->end(); ++vertex) {
            vector<cgp::Vector> normals;
            vector<Triangle> adjacentTris;

            ///< find all adjacent faces
            for (vector<Triangle>::iterator triangle = trs->begin(); triangle != trs->end(); ++triangle) {
                if (triangle->vertexFound(index))
                    adjacentTris.push_back( *triangle );
            }

            ///< calculate normals
            for (vector<Triangle>::iterator triangle = adjacentTris.begin(); triangle != adjacentTris.end(); ++triangle) {
                triangle->deriveNormal(normals, *vts);
            }

            ///< average normals
            cgp::Vector avgNormal = normals[0];
            int n = normals.size();
            for (int i = 1; i < n; ++i) {
                avgNormal.i += normals[i].i;
                avgNormal.j += normals[i].j;
                avgNormal.k += normals[i].k;
            }

//            cerr << "index " << index << " | normals found: " << normals.size() << endl;
//            if (true)
//                for (int i = 0; i < normals.size(); ++i) {
//                    cerr << normals[i].printVec() << endl;
//                }

            avgNormal = cgp::Vector(avgNormal.i/n, avgNormal.j/n, avgNormal.k/n);

            ///< normalise
            avgNormal.normalize();

            netVectors.push_back(avgNormal);

            index++;
        }

        ///< snap net vectors
        float shrinkBy = 0.5f;
        vector<cgp::Point>::iterator vertex = vts->begin();
        for (int i = 0; i < vts->size(); ++i) {
            netVectors[i].snapVector();
            netVectors[i].normalize();
            netVectors[i].specificSnap();
            cgp::Point translateBy = cgp::Point(shrinkBy*netVectors[i].i, shrinkBy*netVectors[i].j, shrinkBy*netVectors[i].k);
            vertex->add(translateBy);
            ++vertex;
        }
    }

    finalMesh->shape = accCube;

    csgroot = finalMesh;
    rep = SceneRep::TREE;
}
