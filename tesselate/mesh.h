/**
 * @file
 *
 * Data structure representing a triangle mesh in 3D space.
 */

#ifndef _MESH
#define _MESH

#include <vector>
#include <stdio.h>
#include <iostream>
#include "renderer.h"
#include "ffd.h"
#include "voxels.h"
#include <unordered_set>

using namespace std;

const int sphperdim = 5;
const bool sphereaccel = false; // controls whether or not the bounding sphere acceleration structure is used.
                                // better set as <false> for any mesh with fewer than a hundred triangles.

/**
 * A triangle in 3D space, with 3 indices into a vertex list and an outward facing normal. Triangle winding is counterclockwise.
 */
struct Triangle
{
    int v[3];   ///< index into the vertex list for triangle vertices
    cgp::Vector n;   ///< outward facing unit normal to the triangle

    bool vertexFound(int vertex)
    {
        if (vertex == v[0] || vertex == v[1] || vertex == v[2])
            return true;
        return false;
    }

    void deriveNormal(vector<cgp::Vector> & normals, vector<cgp::Point> & vts)
    {
        ///< points (coordinates) at each triangle vertex
        cgp::Point p1 = vts[v[0]];
        cgp::Point p2 = vts[v[1]];
        cgp::Point p3 = vts[v[2]];

        ///< derive vectors
        cgp::Vector a, b;
        a.diff(p3, p2);
        b.diff(p1, p2);

        ///< cross vectors -> derive normal
        cgp::Vector normal;
        normal.cross(a, b);

        ///< invert normal
        normal.mult(-1);

        ///< normalize before comparing
        normal.normalize();

        ///< add to normals, if not in already
        bool found = false;
        for (int i = 0; i < normals.size(); ++i) {
            if (normals[i] == normal)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            normals.push_back(normal);
         }
    }

    void printVec()
    {
        cerr << "(" << v[0] << "," << v[1] << "," << v[2] << ")" << endl;
    }
};

/**
 * An edge in 3D space, with 2 indices into a vertex list. The order of the vertices may have significance.
 */
struct Edge
{
    int v[2];   ///< indices into the vertex list for edge endpoints
};

/**
 * Abstract base class for shapes
 */
class BaseShape
{
public:

    /// virtual destructor
    virtual ~BaseShape(){}

    /**
     * Generate geometry for OpenGL rendering
     * @param[out] geom triangle-mesh geometry packed for OpenGL
     * @param view      current view parameters
     */
    virtual void genGeometry(ShapeGeometry * geom, View * view)=0;

    /**
     * Test whether a point falls inside the shape. Will need to be overridden by each inheriting class.
     * @param pnt   point to test for containment
     * @retval true if the point falls within the shape, 
     * @retval false otherwise
     */
    virtual bool pointContainment(cgp::Point pnt)=0;
};

/**
 * A sphere in 3D space, consisting of a center and radius. Used for bounding sphere hierarchy acceleration.
 */
class Sphere: public BaseShape
{
public:
    cgp::Point c;  ///< sphere center
    float r;       ///< sphere radius
    std::vector<int> ind; ///< triangle vertex indices included in the bounding sphere. Used for acceleration struct, otherwise ignored

    /// Default Constructor
    Sphere()
    {
        c = cgp::Point(0.0f, 0.0f, 0.0f);
        r = 0.0f;
    }

    /**
     * Constructor
     * @param center    sphere center
     * @param radius    sphere radius
     */
    Sphere(cgp::Point center, float radius)
    {
        c = center;
        r = radius;
    }

   /**
     * Generate sphere geometry for OpenGL rendering
     * @param[out] geom triangle-mesh geometry packed for OpenGL
     * @param view      current view parameters
     */
    void genGeometry(ShapeGeometry * geom, View * view);

    /**
     * Test whether a point falls inside the sphere
     * @param pnt   point to test for containment
     * @retval true if the point falls within the sphere,
     * @retval false otherwise
     */
    bool pointContainment(cgp::Point pnt);

};

/**
 * A cylinder in 3D space,
 */
class Cylinder: public BaseShape
{
public:
    cgp::Point s, e;   ///< start and end points of the cylinder
    float r;        ///< cylinder radius

    /// Default Constructor
    Cylinder()
    {
        s = cgp::Point(0.0f, 0.0f, 0.0f);
        e = s;
        r = 0.0f;
    }

    /**
     * Constructor
     * @param start     start vertex of spine of cylinder
     * @param end       end vertex of spine of cylinder
     * @param radius    cylinder radius
     */
    Cylinder(cgp::Point start, cgp::Point end, float radius)
    {
        s = start;
        e = end;
        r = radius;
    }

   /**
     * Generate cylinder geometry for OpenGL rendering
     * @param[out] geom triangle-mesh geometry packed for OpenGL
     * @param view      current view parameters
     */
    void genGeometry(ShapeGeometry * geom, View * view);

    /**
     * Test whether a point falls inside the cylinder
     * @param pnt   point to test for containment
     * @retval true if the point falls within the sphere, 
     * @retval false otherwise
     */
    bool pointContainment(cgp::Point pnt);
};

/**
 * A cube in 3D space,
 */
class Square: public BaseShape
{
public:
    cgp::Point c;   ///< square center
    float l;        ///< square length

    /// Default Constructor
    Square()
    {
        c = cgp::Point(0.0f, 0.0f, 0.0f);
        l = 1.0f;
    }

    /**
     * Constructor
     * @param start     square center
     * @param radius    square length
     */
    Square(cgp::Point start, float length)
    {
        c = start;
        l = length;
    }

   /**
     * Generate square geometry for OpenGL rendering
     * @param[out] geom triangle-mesh geometry packed for OpenGL
     * @param view      current view parameters
     */
    void genGeometry(ShapeGeometry * geom, View * view);

    /**
     * Test whether a point falls inside the square
     * @param pnt   point to test for containment
     * @retval true if the point falls within the sphere,
     * @retval false otherwise
     */
    bool pointContainment(cgp::Point pnt);
};

/**
 * A triangle mesh in 3D space. Ideally this should represent a closed 2-manifold but there are validity tests to ensure this.
 */
class Mesh: public BaseShape
{
private:
    std::vector<cgp::Point> verts; ///< vertices of the tesselation structure
    std::vector<cgp::Point> base; ///< undistorted vertices prior to deformation
    std::vector<cgp::Vector> norms;  ///< per vertex normals
    std::vector<Triangle> tris; ///< triangles that join to make up the mesh
    GLfloat * col;              ///< (r,g,b,a) colour
    float scale;                ///< scaling factor
    cgp::Vector trx;                 ///< translation
    float xrot, yrot, zrot;     ///< rotation angles about x, y, and z axes
    std::vector<Sphere> boundspheres; ///< bounding sphere accel structure

    /**
     * Search list of vertices to find matching point
     * @param pnt       point to search for in vertex list
     * @param[out] idx  index of point in list if found, otherwise -1
     * @retval true  if the point is found in the vertex list,
     * @retval false otherwise
     */
    bool findVert(cgp::Point pnt, int &idx);

    /**
     * Construct a hash key based on a 3D point
     * @param pnt   point to convert to key
     * @param bbox  bounding box enclosing all mesh vertices
     * @retval hash key
     */
    long hashVert(cgp::Point pnt, cgp::BoundBox bbox);

    /**
     * Construct a hash key based on the indices of an edge
     * @param v0    first endpoint index
     * @param v1    second endpoint index
     * @retval hash key
     */
    long hashEdge(int v0, int v1);

    /// Connect triangles together by merging duplicate vertices
    void mergeVerts();

    /// Generate vertex normals by averaging normals of the surrounding faces
    void deriveVertNorms();

    /// Generate face normals from triangle vertex positions
    void deriveFaceNorms();

    /**
     * Composite rotations, translation and scaling into a single transformation matrix
     * @param tfm   composited transformation matrix
     */
    void buildTransform(glm::mat4x4 &tfm);

    /**
     * Compare two Triangles to see if they index the same vertices
     * @param t1    first triangle
     * @param t2    second triangle
     * @return true if the triangles are the same
     * @return false otherwise
     */
    bool sameTriangle(Triangle t1, Triangle t2);

    /**
     * Compare two Edges to see if they index the same vertices
     * @param e1    first edge
     * @param e2    second edge
     * @param[out] opposite true if the edges traverse in opposite directions
     * @return true if the edges index the same vertices
     * @return false otherwise
     */
    bool sameEdge(Edge e1, Edge e2, bool & opposite);

    /**
     * Create bounding sphere acceleration structure for mesh
     * @param maxspheres    the number of spheres placed along the longest side of the bounding volume
     */
    void buildSphereAccel(int maxspheres);

public:

    ShapeGeometry geometry;         ///< renderable version of mesh

    /// Default constructor
    Mesh();

    /// Destructor
    virtual ~Mesh();

    /// Remove all vertices and triangles, resetting the structure
    void clear();

    /// Test whether mesh is empty of any geometry (true if empty, false otherwise)
    bool empty(){ return verts.empty(); }

    /// Setter for scale
    void setScale(float scf){ scale = scf; }

    /// Getter for scale
    float getScale(){ return scale; }

    /// Setter for translation
    void setTranslation(cgp::Vector tvec){ trx = tvec; }

    /// Getter for translation
    cgp::Vector getTranslation(){ return trx; }

    /// Setter for rotation angles
    void setRotations(float ax, float ay, float az){ xrot = ax; yrot = ay; zrot = az; }

    /// Getter for rotation angles
    void getRotations(float &ax, float &ay, float &az){ ax = xrot; ay = yrot; az = zrot; }

    /// Setter for colour
    void setColour(GLfloat * setcol){ col = setcol; }

    /// Getter for number of vertices
    int getNumVerts(){ return (int) verts.size(); }

    void mergeAllVerts() { mergeVerts(); }

    /// Getter for vertices
    vector<cgp::Point>* getVerts() { return &verts; }

    /// Getter for number of faces
    int getNumFaces(){ return (int) tris.size(); }

    /// Merge meshses
    void mergeMesh(Mesh * m2, bool lastCall=false);

    /// Setter for cube triangles
    void setCubeTriangles() {
        tris.clear();
        Triangle t;
        t.v[0] = 1; t.v[1] = 2; t.v[2] = 3; // front
        tris.push_back(t);
        t.v[0] = 3; t.v[1] = 4; t.v[2] = 1;
        tris.push_back(t);

        t.v[0] = 4; t.v[1] = 3; t.v[2] = 5; // right
        tris.push_back(t);
        t.v[0] = 5; t.v[1] = 6; t.v[2] = 4;
        tris.push_back(t);

        t.v[0] = 6; t.v[1] = 5; t.v[2] = 7; // back
        tris.push_back(t);
        t.v[0] = 7; t.v[1] = 8; t.v[2] = 6;
        tris.push_back(t);

        t.v[0] = 8; t.v[1] = 7; t.v[2] = 2; // left
        tris.push_back(t);
        t.v[0] = 2; t.v[1] = 1; t.v[2] = 8;
        tris.push_back(t);

        t.v[0] = 8; t.v[1] = 1; t.v[2] = 4; // top
        tris.push_back(t);
        t.v[0] = 4; t.v[1] = 6; t.v[2] = 8;
        tris.push_back(t);

        t.v[0] = 2; t.v[1] = 7; t.v[2] = 5; // bottom
        tris.push_back(t);
        t.v[0] = 5; t.v[1] = 3; t.v[2] = 2;
        tris.push_back(t);
    }

    // Getter for cube triangles
    vector<Triangle>* getCubeTriangles() {
        return &tris;
    }

    /**
     * Generate and bind triangle mesh geometry for OpenGL rendering
     * @param view      current view parameters
     * @param[out] sdd  openGL parameters required to draw this geometry
     * @retval @c true  if buffers are bound successfully, in which case sdd is valid
     * @retval @c false otherwise
     */
    bool bindGeometry(View * view, ShapeDrawData &sdd);

    /**
     * Generate triangle mesh geometry for OpenGL rendering
     * @param[out] geom triangle-mesh geometry packed for OpenGL
     * @param view      current view parameters
     */
    void genGeometry(ShapeGeometry * geom, View * view);

    /**
     * Test whether a point falls inside the mesh using ray-mesh intersection tests
     * @param pnt   point to test for containment
     * @retval true if the point falls within the mesh, 
     * @retval false otherwise
     */
    bool pointContainment(cgp::Point pnt);

    /**
     * Scale geometry to fit bounding cube centered at origin
     * @param sidelen   length of one side of the bounding cube
     */
    void boxFit(float sidelen);

    /**
     * Apply marching cubes to a voxel volume to generate a mesh
     * @param vox           voxel volume
     */
    void marchingCubes(VoxelVolume * vox);

    /**
     * Apply in-place simple Laplacian smoothing to the mesh
     * @param iter  number of smoothing iterations
     * @param rate  proportion of full Laplacian that is applied on each iteration
     */
    void laplacianSmooth(int iter, float rate);

    /**
     * Apply a free-form deformation to the mesh
     * @param lat   ffd lattice being applied
     */
    void applyFFD(ffd * lat);

    /**
     * Read in triangle mesh from STL format binary file
     * @param filename  name of file to load (STL format)
     * @retval true  if load succeeds,
     * @retval false otherwise.
     */
    bool readSTL(string filename);

    /**
     * Write triangle mesh to STL format binary file
     * @param filename  name of file to save (STL format)
     * @retval true  if save succeeds,
     * @retval false otherwise.
     */
    bool writeSTL(string filename);

    /**
     * Read in 3D vector data from file
     * @param filename  name of file to read
     */
    void readGrid(vector<vector<vector<int>>> &voxelgrid, string filename, int len);

    /**
     * Write 3D vector to output file
     * @param filename  name of file to save
     */
    bool writeGrid(vector<vector<vector<int>>> &voxelgrid, string outfilename, int len);

    /**
     * Basic mesh validity tests - no duplicate vertices, no dangling vertices, edge indices within bounds of the vertex list
     * @retval true if basic validity tests are passed,
     * @retval false otherwise
     * @todo basicValidity requires completing for CGP Prac1
     */
    bool basicValidity();

    /**
     * Check that the mesh is a closed two-manifold
     * @retval true if the mesh is two-manifold,
     * @retval false otherwise
     * @todo manifoldValidity requires completing for CGP Prac1
     */
    bool manifoldValidity();

    /**
     * Test that the mesh forms a single connected structure
     * @retval true if any vertex can be reached by edge traversal from any other
     * @retval false otherwise
     * @todo connectionValidity requires completing for CGP Prac1
     */
    bool connectionValidity();

    /**
     * Build a simple valid 2-manifold tetrahedron with correct winding
     */
    void validTetTest();

    /**
     * Build a simple mesh that will break basicValidity, with duplicate and dangling vertices and out of bound indices.
     */
    void basicBreakTest();

    /**
     * Build two simple tetrahedra that touch at a single vertex
     */
    void touchTetsTest();

    /**
     * Build a simple 2-manifold partial tetrahedron with boundary
     */
    void openTetTest();

    /**
     * Build a simple tetrahedron that has a double shell breaking 2-manifold validity
     */
    void overlapTetTest();
};

#endif
