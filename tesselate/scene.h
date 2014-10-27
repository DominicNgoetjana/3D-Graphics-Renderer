// Scene class for building up complex constructive solid geometry
// (c) James Gain, 2014

#ifndef _SCENEGRAPH
#define _SCENEGRAPH

#include <vector>
#include <stdio.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "renderer.h"

using namespace std;

struct Edge
{
    int v[2];   ///< index into the vertex list for edge endpoints
};

class Scene
{
private:
    std::vector<vpPoint> verts; ///< vertices of the tesselation structure
    std::vector<Edge> edges;    ///< edges connecting vertices
    GLfloat * col;              ///< (r,g,b,a) colour
    float sphererad;            ///< sphere radius in cm
    float cylrad;               ///< cylinder radius in cm
    float tesslen;              ///< side length of an individual tesselation element in cm
    Vector voldiag;             ///< diagonal of scene bounding box in cm

    /**
     * Search list of vertices to find matching point
     * @param pnt       point to search for in vertex list
     * @param[out] idx  index of point in list if found, otherwise -1
     * @retval @c true  if the point is found in the vertex list
     * @retval @c false otherwise
     */
    bool findVert(vpPoint pnt, int &idx);

    /**
     * Search list of edges to find match
     * @param edge      edge to search for in edge list
     * @param[out] idx  index of edge in list if found, otherwise -1
     * @retval @c true  if the edge is found in the vertex list
     * @retval @c false otherwise
     */
    bool findEdge(Edge edge, int &idx);

public:

    ShapeGeometry geom;         ///< triangle mesh geometry for shape

    Scene();

    ~Scene();

    /// Remove all vertices and edges, resetting the structure
    void clear();

    /// getter for sphere radius (in cm)
    float getSphereRad(){ return sphererad; }

    /// setter for sphere radius (in cm)
    void setSphereRad(float rad){ sphererad = rad; }

    /// getter for cylinder radius (in cm)
    float getCylRad(){ return cylrad; }

    /// setter for cylinder radius (in cm)
    void setCylRad(float rad){ cylrad = rad; }

    /// getter for scene diagonal
    Vector getBoundDiag(){ return voldiag; }

    /// setter for scene diagonal
    void setBoundDiag(Vector diagonal)
    {
        voldiag = diagonal;
        packCubes(vpPoint(-voldiag.i/2.0f, -voldiag.j/2.0f, -voldiag.k/2.0f), voldiag, tesslen);
    }

    /// getter for tesselation length
    float getTessLength(){ return tesslen; }

    /// setter for tesselation length
    void setTessLength(float len)
    {
        tesslen = len;
        packCubes(vpPoint(-voldiag.i/2.0f, -voldiag.j/2.0f, -voldiag.k/2.0f), voldiag, tesslen);
    }

    /**
     * Append a vertex to the list of vertices
     * @param v     vertex to add
     */
    void addVertex(vpPoint v){ verts.push_back(v); }

    /**
     * Append an edge to the list of edges
     * @param v0, v1    indices of endpoints
     */
    void addEdge(int v0, int v1)
    {
        Edge e;
        e.v[0] = v0; e.v[1] = v1;
        edges.push_back(e);
    }

    /**
     * Generate triangle mesh geometry for OpenGL rendering
     * @param view      current view parameters
     * @param[out] sdd  openGL parameters required to draw this geometry
     * @retval @c true  if buffers are bound successfully, in which case sdd is valid
     * @retval @c false otherwise
     */
    bool genGeometry(View * view, ShapeDrawData &sdd);

    /**
     * Add a cube to the tesselation structure
     * @param origin    bottom, left, front corner of cube in world space
     * @param len       length of a side of the cube
     * @retval @c true if this cube bolts onto an existing structure
     * @retval @c false if the cube is isolated
     */
    bool addCube(vpPoint origin, float len);

    /**
     * Pack a bounding volume with cubes
     * @param origin    bottom, left, front corner of the cube volume
     * @param extent    3D extent of the volume (a diagonal vector)
     * @param cubelen   length of a cube within the volume, any cubes spilling outside are culled
     */
    void packCubes(vpPoint origin, Vector extent, float cubelen);

    /**
     * Test that there are no duplicate vertices or edges in the scene
     * @retval @c true if there are no duplications
     * @retval @c false otherwise.
     */
    bool duplicateValidity();

    /**
     * Test that there are no isolated vertices that do not belong to at least one edge
     * @retval @c true if there are no isolated vertices
     * @retval @c false otherwise.
     */
    bool danglingVertValidity();

    /**
     * Test that edge indices point to actual vertices
     * @retval @c true if all edge indices are in bounds
     * @retval @c false otherwise.
     */
    bool edgeBoundValidity();

    /**
     * Test that the scene forms a single connected structure
     * @retval @c true if any vertex can be reached by edge traversal from any other
     * @retval @c false otherwise
     */
    bool connectionValidity();
};

#endif