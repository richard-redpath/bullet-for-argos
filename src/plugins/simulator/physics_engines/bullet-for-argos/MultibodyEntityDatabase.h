//
// Created by Richard Redpath on 10/11/15.
//

#ifndef ARGOS3_BULLET_URDFDATABASE_H
#define ARGOS3_BULLET_URDFDATABASE_H

#include <string>
#include <map>
#include <vector>
#include <mutex>

#include "MultibodyDefinitions.h"

#include "MeshInfo.h"

#include <argos3/core/utility/datatypes/datatypes.h>

using namespace argos;

/**
 * Definition of a (potentially) multibody entity which stores its mass,
 * joints and links. The main constructor which should be called is the
 * one which takes an XML filename to describe this entity.
 */
class MultibodyDefinition
{
public:
//	/**
//	 * Create an empty entity with no bodies or links and no mass
//	 */
	MultibodyDefinition(){};

	/**
	 * Parse the XML file specified to extract the required morphology
	 * and material definitions.
	 */
	MultibodyDefinition(std::string fileName);

	/**
	 * Return the mass of this entity, recalculating it if the cached version
	 * is dirty.
	 */
	inline Real GetMass()
	{
		if(recalcMass)						// If the mass is dirty
		{
			mass = 0;						// Reset it
			for(auto p : links)
				mass += p.second.mass;		// Add the mass of each sub link
			recalcMass = false;				// Clear the dirty flag
		}
		return mass;
	}

	/**
	 * Gets the name of this entity as defined in its XML file.
	 */
	const std::string getName() { return name; }

	/**
	 * Returns a reference to the name->link map for this entity.
	 * Provided to allow using classes to iterate over all links
	 * or access them directly by name
	 */
    const std::map<std::string, Link>& getLinkMap() { return links; }

	/**
	 * Returns a reference to the name->joint map for this entity.
	 * Serves a similar purpose to getLinkMap().
	 */
    const std::map<std::string, JointDefinition>& getJointMap() { return joints; }

	/**
	 * Returns a saved mesh to avoid needing to reparse and recreate trimeshes.
	 */
	static MeshInfo& GetMeshInfo(int meshIdx)
	{
		return *meshes[meshIdx];
	}


private:
	static std::vector<MeshInfo*> meshes;
	static std::map<std::string, int> meshIndices;

	std::map<std::string, Link> links;
    std::map<std::string, JointDefinition> joints;

    std::string name;
	std::string fileName;

	float mass = 0;
	bool recalcMass = false;

	/**
	 * Parse an XML node to build up a new link and add it to the link map
	 */
	void addLink(ticpp::Element *element, MaterialPrototypeStack& materialStack);

	/**
	 * Parse an XML node to build up a new joint and add it to the joint map
	 */
	void addJoint(ticpp::Element *element, MaterialPrototypeStack& materialStack);

	/**
	 * Parse the inertia coefficients for the provided link from the provided XML node
	 */
	void handleInertialTag(ticpp::Element* inertialTag, Link& link);

	/**
	 * Parse a single visual or collision tag and add it to the provided link.
	 */
	void handleVisualOrCollision(ticpp::Element* definitionElement, Link& link, MaterialPrototypeStack& materialStack, bool asVisual);

	/**
	 * Parse a material from XML and return a prototype definition of it.
	 */
	MaterialPrototype parseSingleMaterial(ticpp::Element* definitionElement);

	/**
	 * Looks for any material which are children of the provided XML node. Any which are found
	 * are parsed and pushed onto the material stack.
	 */
	void getMaterialsForElement(ticpp::Element* element, MaterialPrototypeStack& materialStack);
};

/**
 * A class which is responsible for loading any multibody entities from XML files and saving their
 * definitions. Parse results are cached to avoid spending unnecessary parsing time.
 */
class MultibodyEntityDatabase
{
public:
	static MultibodyEntityDatabase & getInstance();
	MultibodyDefinition* getModel(std::string fileName);
private:
	std::map<std::string, MultibodyDefinition*> loadedModels;
};


#endif //ARGOS3_BULLET_URDFDATABASE_H
