// Mesh class for representing triangle mesh objects
// (c) James Gain, 2014

#ifndef _MESH
#define _MESH

#include <vector>
#include <stdio.h>
#include <iostream>
#include "renderer.h"

using namespace std;

const int sphperdim = 20;

struct Triangle
{
    int v[3];   ///< index into the vertex list for triangle vertices
    Vector n;   ///< outward facing unit normal to the triangle
};

class AccelSphere
{
public:
    vpPoint c;  ///< sphere center
    float r;    ///< spehre radius
    std::vector<int> ind; ///< triangle indices included in the bounding sphere

    /**
     * Test whether a point falls inside the acceleration sphere
     * @param pnt   point to test for containment
     * @retval @c true if the point falls within the sphere
     * @retval @c false otherwise
     */
    bool pointInSphere(vpPoint pnt);
};

class Mesh
{
private:
    std::vector<vpPoint> verts; ///< vertices of the tesselation structure
    std::vector<Vector> norms;  ///< per vertex normals
    std::vector<Triangle> tris; ///< edges connecting vertices
    GLfloat * col;              ///< (r,g,b,a) colour
    float scale;                ///< scaling factor
    Vector trx;                 ///< translation
    float xrot, yrot, zrot;     ///< rotation angle about x, y, and z axes
    std::vector<AccelSphere> boundspheres; ///< bounding sphere accel structure

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
     * Composite rotations, translation and scaling into a single transformation matrix
     * @param tfm   composited transformation matrix
     */
    void buildTransform(glm::mat4x4 &tfm);

    /**
     * Create bounding sphere acceleration structure for mesh
     * @param maxspheres    the number of spheres placed along the longest side of the bounding volume
     */
    void buildSphereAccel(int maxspheres);

public:

    ShapeGeometry geom;         ///< renderable version of mesh

    Mesh();

    ~Mesh();

    /// Remove all vertices and triangles, resetting the structure
    void clear();

    /// Test whether mesh is empty of any geometry (true if empty, false otherwise)
    bool empty(){ return verts.empty(); }

    /// Setter for scale
    void setScale(float scf){ scale = scf; }

    /// Getter for scale
    float getScale(){ return scale; }

    /// Setter for translation
    void setTranslation(Vector tvec){ trx = tvec; }

    /// Getter for translation
    Vector getTranslation(){ return trx; }

    /// Setter for rotation angles
    void setRotations(float ax, float ay, float az){ xrot = ax; yrot = ay; zrot = az; }

    /// Getter for rotation angles
    void getRotations(float &ax, float &ay, float &az){ ax = xrot; ay = yrot; az = zrot; }

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
     * Scale geometry to fit bounding cube centered at origin
     * @param sidelen   length of one side of the bounding cube
     */
    void boxFit(float sidelen);

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