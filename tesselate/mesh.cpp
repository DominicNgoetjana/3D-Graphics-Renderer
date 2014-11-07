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
#include <list>
#include <sys/stat.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/intersect.hpp>

using namespace std;

GLfloat stdCol[] = {0.7f, 0.7f, 0.75f, 0.4f};
const int raysamples = 3;

bool AccelSphere::pointInSphere(vpPoint pnt)
{
    Vector delvec;

    delvec.diff(c, pnt);
    if(delvec.sqrdlength() < r*r)
        return true;
    else
        return false;
}

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

void Mesh::buildSphereAccel(int maxspheres)
{
    vector<AccelSphere> packedspheres;
    list<int> insphere;
    int i, v, p, t, s, numsx, numsy, numsz, sx, sy, sz, x, y, z, sind = 0;
    vpPoint centroid, center;
    Vector radvec, diag, edgevec;
    float radius, dist, packrad, packspace, maxedge;
    BoundBox bbox;
    AccelSphere sph;
    bool found;

    if((int) tris.size() > 0) // must actually have a mesh to build the acceleration structure on top of
    {
        // calculate longest triangle edge
        maxedge = 0.0f;
        for(t = 0; t < (int) tris.size(); t++)
        {
            edgevec.diff(verts[tris[t].v[0]], verts[tris[t].v[1]]);
            dist = edgevec.length();

            edgevec.diff(verts[tris[t].v[1]], verts[tris[t].v[2]]);
            dist = min(dist, edgevec.length());

            edgevec.diff(verts[tris[t].v[2]], verts[tris[t].v[0]]);
            dist = min(dist, edgevec.length());
            if(dist > maxedge)
                maxedge = dist;
        }

        // calculate mesh bounding box
        for(v = 0; v < (int) verts.size(); v++)
            bbox.includePnt(verts[v]);

        diag = bbox.getDiag();
        dist = std::max(diag.i, diag.j); dist = std::max(dist, diag.k);
        packrad = dist / (float) maxspheres;
        if(packrad < maxedge) // sphere radius must be larger than the longest shorest edge in any triangle
        {
            cerr << "Error Mesh::buildSphereAccel: bounding spheres too small relative to edge length. Inflating spheres accordingly." << endl;
            packrad = maxedge;
        }
        packspace = 2.0f * packrad;

        // number of spheres to pack in each dimension filling the bounding box
        numsx = (int) ceil(diag.i / packspace) +1;
        numsy = (int) ceil(diag.j / packrad) +1;
        numsz = (int) ceil(diag.k / packrad) +1;

        // pack spheres into volume so that there is no unsampled space
        // alternating rows are offset by the sphere radius and x-adjacent spheres are spaced to just touch
        // this provides a minimal overlap between spheres that nevertheless covers all available space
        for(x = 0; x < numsx; x++)
            for(y = 0; y < numsy; y++)
                for(z = 0; z < numsz; z++)
                {
                    center = vpPoint((float) x * packspace + bbox.min.x, (float) y * packrad + bbox.min.y, (float) z * packrad + bbox.min.z);
                    if(z%2==1) // row shifting alternates in layers
                    {
                        if(y%2 == 1) // shift alternating rows by half spacing in x
                            center.x += packrad;
                    }
                    else
                    {
                        if(y%2 == 0) // shift alternating rows by half spacing in x
                            center.x += packrad;
                    }

                    sph.c = center;
                    sph.r = packrad;
                    sph.ind.clear();
                    packedspheres.push_back(sph);
                }

        // assign triangles to spheres based on their vertices
        for(t = 0; t < (int) tris.size(); t++)
        {

            // find the first sphere that contains any of the triangle vertices
            found = false; s = 0;
            while(!found && s < (int) packedspheres.size())
            {
                for(p = 0; p < 3; p++)
                    if(packedspheres[s].pointInSphere(verts[tris[t].v[p]]))
                    {
                        found = true;
                        // add triangle to packedsphere
                        packedspheres[s].ind.push_back(t);
                        sind = s;
                    }
                s++;
            }

            if(!found)
            {
                cerr << "Error Mesh::buildSphereAccel: packed spheres do not entirely fill the volume." << endl;
            }
            else
            {
                // once a sphere triangle intersection is found only a few adjacent spheres need be tested for intersection
                // also test against +x, +y, +z spheres (and diagonal adjacents as well for possible intersection due to partial sphere overlaps

                // neighbour search of packed spheres
                // decompose into x, y, z sphere grid position

                // unflatten index
                sx = (int) ((float) sind / (float) (numsy*numsz));
                sy = (int) ((float) sind / (float) numsz);
                sz = sind - (sx * numsy * numsz + sy * numsz);
                for(x = sx; x <= sx+1; x++)
                    for(y = sy; y <= sy+1; y++)
                        for(z = sz; z <= sz+1; z++)
                        {
                            if(x < numsx && y < numsy && z < numsz) // bounds check
                                if(!(x == sx && y == sy && z == sz)) // ignore starter sphere
                                {
                                    s = x * numsy * numsz + y * numsz + z; // flatten 3d index
                                    found = false;
                                    for(p = 0; p < 3; p++)
                                        if(packedspheres[s].pointInSphere(verts[tris[t].v[p]]))
                                            found = true;
                                    if(found) // add triangle to packedsphere
                                        packedspheres[s].ind.push_back(t);
                                }

                        }

            }
        }

        // discard any spheres containing no triangles
        for(i = 0; i < (int) packedspheres.size(); i++)
        {
            if(!packedspheres[i].ind.empty()) // bounding sphere has at least one triangle
                boundspheres.push_back(packedspheres[i]);
        }
        packedspheres.clear();

        // recaculate spheres by using center of mass and furthest triangle vertex to get tighter bounding
        for(i = 0; i < (int) boundspheres.size(); i++)
        {
            insphere.clear();
            // gather included vertices
            for(t = 0; t < (int) boundspheres[i].ind.size(); t++)
                for(p = 0; p < 3; p++)
                    insphere.push_back(tris[boundspheres[i].ind[t]].v[p]);

            // remove duplicates because they will bias the center of mass
            insphere.sort();
            insphere.unique();

            // calculate centroid as average of vertices
            centroid = vpPoint(0.0f, 0.0f, 0.0f);
            for (list<int>::iterator it=insphere.begin(); it!=insphere.end(); ++it)
            {
                centroid.x += verts[(* it)].x; centroid.y += verts[(* it)].y; centroid.z += verts[(* it)].z;

            }
            centroid.x /= (float) insphere.size();  centroid.y /= (float) insphere.size();  centroid.z /= (float) insphere.size();
            boundspheres[i].c = centroid;

            // calculate new radius
            radius = 0.0f;
            for (list<int>::iterator it=insphere.begin(); it!=insphere.end(); ++it)
            {
                radvec.diff(boundspheres[i].c, verts[(* it)]);
                dist = radvec.length();
                if(dist > radius)
                    radius = dist;
            }
            boundspheres[i].r = radius;

            /*
            // check current radius
            for (list<int>::iterator it=insphere.begin(); it!=insphere.end(); ++it)
            {
                radvec.diff(boundspheres[i].c, verts[(* it)]);
                dist = radvec.length();
                if(dist > boundspheres[i].r)
                    cerr << "Error Mesh::buildSphereAccel:vertex " << (* it) << " outside containing sphere by " << (dist - packrad) << endl;
            }*/
        }
    }
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
    for(int i = 0; i < (int) boundspheres.size(); i++)
        boundspheres[i].ind.clear();
    boundspheres.clear();
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

    /*
    // generate geometry for acceleration spheres for testing
    for(p = 0; p < (int) boundspheres.size(); p++)
    {
        glm::mat4x4 idt;

        idt = glm::mat4(1.0f);
        tfm = glm::translate(idt, glm::vec3(trx.i+boundspheres[p].c.x, trx.j+boundspheres[p].c.y, trx.k+boundspheres[p].c.z));
        tfm = glm::rotate(tfm, zrot, glm::vec3(0.0f, 0.0f, 1.0f));
        tfm = glm::rotate(tfm, yrot, glm::vec3(0.0f, 1.0f, 0.0f));
        tfm = glm::rotate(tfm, xrot, glm::vec3(1.0f, 0.0f, 0.0f));
        tfm = glm::scale(tfm, glm::vec3(scale));
        geom.genSphere(boundspheres[p].r, 20, 20, tfm);
    }*/

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
    int incount = 0, outcount = 0, hits, i, t, p, s;
    glm::vec3 v[3], origin, xsect, ray;
    glm::mat4x4 tfm, idt;
    glm::vec4 vproj;
    vpPoint vert;
    Vector dir;
    float dist, tval;
    list<int> inspheres;

    srand(time(0));

    // sample over multiple rays to avoid numerical issues (e.g., ray hits a vertex or edge)
    origin = glm::vec3(pnt.x, pnt.y, pnt.z);

    // construct transformation matrix
    buildTransform(tfm);

    if(boundspheres.empty()) // no acceleration structure so build
        buildSphereAccel(sphperdim);

    for(i = 0; i < raysamples; i++)
    {
        hits = 0;
        inspheres.clear();

        // sampling ray with random direction
        // avoid axis aligned rays because more likely to lead to numerical issues with axis aligned structures
        dir = Vector((float) (rand()%1000-500), (float) (rand()%1000-500), (float) (rand()%1000-500));
        dir.normalize();
        ray = glm::vec3(dir.i, dir.j, dir.k);

        // gather potential triangle intersector indices and remove duplicates
        for(s = 0; s < (int) boundspheres.size(); s++)
        {
            rayPointDist(pnt, dir, boundspheres[s].c, tval, dist);
            if(dist <= boundspheres[s].r) // if ray hits the bounding sphere
                for(t = 0; t < (int) boundspheres[s].ind.size(); t++) // include all triangles allocated to the bounding sphere
                    inspheres.push_back(boundspheres[s].ind[t]);
        }

        // remove duplicate triangle indices because a triangle may appear in multiple bounding spheres
        inspheres.sort();
        inspheres.unique();

        // test intersection against list of triangles
        for (list<int>::iterator it=inspheres.begin(); it!=inspheres.end(); ++it)
        {
            for(p = 0; p < 3; p++)
            {
                vert = verts[tris[(* it)].v[p]];
                vproj = tfm * glm::vec4(vert.x, vert.y, vert.z, 1.0f);
                v[p] = glm::vec3(vproj.x, vproj.y, vproj.z);
            }
            if(glm::intersectRayTriangle(origin, ray, v[0], v[1], v[2], xsect) || glm::intersectRayTriangle(origin, ray, v[0], v[2], v[1], xsect)) // test triangle in both windings because intersectLineTriangle is winding dependent
                hits++;
        }

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
    vpPoint pnt;
    Vector shift, diag, halfdiag;
    float scale;
    int v;
    BoundBox bbox;

    // calculate current bounding box
    for(v = 0; v < (int) verts.size(); v++)
        bbox.includePnt(verts[v]);

    if((int) verts.size() > 0)
    {
        // calculate translation necessary to move center of bounding box to the origin
        diag = bbox.getDiag();
        shift.pntconvert(bbox.min);
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

        buildSphereAccel(sphperdim);
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
        clear();

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