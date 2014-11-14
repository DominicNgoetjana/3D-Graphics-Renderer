//
// Scene
//

#include "scene.h"
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

GLfloat defaultCol[] = {0.243f, 0.176f, 0.75f, 1.0f};

bool Scene::findVert(vpPoint pnt, int &idx)
{
    bool found = false;
    int i = 0;

    idx = -1;
    // linear search of vertex list
    while(!found && i < (int) verts.size())
    {
        if(verts[i] == pnt)
        {
            found = true;
            idx = i;
        }
        i++;
    }
    return found;
}

bool Scene::findEdge(Edge edge, int &idx)
{
    bool found = false;
    int i = 0;

    idx = -1;
    // linear search of edge list
    while(!found && i < (int) edges.size())
    {
        if((edges[i].v[0] == edge.v[0] && edges[i].v[1] == edge.v[1]) ||
           (edges[i].v[0] == edge.v[1] && edges[i].v[1] == edge.v[0])) // edge can match in either direction
        {
            found = true;
            idx = i;
        }
        i++;
    }
    return found;
}

vpPoint Scene::deform(vpPoint pnt, AxialDef axdef, float axval)
{
    vpPoint defpnt;
    glm::mat4x4 idt, tfm;
    glm::vec4 gpnt;
    float yrot;

    switch(axdef)
    {
        case AxialDef::IDENTITY:
            defpnt = pnt;
            break;
        case AxialDef::TWIST:
            idt = glm::mat4(1.0f);
            yrot = pnt.y * axval;
            tfm = glm::rotate(idt, yrot, glm::vec3(0.0f, 1.0f, 0.0f));
            gpnt = tfm * glm::vec4(pnt.x, pnt.y, pnt.z, 1.0f);
            defpnt = vpPoint(gpnt.x, gpnt.y, gpnt.z);
            break;
        default:
            defpnt = pnt;
            break;
    }
    return defpnt;
}

bool Scene::genTessRender(View * view, ShapeDrawData &sdd)
{
    int i;
    glm::mat4 tfm, idt;
    glm::vec3 trs, rot;
    Vector edgevec, zerovec, axisvec, zaxis;
    float edgelen, aval;

    geom.clear();
    geom.setColour(col);

    idt = glm::mat4(1.0f); // identity matrix
    zerovec = Vector(0.0f, 0.0f, 0.0f);

    // place a sphere at every vertex
    for(i = 0; i < (int) verts.size(); i++)
    {
        if(!deadverts[i])
        {
            trs = glm::vec3(verts[i].x, verts[i].y, verts[i].z);
            tfm = glm::translate(idt, trs);
            geom.genSphere(sphererad, 30, 30, tfm);
        }
    }

    // place a cylinder along every edge
    for(i = 0; i < (int) edges.size(); i++)
    {
        if(!deadedges[i])
        {
            edgevec.diff(verts[edges[i].v[0]], verts[edges[i].v[1]]);
            edgelen = edgevec.length();
            edgevec.normalize();

            // translate to starting vertex
            trs = glm::vec3(verts[edges[i].v[0]].x, verts[edges[i].v[0]].y, verts[edges[i].v[0]].z);
            tfm = glm::translate(idt, trs);

            // determine rotation axis, normal to plane of z axis and edge vector
            zaxis = Vector(0.0f, 0.0f, 1.0f);
            axisvec.cross(zaxis, edgevec);

            // check for degenerate cases
            if(axisvec == zerovec)
            {
                if(!(edgevec == zaxis)) // diametrically opposite
                {
                    aval = 180.0f;
                    rot = glm::vec3(0.0f, 1.0f, 0.0f);
                    tfm = glm::rotate(tfm, aval, rot);
                }
                // otherwise aligned so no rotation
            }
            else
            {
                axisvec.normalize();
                aval = acosf(zaxis.dot(edgevec)) * RAD2DEG;
                rot = glm::vec3(axisvec.i, axisvec.j, axisvec.k);
                tfm = glm::rotate(tfm, aval, rot);
            }

            // align to edge vector by rotation
            geom.genCylinder(cylrad, edgelen, 30, 5, tfm);
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

bool Scene::genVoxRender(View * view, ShapeDrawData &sdd)
{
    int x, y, z, xdim, ydim, zdim;
    glm::mat4 tfm, idt;
    glm::vec3 trs;
    vpPoint pnt;

    geom.clear();
    geom.setColour(col);

    if(voxactive)
    {
        idt = glm::mat4(1.0f); // identity matrix

        vox.getDim(xdim, ydim, zdim);

        // place a sphere at every filled voxel center
        for(x = 0; x < xdim; x++)
            for(y = 0; y < ydim; y++)
                for(z = 0; z < zdim; z++)
                {
                    if(vox.get(x, y, z))
                    {
                        pnt = vpPoint(voxsidelen * (float) x - voldiag.i/2.0f, voxsidelen * (float) y - voldiag.j/2.0f, voxsidelen * (float) z - voldiag.k/2.0f); // convert from voxel space to world coordinates
                        trs = glm::vec3(pnt.x, pnt.y, pnt.z);
                        tfm = glm::translate(idt, trs);
                        geom.genSphere(voxsidelen / 4.0f, 5, 5, tfm);
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
    col = defaultCol;
    sphererad = 0.1f;
    cylrad = 0.1f;
    voldiag = Vector(10.0f, 10.0f, 10.0f);
    tesslen = 1.0f;
    deftype = AxialDef::TWIST;
    defval = 10.0f;
    voxsidelen = 0.0f;
    voxactive = false;
    packCubes(vpPoint(-voldiag.i/2.0f, -voldiag.j/2.0f, -voldiag.k/2.0f), voldiag, tesslen);
}

Scene::~Scene()
{
    clear();
}

void Scene::clear()
{
    verts.clear();
    deadverts.clear();
    edges.clear();
    deadedges.clear();
    geom.clear();
    vox.clear();
}

bool Scene::genGeometry(View * view, ShapeDrawData &sdd)
{
    if(voxactive)
    {
        voxmesh.setColour(defaultCol);
        return voxmesh.genGeometry(view, sdd);
    }
    else
        return genTessRender(view, sdd);
}

bool Scene::addCube(vpPoint origin, float len)
{
    vpPoint pnt, defpnt;
    float x, y, z;
    bool snapon = false;
    int vidx[8], eidx, i = 0;
    Edge e;

    if(len <= 0.0f)
        cerr << "Error Scene::addCube: cube length must be positive." << endl;

    // create vertices of cube
    for(x = origin.x; x <= origin.x + len + pluszero; x+= len)
        for(y = origin.y; y <= origin.y + len + pluszero; y+= len)
            for(z = origin.z; z <= origin.z + len + pluszero; z += len)
            {
                pnt = vpPoint(x, y, z);
                defpnt = deform(pnt, deftype, defval);
                if(findVert(defpnt, vidx[i])) // point already exists
                {
                    snapon = true;
                }
                else // new point not yet in vertex list, so add
                {
                    vidx[i] = (int) verts.size();
                    verts.push_back(defpnt);
                    deadverts.push_back(false);
                }
                i++;
            }

    // create edges of cube using vertex indices
    e.v[0] = vidx[0]; e.v[1] = vidx[1]; // left face
    if(!findEdge(e, eidx))
    {
        edges.push_back(e); deadedges.push_back(false);
    }
    e.v[0] = vidx[0]; e.v[1] = vidx[2];
    if(!findEdge(e, eidx))
    {
        edges.push_back(e); deadedges.push_back(false);
    }
    e.v[0] = vidx[1]; e.v[1] = vidx[3];
    if(!findEdge(e, eidx))
    {
        edges.push_back(e); deadedges.push_back(false);
    }
    e.v[0] = vidx[2]; e.v[1] = vidx[3];
    if(!findEdge(e, eidx))
    {
        edges.push_back(e); deadedges.push_back(false);
    }
    e.v[0] = vidx[4]; e.v[1] = vidx[5]; // right face
    if(!findEdge(e, eidx))
    {
        edges.push_back(e); deadedges.push_back(false);
    }
    e.v[0] = vidx[4]; e.v[1] = vidx[6];
    if(!findEdge(e, eidx))
    {
        edges.push_back(e); deadedges.push_back(false);
    }
    e.v[0] = vidx[5]; e.v[1] = vidx[7];
    if(!findEdge(e, eidx))
    {
        edges.push_back(e); deadedges.push_back(false);
    }
    e.v[0] = vidx[6]; e.v[1] = vidx[7];
    if(!findEdge(e, eidx))
    {
        edges.push_back(e); deadedges.push_back(false);
    }
    e.v[0] = vidx[0]; e.v[1] = vidx[4]; // front face
    if(!findEdge(e, eidx))
    {
        edges.push_back(e); deadedges.push_back(false);
    }
    e.v[0] = vidx[2]; e.v[1] = vidx[6];
    if(!findEdge(e, eidx))
    {
        edges.push_back(e); deadedges.push_back(false);
    }
    e.v[0] = vidx[1]; e.v[1] = vidx[5]; // back face
    if(!findEdge(e, eidx))
    {
        edges.push_back(e); deadedges.push_back(false);
    }
    e.v[0] = vidx[3]; e.v[1] = vidx[7];
    if(!findEdge(e, eidx))
    {
        edges.push_back(e); deadedges.push_back(false);
    }

    return snapon;
}

void Scene::packCubes(vpPoint origin, Vector extent, float cubelen)
{
    int numx, numy, numz; // number of cubes in each direction
    int x, y, z;
    vpPoint o;
    bool snap;

    if(!(extent.i > 0.0f && extent.j > 0.0f && extent.k > 0.0f))
    {
        cerr << "Error Scene::packCubes: expect a positive extent vector as input." << endl;
    }

    clear();

    // TO DO - modify so that even with global axial deformation this still fits into the volume

    // number of cubes in dimension
    numx = (int) (extent.i / cubelen);
    numy = (int) (extent.j / cubelen);
    numz = (int) (extent.k / cubelen);
    for(x = 0; x < numx; x++)
        for(y = 0; y < numy; y++)
            for(z = 0; z < numz; z++)
            {
                o = vpPoint(origin.x + (float) x * cubelen, origin.y + (float) y * cubelen, origin.z + (float) z * cubelen);
                snap = addCube(o, cubelen);
                if(!(x == 0 && y==0 && z==0)) // subsequent cubes must snap to the existing structure
                    if(!snap)
                        cerr << "Error Scene::packCubes: cubes not correctly snapped together." << endl;
            }
}

void Scene::packCubesInMesh(Mesh * mesh)
{
    int numx, numy, numz; // number of cubes in each direction
    int x, y, z;
    vpPoint o, c, cdef;
    vpPoint origin = vpPoint(-voldiag.i/2.0f, -voldiag.j/2.0f, -voldiag.k/2.0f);
    Vector extent = voldiag;
    float cubelen = tesslen;
    float halflen = 0.5f * cubelen;
    // Timer time;

    if(!(extent.i > 0.0f && extent.j > 0.0f && extent.k > 0.0f))
    {
        cerr << "Error Scene::packCubes: expect a positive extent vector as input." << endl;
    }

    clear();

    // time.start();
    // number of cubes in dimension
    numx = (int) (extent.i / cubelen);
    numy = (int) (extent.j / cubelen);
    numz = (int) (extent.k / cubelen);
    for(x = 0; x < numx; x++)
        for(y = 0; y < numy; y++)
            for(z = 0; z < numz; z++)
            {
                o = vpPoint(origin.x + (float) x * cubelen, origin.y + (float) y * cubelen, origin.z + (float) z * cubelen);
                c = vpPoint(o.x + halflen, o.y + halflen, o.z + halflen); // cube center
                cdef = deform(c, deftype, defval);
                if(mesh->containedPoint(cdef)) // only include if center falls inside mesh
                    addCube(o, cubelen);
            }
    // time.stop();
    // cerr << "mesh gen time = " << time.peek() << endl;
}

void Scene::intersectMesh(Mesh * mesh)
{
    int v, e;
    int deadcount = 0;

    // reset previous intersection
    for(v = 0; v < (int) verts.size(); v++)
        deadverts[v] = false;

    for(e = 0; e < (int) edges.size(); e++)
        deadedges[e] = false;

    // use deadverts and deadedges to avoid actual deletion of verts and edges and the re-indexing that would be required
    for(v = 0; v < (int) verts.size(); v++)
    {
        if(!deadverts[v])
            if(!mesh->containedPoint(verts[v])) // outside mesh so delete
            {
                deadverts[v] = true;
                deadcount++;
            }
    }
    // cerr << "Culled " << deadcount << " of " << (int) verts.size() << " vertices" << endl;

    // mark all edges with a dead vertex as dead
    for(e = 0; e < (int) edges.size(); e++)
    {
        if(deadverts[edges[e].v[0]] || deadverts[edges[e].v[1]])
            deadedges[e] = true;
    }
}

void Scene::voxelise(float voxlen)
{
    vpPoint pnt;
    Vector halfdiag;
    int i, x, y, z, xdim, ydim, zdim, minx, miny, minz, maxx, maxy, maxz;
    Sphere sph;
    Cylinder cyl;
    BoundBox bbox;
    int voxcount;

    // calculate voxel volume dimensions based on voxlen
    xdim = ceil(voldiag.i / voxlen)+2; // needs a 1 voxel border to ensure a closed mesh
    ydim = ceil(voldiag.j / voxlen)+2;
    zdim = ceil(voldiag.k / voxlen)+2;
    voxsidelen = voxlen;
    voxactive = true;
    vox.setDim(xdim, ydim, zdim);
    halfdiag = voldiag;
    halfdiag.mult(0.5f);
    halfdiag = Vector((float) xdim * voxlen / 2.0f, (float) ydim * voxlen / 2.0f, (float) zdim * voxlen / 2.0f);

    cerr << "Voxel volume dimensions = " << xdim << " x " << ydim << " x " << zdim << endl;

    // iterate over spheres, only testing voxels that fall within their bounding boxes
    sph.r = sphererad;
    cerr << "total potential spheres = " << (int) verts.size() << endl;
    voxcount = 0;
    for(i = 0; i < (int) verts.size(); i++)
    {
        if(!deadverts[i])
        {
            sph.c = verts[i];
            bbox.reset();
            bbox.min = vpPoint(sph.c.x - sph.r, sph.c.y - sph.r, sph.c.z - sph.r);
            bbox.max = vpPoint(sph.c.x + sph.r, sph.c.y + sph.r, sph.c.z + sph.r);
            // convert coords to voxel space
            minx = (int) floor((bbox.min.x + halfdiag.i) / voxlen); minx = max(0, minx);
            miny = (int) floor((bbox.min.y + halfdiag.j) / voxlen); miny = max(0, miny);
            minz = (int) floor((bbox.min.z + halfdiag.k) / voxlen); minz = max(0, minz);
            maxx = (int) ceil((bbox.max.x + halfdiag.i) / voxlen); maxx = min(maxx, xdim-1);
            maxy = (int) ceil((bbox.max.y + halfdiag.j) / voxlen); maxy = min(maxy, ydim-1);
            maxz = (int) ceil((bbox.max.z + halfdiag.k) / voxlen); maxz = min(maxz, zdim-1);

            // iterate over voxels in bounding box, testing whether it is inside the sphere or not
            for(x = minx; x <= maxx; x++)
                for(y = miny; y <= maxy; y++)
                    for(z = minz; z < maxz; z++)
                    {
                        pnt = vpPoint(voxlen * (float) x - halfdiag.i, voxlen * (float) y - halfdiag.j, voxlen * (float) z - halfdiag.k); // convert from voxel space to world coordinates
                        if(sph.pointInSphere(pnt))
                        {
                            vox.set(x, y, z, true);
                            voxcount++;
                        }
                    }
        }
    }

    // iterate over cylinders, only testing voxels within bounds
    cerr << "vox spheres fin" << endl;
    cerr << "voxels set = " << voxcount << endl;

    cyl.r = cylrad;
    for(i = 0; i < (int) edges.size(); i++)
    {
        if(!deadedges[i])
        {
            cyl.s = verts[edges[i].v[0]];
            cyl.e = verts[edges[i].v[1]];

            bbox.reset();
            bbox.includePnt(vpPoint(cyl.s.x - cyl.r, cyl.s.y - cyl.r, cyl.s.z - cyl.r));
            bbox.includePnt(vpPoint(cyl.s.x + cyl.r, cyl.s.y + cyl.r, cyl.s.z + cyl.r));
            bbox.includePnt(vpPoint(cyl.e.x - cyl.r, cyl.e.y - cyl.r, cyl.e.z - cyl.r));
            bbox.includePnt(vpPoint(cyl.e.x + cyl.r, cyl.e.y + cyl.r, cyl.e.z + cyl.r));

            // convert coords to voxel space
            minx = (int) floor((bbox.min.x + halfdiag.i) / voxlen); minx = max(0, minx);
            miny = (int) floor((bbox.min.y + halfdiag.j) / voxlen); miny = max(0, miny);
            minz = (int) floor((bbox.min.z + halfdiag.k) / voxlen); minz = max(0, minz);
            maxx = (int) ceil((bbox.max.x + halfdiag.i) / voxlen); maxx = min(maxx, xdim-1);
            maxy = (int) ceil((bbox.max.y + halfdiag.j) / voxlen); maxy = min(maxy, ydim-1);
            maxz = (int) ceil((bbox.max.z + halfdiag.k) / voxlen); maxz = min(maxz, zdim-1);

            // iterate over voxels in bounding box, testing whether it is inside the cylinder or not
            for(x = minx; x <= maxx; x++)
                for(y = miny; y <= maxy; y++)
                    for(z = minz; z < maxz; z++)
                    {
                        pnt = vpPoint(voxlen * (float) x - halfdiag.i, voxlen * (float) y - halfdiag.j, voxlen * (float) z - halfdiag.k); // convert from voxel space to world coordinates
                        if(cyl.pointInCylinder(pnt))
                        {
                            vox.set(x, y, z, true);
                            voxcount++;
                        }
                    }
        }
    }

    cerr << "vox cylinders fin" << endl;
    cerr << "voxels set = " << voxcount << endl;

    halfdiag.mult(-1.0f);
    voxmesh.marchingCubes(vox, voxsidelen, halfdiag);

    cerr << "MC fin" << endl;
}

bool Scene::duplicateValidity()
{
    bool found = false;
    int v = 0, vrest = 0, e = 0, erest = 0;

    // don't use FindEdge or FindVert because these will search the whole list every time

    // search vertex list for duplicates
    while(!found && v < (int) verts.size()-1)
    {
        // search remainder of list
        vrest = v+1;
        while(!found && vrest < (int) verts.size())
        {
            if(verts[v] == verts[vrest])
                found = true;
            vrest++;
        }
        v++;
    }

    // search edge list for duplicates
    while(!found && e < (int) edges.size()-1)
    {
        // search remainder of list
        erest = e+1;
        while(!found && erest < (int) edges.size())
        {
            if((edges[v].v[0] == edges[vrest].v[0] && edges[v].v[1] == edges[vrest].v[1]) ||
                (edges[v].v[0] == edges[vrest].v[1] && edges[v].v[1] == edges[vrest].v[0]))
                found = true;
            erest++;
        }
        e++;
    }
    return !found;
}

bool Scene::danglingVertValidity()
{
    vector<bool> dangle;
    int v, e;
    bool valid = true;

    // build flag list corresponding to vertex list. Entry true if a vertex is dangling.
    for(v = 0; v < (int) verts.size(); v++)
        dangle.push_back(true);

    // switch flags for vertices incident on an edge
    for(e = 0; e < (int) edges.size(); e++)
    {
        if(edges[e].v[0] >= 0 && edges[e].v[0] < (int) verts.size())
            dangle[edges[e].v[0]] = false;
        if(edges[e].v[1] >= 0 && edges[e].v[1] < (int) verts.size())
            dangle[edges[e].v[1]] = false;
    }

    // check if any vertices are still flagged as dangling
    for(v = 0; v < (int) verts.size(); v++)
    {
        if(dangle[v])
            valid = false;
    }

    return valid;
}

bool Scene::edgeBoundValidity()
{
    int e;
    bool valid = true;

    for(e = 0; e < (int) edges.size(); e++)
    {
        if(edges[e].v[0] < 0 || edges[e].v[1] < 0 || edges[e].v[0] >= (int) verts.size() || edges[e].v[1] >= (int) verts.size())
            valid = false;
    }
    return valid;
}

bool Scene::connectionValidity()
{
    vector<bool> visited;
    stack<int> traverse;
    int v, e, curre, currv;
    bool valid = true;

    // build flag list corresponding to vertex list. Entry true if a vertex has been visited during traversal.
    for(v = 0; v < (int) verts.size(); v++)
        visited.push_back(false);


    // depth first traversal of edge graph
    traverse.push(0); visited[edges[0].v[0]] = true;
    while(!traverse.empty())
    {
        curre = traverse.top();
        traverse.pop();

        // find which vertex of the edge has yet to be visited (if any)
        currv = -1;
        if(!visited[edges[curre].v[0]])
            currv = edges[curre].v[0];
        else if(!visited[edges[curre].v[1]])
            currv = edges[curre].v[1];

        if(currv >= 0)
        {
            visited[currv] = true;

            // find all edges incident on the unvisited endpoint
            // slow because there is no incident edge list in the scene
            for(e = 0; e < (int) edges.size(); e++)
                if(e != curre)
                {
                    if(edges[e].v[0] == currv) // incident on current vertex
                    {
                        if(!visited[edges[e].v[1]]) // check to see if far vertex is unvisited
                            traverse.push(e);
                    }
                    else if(edges[e].v[1] == currv)
                    {
                        if(!visited[edges[e].v[0]]) // check to see if far vertex is unvisited
                            traverse.push(e);
                    }
                }
        }
    }

    // check if any vertices are still unvisited
    for(v = 0; v < (int) verts.size(); v++)
    {
        if(!visited[v])
            valid = false;
    }
    return valid;
}