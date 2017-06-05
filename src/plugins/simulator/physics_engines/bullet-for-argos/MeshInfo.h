//
// Created by rar500 on 02/11/15.
//

#ifndef ARGOS3_BULLET_MESHINFO_H
#define ARGOS3_BULLET_MESHINFO_H

#include <string>
#include <map>
#include <vector>

/**
 * Store data about a single texture. The renderId is provided
 * as a convenience for renderer to store internal texture IDs
 */
struct MeshTexture
{
	unsigned int renderId;
	int width, height;
	unsigned char* data = nullptr;
};

/**
 * A full mesh definition
 */
struct MeshInfo
{
	/**
	 * Clear out all maps and (optionally) deallocate associated arrays
	 */
	void ClearData(bool freeResources);

	/**
	 * Load a mesh from the specified file
	 */
	bool LoadFromFile(std::string sourceFile);

	std::vector<std::string> keys;						// A mesh map have several sub-meshes
	std::map<std::string, float*> vertsMap;				// Vertex array of each sub-mesh
	std::map<std::string, float*> uvsMap;				// Texture coordinates of each sub-mesh
	std::map<std::string, MeshTexture*> textures;		// Texture info of each sub-mesh
	std::map<std::string, unsigned long> numVerts;		// How many vertices in each sub-mesh?
	std::map<std::string, int*> indicesMap;				// What are the order of vertices to be draw in each sub-mesh
	std::map<std::string, unsigned long> numIndices;	// How many sets of indices are in this mesh
};

#endif //ARGOS3_BULLET_MESHINFO_H
