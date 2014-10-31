// Mesh class for representing triangle mesh objects
// (c) James Gain, 2014

#ifndef _MESH
#define _MESH

#include <vector>
#include <stdio.h>
#include <iostream>
#include "renderer.h"

using namespace std;

struct Triangle
{
    int v[3];   ///< index into the vertex list for triangle vertices
    Vector n;   ///< outward facing unit normal to the triangle
};

class Mesh
{
private:
    std::vector<vpPoint> verts; ///< vertices of the tesselation structure
    std::vector<Vector> norms;  ///< per vertex normals
    std::vector<Triangle> tris; ///< edges connecting vertices
    GLfloat * col;              ///< (r,g,b,a) colour

    /**
     * Search list of vertices to find matching point
     * @param pnt       point to search for in vertex list
     * @param[out] idx  index of point in list if found, otherwise -1
     * @retval @c true  if the point is found in the vertex list
     * @retval @c false otherwise
     */
    bool findVert(vpPoint pnt, int &idx);

    /// Generate vertex normals by averaging normals of the surrounding faces
    void deriveVertNorms();

    /**
     * Scale geometry to fit bounding cube centered at origin
     * @param sidelen   length of one side of the bounding cube
     */
    void boxFit(float sidelen);

public:

    ShapeGeometry geom;         ///< renderable version of mesh

    Mesh();

    ~Mesh();

    /// Remove all vertices and triangles, resetting the structure
    void clear();

    /**
     * Generate triangle mesh geometry for OpenGL rendering
     * @param view      current view parameters
     * @param[out] sdd  openGL parameters required to draw this geometry
     * @retval @c true  if buffers are bound successfully, in which case sdd is valid
     * @retval @c false otherwise
     */
    bool genGeometry(View * view, ShapeDrawData &sdd);

    /**
     * Test whether a point lies inside the mesh
     * @param pnt   point being tested for containment
     * @retval @c true  if the point lies on or inside the mesh
     * @retval @c false otherwise
     */
    bool containedPoint(vpPoint pnt);

    /**
     * Read in triangle mesh from STL format binary file
     * @param filename  name of file to load (STL format)
     * @retval @c true  if load succeeds
     * @retval @c false otherwise.
     */
    bool readSTL(string filename);

    /**
     * Write triangle mesh to STL format binary file
     * @param filename  name of file to save (STL format)
     * @retval @c true  if save succeeds
     * @retval @c false otherwise.
     */
    bool writeSTL(string filename);
};

#endif