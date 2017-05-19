//
// Created by rar500 on 02/11/15.
//

#include "tinyobjloader/tiny_obj_loader.h"
#include <iostream>
#include "MeshInfo.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace tinyobj;

/**
 * Used to load OBJ files (currently the only mesh type supported)
 * and associated texture files
 */
bool MeshInfo::LoadFromFile(std::string sourceFile)
{
	// Clear and deallocate any old data.
	ClearData(true);

	// Where to store the parsed data
	std::vector<shape_t> shapes;
	std::vector<material_t> mats;

	// Get the directory that this mesh exists in
	int pathEnd = sourceFile.find_last_of('/');
	std::string dir = sourceFile.substr(0, pathEnd + 1);

	// Try to parse the OBJ file
	std::string err = LoadObj(shapes, mats, sourceFile.c_str(), dir.c_str());

	// Somewhere to store required material definitions
	MeshTexture *loadedTextures[mats.size()];

	// Load all materials
	for (int i = 0; i < mats.size(); ++i) {
		// Get the current material
		auto &mat = mats[i];

		// Allocate a new mesh object
		loadedTextures[i] = new MeshTexture;

		// Get the mesh texture object we just created
		auto loadedMat = loadedTextures[i];

		// What is the name of the texture file we should use?
		auto filename = mat.diffuse_texname.c_str();
		int width, height, components;

		// Clear the renderer texture ID, we are about to load new data
		loadedMat->renderId = 0;

		stbi_set_flip_vertically_on_load(1);
		// Load the new image file into an unsigned char array using stb_image
		auto data = stbi_load((dir + filename).c_str(), &width, &height, &components, 4);

		// Number of data elements we need to copy
		int numElements = width * height * 4;

		if (data != nullptr) {
			// Allocate new space to store the image data
			loadedMat->data = new unsigned char[numElements];    //(unsigned char*)malloc(numElements * sizeof(unsigned char));

			// Copy the loaded data to our mesh texture's array
			memcpy(loadedMat->data, data, numElements * sizeof(unsigned char));

			// Set our texture size
			loadedMat->height = height;
			loadedMat->width = width;

			// Clear the loaded data
			free(data);
		}
		else {
			loadedMat->data = nullptr;
			loadedMat->height = -1;
			loadedMat->width = -1;
		}
	}

	// Abort if an error has occurred
	if (!err.empty())
		return false;

	// If no shapes are found then abort
	unsigned long nShapes = shapes.size();
	if (nShapes == 0)
		return false;

	// Iterate over all bodies
	for (int i = 0; i < nShapes; ++i) {
		// Get the name and mesh datal
		auto shapeName = shapes[i].name;
		auto mesh = shapes[i].mesh;

		// Extract all vertices
		float *verts = new float[mesh.positions.size()];

		for (int v = 0; v < mesh.positions.size(); ++v)
			verts[v] = mesh.positions[v];

		// Extract all edges
		int *indices = new int[mesh.indices.size()];

		for (int ind = 0; ind < mesh.indices.size(); ++ind)
			indices[ind] = mesh.indices[ind];

		float *uvs = new float[mesh.texcoords.size()];
		for (int ind = 0; ind < mesh.texcoords.size(); ++ind)
			uvs[ind] = mesh.texcoords[ind];

		vertsMap[shapeName] = verts;
		numVerts[shapeName] = mesh.positions.size();

		indicesMap[shapeName] = indices;
		numIndices[shapeName] = mesh.indices.size();

		if(mesh.material_ids[0] >= 0) {
			MeshTexture *t = loadedTextures[mesh.material_ids[0]];
			textures[shapeName] = t;
		}

		uvsMap[shapeName] = uvs;

		keys.push_back(shapeName);
	}

	return true;
}

/**
 * Clear the data stored about this texture. Free the memory of associated
 * meshes if freeResources is true
 */
void MeshInfo::ClearData(bool freeResources)
{
	// Clear the resources for all mesh parts
	for(auto key : keys)
	{
		// Free memory if requested
		if (freeResources)
		{
			delete vertsMap[key];
			delete indicesMap[key];
		}

		// Clear the data from associated maps
		vertsMap.erase(key);
		numVerts.erase(key);

		indicesMap.erase(key);
		numIndices.erase(key);
	}

	// We should also deallocate meshes if we are asked to free resources
	if(freeResources)
	{
		// Iterate over all MeshTextures
		for(auto& pair : textures)
		{
			// Free the image data
			free(pair.second->data);

			// Then delete the MeshTexture object
			delete(pair.second);
		}

		// And empty the texture map
		textures.clear();
	}
}
