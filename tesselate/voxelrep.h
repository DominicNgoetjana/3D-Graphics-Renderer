#ifndef VOXELREP_H
#define VOXELREP_H

#include <vector>
#include <map>
#include "vecpnt.h"
#include <iostream>

using namespace std;

class VoxelRep
{
private:
    vector<vector<int>> voxR;                       ///<    matrix storing { numTrisRemSelf, numTrisRemAcc, startPos } per row
    std::map<string, string> opp;                   ///<    opposing cube faces
    std::map<string, int> side;                     ///<    relative position of each cube face in triangle list (unaltered)
    vector<vector<int>> removedFacesCode;           ///<    matrix storing side code of each removed face
    vector<vector<vector<int>>> entryCount;
    int numEntries = 0;
public:

    VoxelRep() : entryCount(32, vector<vector<int>>(32, vector<int>(32, -1)))
    {
        opp["T"] = "Bm";
        opp["Bm"] = "T";
        opp["Bk"] = "F";
        opp["F"] = "Bk";
        opp["R"] = "L";
        opp["L"] = "R";
        //
        side["T"] = 0;
        side["Bm"] = 2;
        side["Bk"] = 4;
        side["R"] = 6;
        side["L"] = 8;
        side["F"] = 10;
    }

    /**
     * Add new voxel rep element to voxR and update entry count
     * @param number of triangles removed from current cube, position of cube in grid
     */
    void add(int numTrisRemSelf, vector<int> pos)
    {
        if (voxR.size() == 0)
        {
            voxR.push_back( {0, 0, 0} );
        } else {
            voxR.push_back( {numTrisRemSelf, numTrisRemSelf + voxR[numEntries-1][1], numEntries*12 - voxR[numEntries-1][1]} );
        }
        entryCount[pos[0]][pos[1]][pos[2]] = numEntries;
        removedFacesCode.push_back({});
        numEntries++;
    }

    /**
     * update voxR with changes to triangles removed in cube at pos
     * @param number of triangles removed from cube at pos, position of cube in grid
     */
    void update(int numTris, vector<int> pos)
    {
        int tmp = entryCount[pos[0]][pos[1]][pos[2]];
        voxR[tmp][0] += numTris;
        voxR[tmp][1] += numTris;
        if (numEntries >= 2)
        {
            for (int i = tmp+1; i < voxR.size(); ++i) {
                voxR[i][1] += numTris;
                voxR[i][2] -= numTris;
            }
        }
    }

    /**
     * get entry count of cube at pos
     * @param position of cube in grid
     * @return entry count
     */
    int getEntryCount(vector<int> pos)
    {
        return entryCount[pos[0]][pos[1]][pos[2]];
    }

    string getOpposite(string original)
    {
        return opp[original];
    }

    int getSideCode(string s, vector<int> pos)
    {
        int subject = side[s];
        int removed = 0;
        int eCount = getEntryCount(pos);

        for (int i = 0; i < removedFacesCode[eCount].size(); ++i) {
            if (removedFacesCode[eCount][i] < subject)
                removed+=2;
        }

        return subject - removed;
    }

    int getStartPos(int eCount)
    {
        return voxR[eCount][2];
    }

    void addRemovedFaceCode(int eCount, /*vector<int> pos*/ string s)
    {
        removedFacesCode[eCount].push_back(side[s]);
    }
};

#endif // VOXELREP_H
