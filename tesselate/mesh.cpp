//
// Mesh
//

#include "mesh.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <sys/stat.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/intersect.hpp>

using namespace std;

GLfloat stdCol[] = {0.7f, 0.7f, 0.75f, 0.4f};
const int raysamples = 3;

bool Mesh::findVert(vpPoint pnt, int &idx)
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

void Mesh::deriveVertNorms()
{
    vector<int> vinc; // number of faces incident on vertex
    int p, t;
    Vector n;

    // init structures
    for(p = 0; p < (int) verts.size(); p++)
    {
        vinc.push_back(0);
        norms.push_back(Vector(0.0f, 0.0f, 0.0f));
    }

    // accumulate face normals into vertex normals
    for(t = 0; t < (int) tris.size(); t++)
    {
        n = tris[t].n; n.normalize();
        for(p = 0; p < 3; p++)
        {
            norms[tris[t].v[p]].add(n);
            vinc[tris[t].v[p]]++;
        }
    }

    // complete average
    for(p = 0; p < (int) verts.size(); p++)
    {
        norms[p].mult(1.0f/((float) vinc[p]));
        norms[p].normalize();
    }
}

void Mesh::buildTransform(glm::mat4x4 &tfm)
{
    glm::mat4x4 idt;

    idt = glm::mat4(1.0f);
    tfm = glm::translate(idt, glm::vec3(trx.i, trx.j, trx.k));
    tfm = glm::rotate(tfm, zrot, glm::vec3(0.0f, 0.0f, 1.0f));
    tfm = glm::rotate(tfm, yrot, glm::vec3(0.0f, 1.0f, 0.0f));
    tfm = glm::rotate(tfm, xrot, glm::vec3(1.0f, 0.0f, 0.0f));
    tfm = glm::scale(tfm, glm::vec3(scale));
}

Mesh::Mesh()
{
    col = stdCol;
    scale = 1.0f;
    xrot = yrot = zrot = 0.0f;
    trx = Vector(0.0f, 0.0f, 0.0f);
}

Mesh::~Mesh()
{
    clear();
}

void Mesh::clear()
{
    verts.clear();
    tris.clear();
    geom.clear();
    col = stdCol;
    scale = 1.0f;
    xrot = yrot = zrot = 0.0f;
    trx = Vector(0.0f, 0.0f, 0.0f);
}

bool Mesh::genGeometry(View * view, ShapeDrawData &sdd)
{
    vector<int> faces;
    int t, p;
    glm::mat4x4 tfm;

    geom.clear();
    geom.setColour(col);

    // transform mesh data structures into a form suitable for rendering
    // by flattening the triangle list
    for(t = 0; t < (int) tris.size(); t++)
        for(p = 0; p < 3; p++)
            faces.push_back(tris[t].v[p]);

    // construct transformation matrix
    buildTransform(tfm);
    geom.genMesh(&verts, &norms, &faces, tfm);

    // bind geometry to buffers and return drawing parameters, if possible
    if(geom.bindBuffers(view))
    {
        sdd = geom.getDrawParameters();
        return true;
    }
    else
       return false;
}

bool Mesh::containedPoint(vpPoint pnt)
{
    int incount = 0, outcount = 0, hits, i, t, p;
    glm::vec3 v[3], origin, xsect, ray;
    vpPoint vert;
    Vector dir;
    glm::mat4x4 tfm, idt;
    glm::vec4 vproj;

    // ideally this needs an acceleration structure, such as a bounding sphere hierarchy

    srand(time(0));

    // sample over multiple rays to avoid numerical issues (e.g., ray hits a vertex or edge)
    origin = glm::vec3(pnt.x, pnt.y, pnt.z);
    // cerr << "origin: " << pnt.x << ", " << pnt.y << ", " << pnt.z << endl;

    // construct transformation matrix
    buildTransform(tfm);

    for(i = 0; i < raysamples; i++)
    {
        hits = 0;

        // sampling ray with random direction
        // avoid axis aligned rays because more likely to lead to numerical issues with axis aligned structures
        dir = Vector((float) (rand()%1000-500), (float) (rand()%1000-500), (float) (rand()%1000-500));
        dir.normalize();
        ray = glm::vec3(dir.i, dir.j, dir.k);

        for(t = 0; t < (int) tris.size(); t++)
        {
            for(p = 0; p < 3; p++)
            {
                vert = verts[tris[t].v[p]];
                vproj = tfm * glm::vec4(vert.x, vert.y, vert.z, 1.0f);
                v[p] = glm::vec3(vproj.x, vproj.y, vproj.z);
            }
            if(glm::intersectRayTriangle(origin, ray, v[0], v[1], v[2], xsect) || glm::intersectRayTriangle(origin, ray, v[0], v[2], v[1], xsect)) // test triangle in both windings because intersectLineTriangle is winding dependent
                hits++;
        }

        // cerr << "num hits on " << i << " = " << hits << endl;
        if(hits%2 == 0) // even number of intersection means point is outside
            outcount++;
        else // point is inside
            incount++;
    }

    // consensus wins
    return (incount > outcount);
}

void Mesh::boxFit(float sidelen)
{
    vpPoint bnear, bfar, pnt;
    Vector shift, diag, halfdiag;
    float scale;
    int v;

    // calculate current bounding box
    bnear = vpPoint(HUGE_VALF, HUGE_VALF, HUGE_VALF);
    bfar = vpPoint(-HUGE_VALF, -HUGE_VALF, -HUGE_VALF);
    for(v = 0; v < (int) verts.size(); v++)
    {
        pnt = verts[v];
        if(pnt.x < bnear.x) bnear.x = pnt.x;
        if(pnt.y < bnear.y) bnear.y = pnt.y;
        if(pnt.z < bnear.z) bnear.z = pnt.z;
        if(pnt.x > bfar.x) bfar.x = pnt.x;
        if(pnt.y > bfar.y) bfar.y = pnt.y;
        if(pnt.z > bfar.z) bfar.z = pnt.z;
    }

    if((int) verts.size() > 0)
    {
        // calculate translation necessary to move center of bounding box to the origin
        diag = Vector(bfar.x - bnear.x, bfar.y - bnear.y, bfar.z - bnear.z);
        shift.pntconvert(bnear);
        halfdiag = diag; halfdiag.mult(0.5f);
        shift.add(halfdiag);
        shift.mult(-1.0f);

        // scale so that largest side of bounding box fits sidelen
        scale = max(diag.i, diag.j); scale = max(scale, diag.k);
        scale = sidelen / scale;

        // shift center to origin and scale uniformly
        for(v = 0; v < (int) verts.size(); v++)
        {
            pnt = verts[v];
            shift.pntplusvec(pnt, &pnt);
            pnt.x *= scale; pnt.y *= scale; pnt.z *= scale;
            verts[v] = pnt;
        }
    }
}

bool Mesh::readSTL(string filename)
{
    ifstream infile;
	char * inbuffer;
    struct stat results;
    int insize, inpos, numt, t, i;
    vpPoint vpos;
    Triangle tri;

    infile.open((char *) filename.c_str(), ios_base::in | ios_base::binary);
	if(infile.is_open())
	{
		// get the size of the file
		stat((char *) filename.c_str(), &results);
		insize = results.st_size;

		// put file contents in buffer
		inbuffer = new char[insize];
		infile.read(inbuffer, insize);
		if(!infile) // failed to read from the file for some reason
		{
            cerr << "Error Mesh::readSTL: unable to populate read buffer" << endl;
			return false;
		}

		// interpret buffer as STL file
        if(insize <= 84)
        {
            cerr << "Error Mesh::readSTL: invalid STL binary file, too small" << endl;
            return false;
        }

        inpos = 80; // skip 80 character header
        if(inpos+4 >= insize){ cerr << "Error Mesh::readSTL: malformed header on stl file" << endl; return false; }
        numt = (int) (* ((long *) &inbuffer[inpos]));
        inpos += 4;

        t = 0;
        while(t < numt) // read in triangle data
        {
            // normal
            if(inpos+12 >= insize){ cerr << "Error Mesh::readSTL: malformed stl file" << endl; return false; }
            // IEEE floating point 4-byte binary numerical representation, IEEE754, little endian
            tri.n = Vector((* ((float *) &inbuffer[inpos])), (* ((float *) &inbuffer[inpos+4])), (* ((float *) &inbuffer[inpos+8])));
            // cerr << "n = " << tri.n.i << ", " << tri.n.j << ", " << tri.n.k << endl;
            inpos += 12;

            // vertices
            for(i = 0; i < 3; i++)
            {
                if(inpos+12 >= insize){ cerr << "Error Mesh::readSTL: malformed stl file" << endl; return false; }
                vpos = vpPoint((* ((float *) &inbuffer[inpos])), (* ((float *) &inbuffer[inpos+4])), (* ((float *) &inbuffer[inpos+8])));
                if(!findVert(vpos, tri.v[i])) // new point not yet in vertex list, so add
                {
                    tri.v[i] = (int) verts.size();
                    verts.push_back(vpos);
                }
                inpos += 12;
            }
            tris.push_back(tri);
            t++;
            inpos += 2; // handle attribute byte count - which can simply be discarded
        }

        // tidy up
		delete inbuffer;
		infile.close();

        deriveVertNorms();
    }
    else
    {
        cerr << "Error Mesh::readSTL: unable to open " << filename << endl;
        return false;
    }
    return true;

}

bool Mesh::writeSTL(string filename)
{
    ofstream outfile;
    int t, p, numt;
    unsigned short val;

    outfile.open((char *) filename.c_str(), ios_base::out | ios_base::binary);
	if(outfile.is_open())
	{
        outfile.write("File Generated by Tesselator. Binary STL", 80); // skippable header
        numt = (int) tris.size();
        outfile.write((char *) &numt, 4);

        for(t = 0; t < numt; t++)
        {
            // normal
            outfile.write((char *) &tris[t].n.i, 4);
            outfile.write((char *) &tris[t].n.j, 4);
            outfile.write((char *) &tris[t].n.k, 4);

            // triangle vertices
            for(p = 0; p < 3; p++)
            {
                outfile.write((char *) &verts[tris[t].v[p]].x, 4);
                outfile.write((char *) &verts[tris[t].v[p]].y, 4);
                outfile.write((char *) &verts[tris[t].v[p]].z, 4);
            }

            // attribute byte count - null
            val = 0;
            outfile.write((char *) &val, 2);
        }

        // tidy up
		outfile.close();
    }
    else
    {
        cerr << "Error Mesh::writeSTL: unable to open " << filename << endl;
        return false;
    }
    return true;
}