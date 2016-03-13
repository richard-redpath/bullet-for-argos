//
// Created by rar500 on 10/11/15.
//

#include <algorithm>
#include <set>

#include "MultibodyEntityDatabase.h"

#include "StringFuncs.h"

using namespace ticpp;

MultibodyEntityDatabase &MultibodyEntityDatabase::getInstance()
{
	static MultibodyEntityDatabase instance;
	return instance;
}

MultibodyDefinition* MultibodyEntityDatabase::getModel(std::string fileName)
{
	// Load the model if we haven't already
	if(loadedModels.find(fileName)==loadedModels.end())
		loadedModels[fileName] = new MultibodyDefinition{fileName};

	// Return a pointer to the model
	return loadedModels[fileName];
}

MultibodyDefinition::MultibodyDefinition(std::string fileName)
{
	// Parse the file
	Document doc;
	doc.LoadFile(fileName);

	// Get the root tag
	Element *node = doc.FirstChildElement("entity");
	name = node->GetAttribute("name");

	/*
	 * Parse all materials in the root of the tree to bootstrap
	 * the material definition process
	 */
	MaterialPrototypeStack materialStack;
	getMaterialsForElement(node, materialStack);

	// Parse all links
	Element *currentLink = node->FirstChildElement("link", false);
	while(currentLink)
	{
		addLink(currentLink, materialStack);
		currentLink = currentLink->NextSiblingElement("link", false);
	}

	// ... and joints
	Element *currentJoint = node->FirstChildElement("joint", false);
	while(currentJoint)
	{
		addJoint(currentJoint, materialStack);
		currentJoint = currentJoint->NextSiblingElement("joint", false);
	}

	// Set link parents from joint definitions
	for(auto& jointPair : joints)
	{
		auto& joint = jointPair.second;
		auto& childLink = links[joint.child];
		auto& parentLink = links[joint.parent];
		childLink.parent = &parentLink;
	}
}

/*
 * Parse a single material tag for the provided element and return a populated definition
 */
MaterialPrototype MultibodyDefinition::parseSingleMaterial(ticpp::Element *definitionElement)
{
	MaterialPrototype ret;

	// Do we have contact values? If so get them, defaulting to NaN for any missing ones
	Element* physicalDefs = definitionElement->FirstChildElement("contact_coefficients", false);
	if(physicalDefs)
	{
		// Extract the values we are interested in
		extractFromString(physicalDefs->GetAttributeOrDefault("mu", "NaN"), ret.mean.friction);
		extractFromString(physicalDefs->GetAttributeOrDefault("kp", "NaN"), ret.mean.stiffness);
		extractFromString(physicalDefs->GetAttributeOrDefault("kd", "NaN"), ret.mean.dampening);

		// Extract the noise values from the definitions
		extractFromString(physicalDefs->GetAttributeOrDefault("mu_std", "NaN"), ret.std_dev.friction);
		extractFromString(physicalDefs->GetAttributeOrDefault("kp_std", "NaN"), ret.std_dev.stiffness);
		extractFromString(physicalDefs->GetAttributeOrDefault("kd_std", "NaN"), ret.std_dev.dampening);
	}

	// Do we have colour values? If so get them
	Element* colourDefs = definitionElement->FirstChildElement("color", false);
	if(colourDefs)
		extractFromString(colourDefs->GetAttribute("rgba"), ret.colour.red, ret.colour.green, ret.colour.blue, ret.colour.alpha);

	return ret;
}

/*
 * Parse all materials from the current element and add them to the definition stack
 */
void MultibodyDefinition::getMaterialsForElement(ticpp::Element *element, MaterialPrototypeStack& materialStack)
{
	// Get the first material node which is a child of element
	Element* materialElement = element->FirstChildElement("material", false);
	while(materialElement)
	{
		// Extract the material values from it
		MaterialPrototype proto = parseSingleMaterial(materialElement);

		// Add it to the material stack, overriding values where necessary
		materialStack.setMaterial(materialElement->GetAttribute("name"), proto);

		// Get the next element or null if this is the last one
		materialElement = materialElement->NextSiblingElement("material", false);
	}
}

/*
 * Extract the required information from a link XML element
 * and add the defined link to our representation
 */
void MultibodyDefinition::addLink(Element *element, MaterialPrototypeStack& materialStack)
{
	if(!element)
		return;

	// Name of the link
	std::string linkname = element->GetAttribute("name");

	// And its definition
	Link newLink;
	newLink.name = linkname;

	// Parse any link specific material tags
	materialStack.pushLevel();
	getMaterialsForElement(element, materialStack);

	// Check if inertia is provided and handle it if it is
	Element *inertial = element->FirstChildElement("inertial", false);
	if(inertial)
		handleInertialTag(inertial, newLink);

	// Check if either or both visual and collision shapes are given
	Element *visual = element->FirstChildElement("visual", false);
	Element *collision = element->FirstChildElement("collision", false);

	// Used to determine how we handle definitions, if only one is provided
	// we use it for both visual and collision
	bool hasVisual = (visual != nullptr);
	bool hasCollision = (collision != nullptr);

	// Handle visual and collision shapes differently
	if(hasVisual && hasCollision)
	{
		while(visual)
		{
			handleVisualOrCollision(visual, newLink, materialStack, true);

			visual = visual->NextSiblingElement("visual", false);
		}

		while(collision)
		{
			handleVisualOrCollision(collision, newLink, materialStack, false);

			collision = collision->NextSiblingElement("collision", false);
		}
	}
	// Only use visual for both visual and collision
	else if(hasVisual)
	{
		while(visual)
		{
			handleVisualOrCollision(visual, newLink, materialStack, true);
			handleVisualOrCollision(visual, newLink, materialStack, false);

			visual = visual->NextSiblingElement("visual", false);
		}
	}
	// Only use collision for both
	else if(hasCollision)
	{
		while(collision)
		{
			handleVisualOrCollision(collision, newLink, materialStack, true);
			handleVisualOrCollision(collision, newLink, materialStack, false);

			collision = collision->NextSiblingElement("collision", false);
		}
	}
	// We dont have visual or collision definitions - exit
	else
	{
		std::cerr << "No shape definition found (collision or visual) when parsing definition of entity (" << name <<
		" in file " << fileName << ")" << std::endl;
		exit(1);
	}

	// Try to access the material specified
	Element* materialElement = element->FirstChildElement("material", false);
	if(materialElement)
	{
		const MaterialInstance* material = materialStack.getMaterialInstance(materialElement->GetAttribute("name"));
		if(material)
			newLink.collisionMaterial = *material;
		delete(material);
	}

	// Record our specification
	links[linkname] = newLink;

	// And mark the mass as dirty since it will need to be recalculated
	recalcMass = true;

	// Restore the old material definitions
	materialStack.popLevel();
}

void MultibodyDefinition::addJoint(Element *element, MaterialPrototypeStack& materialStack)
{
	if(!element)
		return;

	// Make sure we have a name
	if(!element->HasAttribute("name"))
	{
		std::cerr<<"Missing 'name' attribute for joint when parsing entity (" << name << " in file " << fileName << ")" << std::endl;
		exit(1);
	}
	// And get it
	std::string jointName = element->GetAttribute("name");

	// Make sure we have a type
	if(!element->HasAttribute("type"))
	{
		std::cerr<<"Unable to find 'type' attribute for joint ("<<jointName<<") when parsing entity (" << name << " in file " << fileName << ")" << std::endl;
		exit(1);
	}
	// And get it
	std::string jointType = element->GetAttribute("type");

	// Start a new definition
	JointDefinition joint;
	joint.name = jointName;

	// Get the joint type
	if(jointType == "revolute")
		joint.type = Revolute;
	else if(jointType == "continuous")
		joint.type = Continuous;
	else if(jointType == "prismatic")
		joint.type = Prismatic;
	else if(jointType == "fixed")
		joint.type = Fixed;
	else if(jointType == "floating")
		joint.type = Floating;
	else if(jointType == "planar")
		joint.type = Planar;
	else
	{
		std::cerr << "Unknown joint type (" << jointType << ") provided when parsing joint (" << jointName << ") for entity (" << name << " in file " << fileName << ")" << std::endl;
		exit(1);
	}

	// Extract the origin of this link (default to all 0)
	Element* origin = element->FirstChildElement("origin", false);
	extractFromString((origin ? origin->GetAttributeOrDefault("xyz", "0 0 0") : "0 0 0"), joint.originX, joint.originY, joint.originZ);
	extractFromString((origin ? origin->GetAttributeOrDefault("rpy", "0 0 0") : "0 0 0"), joint.originRoll, joint.originPitch, joint.originYaw);

	// Extract the parent tag and check a link name is provided
	Element* parentTag = element->FirstChildElement("parent", false);
	if(!parentTag)
	{
		std::cerr << "No parent provided when parsing joint (" << jointName << ") for entity (" << name << " in file " << fileName << ")" << std::endl;
		exit(1);
	}

	if(!parentTag->HasAttribute("link"))
	{
		std::cerr << "Parent tag provided does not have required 'link' attribute when parsing joint (" << jointName << ") for entity (" << name << " in file " << fileName << ")" << std::endl;
		exit(1);
	}

	// Extract the child link and ensure a link name is provided
	Element* childTag = element->FirstChildElement("child", false);
	if(!childTag)
	{
		std::cerr << "No child provided when parsing joint (" << jointName << ") for entity (" << name << " in file " << fileName << ")" << std::endl;
		exit(1);
	}

	if(!childTag->HasAttribute("link"))
	{
		std::cerr << "Child tag provided does not have required 'link' attribute when parsing joint (" << jointName << ") for entity (" << name << " in file " << fileName << ")" << std::endl;
		exit(1);
	}

	// Extract the names of parent and child
	joint.parent = parentTag->GetAttribute("link");
	joint.child = childTag->GetAttribute("link");

	// Ensure the links provided have been defined
	if(links.find(joint.parent) == links.end())
	{
		std::cerr<<"Parent link ("<<joint.parent<<") specified by joint ("<<jointName<<") has not been defined when parsing joint (" << jointName << ") for entity (" << name << " in file " << fileName << ")" << std::endl;
		exit(1);
	}

	if(links.find(joint.child) == links.end())
	{
		std::cerr<<"Child link ("<<joint.child<<") specified by joint ("<<jointName<<") has not been defined when parsing joint (" << jointName << ") for entity (" << name << " in file " << fileName << ")" << std::endl;
		exit(1);
	}

	// Determine the axis that this joint works on
	Element* axisElement = element->FirstChildElement("axis", false);
	if(axisElement)
	{
		// Axis tag has been provided by xyz is missing, probably a mistake, alert the user
		if(!axisElement->HasAttribute("xyz"))
		{
			std::cerr<<"Axis tag provided without xyz attribute for joint ("<<jointName<<") has not been defined when parsing joint (" << jointName << ") for entity (" << name << " in file " << fileName << ")" << std::endl;
			exit(1);
		}
	}
	extractFromString((axisElement ? axisElement->GetAttribute("xyz") : "1 0 0"), joint.axisX, joint.axisY, joint.axisZ);

	// Determine characteristics of the joint
	Element* dynamicsElement = element->FirstChildElement("dynamics", false);
	if(dynamicsElement)
	{
		extractFromString(dynamicsElement->GetAttributeOrDefault("damping", "0"), joint.dynamicsDamping);
		extractFromString(dynamicsElement->GetAttributeOrDefault("friction", "0"), joint.dynamicsFriction);
	}
	else
	{
		joint.dynamicsFriction = 0;
		joint.dynamicsDamping = 0;
	}

	// Ensure that the limit tag is provided if it is required
	Element* limitElement = element->FirstChildElement("limit", false);
	if((joint.type == Revolute || joint.type == Prismatic) && (!limitElement))
	{
		std::cerr<<"Required limit tag not found when parsing "<<(joint.type == Revolute ? "Revolute" : "Prismatic")<<" joint ("<<jointName<<") for entity (" << name << " in file " << fileName << ")" << std::endl;
		exit(1);
	}

	// Extract the limit attributes, ensuring that the required ones are provided
	if(limitElement)
	{
//		if(!limitElement->HasAttribute("effort"))
//		{
//			std::cerr<<"No 'effort' attribute provided for 'limit' tag when parsing joint ("<<jointName<<") for entity (" << name << " in file " << fileName << ")" << std::endl;
//			exit(1);
//		}
		if(!limitElement->HasAttribute("velocity"))
		{
			std::cerr<<"No 'velocity' attribute provided for 'limit' tag when parsing joint ("<<jointName<<") for entity (" << name << " in file " << fileName << ")" << std::endl;
			exit(1);
		}

		// Provide default values where necessary
		extractFromString(limitElement->GetAttributeOrDefault("lower", "0"), joint.limitLower);
		extractFromString(limitElement->GetAttributeOrDefault("upper", "0"), joint.limitUpper);
		extractFromString(limitElement->GetAttributeOrDefault("effort", "100"), joint.limitEffort);
		extractFromString(limitElement->GetAttribute("velocity"), joint.limitVelocity);
	}

	// Error when an unsupported joint type is requested
	switch(joint.type)
	{
		case Continuous:
		case Fixed:
			break;
		case Revolute:
		case Prismatic:
		case Floating:
		case Planar:
		{
			std::cerr << "Joint type (" << jointType << ") not yet supported (used for joint " << jointName << " when parsing entity " << this->name << " in file " << fileName << ")" << std::endl;
			exit(1);
		}
	}

    joints[jointName] = joint;
}

/**
 * Extracts the x, y, z, roll, pitch and yaw attributes from the provided tag
 */
void extractLocationAttributes(Element *element, float &x, float &y, float &z, float &roll, float &pitch, float &yaw)
{
	// Both of these attribute are optional and should default to "0 0 0" if the tag or attribute is not provided
	std::string xyzString = (element ? element->GetAttributeOrDefault("xyz", "0 0 0") : "0 0 0");
	std::string rpyString = (element ? element->GetAttributeOrDefault("rpy", "0 0 0") : "0 0 0");

	// Get the values from them
	extractFromString(xyzString, x, y, z);
	extractFromString(rpyString, roll, pitch, yaw);
}

/**
 * Extract the mass properties for the provided link from the inertial tag
 */
void MultibodyDefinition::handleInertialTag(Element *inertial, Link &link)
{
	if(!inertial)
		return;

	link.inertiaProvided = true;

//	// Check if a center of gravity is provided
//	Element *origin = inertial->FirstChildElement("origin", false);
//	if(origin)
//		// And parse it if so
//		extractLocationAttributes(origin, link.originX, link.originY, link.originZ, link.roll, link.pitch, link.yaw);

	// Mass is required so we log and error and exit if it is not provided
	Element *massElement = inertial->FirstChildElement("mass", false);
	if(!massElement)
	{
		std::cerr << "Required mass element missing from inertial when parsing definition of entity (" << name <<
		" in file " << fileName << ")" << std::endl;
		exit(1);
	}

	// More safety checking
	if(!massElement->HasAttribute("value"))
	{
		std::cerr << "Mass tag in inertial missing 'value' attribute when parsing definition of entity (" << name <<
		" in file " << fileName << ")" << std::endl;
		exit(1);
	}

	// No need for default as we are sure the value exists already
	extractFromString(massElement->GetAttribute("value"), link.mass);

	// Check if the inertia matrix values are provided and error if not
	Element *inertiaElement = inertial->FirstChildElement("inertia", false);
	if(!inertiaElement)
	{
		std::cerr << "Required inertia element missing from inertial when parsing definition of entity (" << name <<
		" in file " << fileName << ")" << std::endl;
		exit(1);
	}

	if(!(inertiaElement->HasAttribute("ixx") && inertiaElement->HasAttribute("ixy") &&
		 inertiaElement->HasAttribute("ixz")
		 && inertiaElement->HasAttribute("iyy") && inertiaElement->HasAttribute("iyz") &&
		 inertiaElement->HasAttribute("izz")))
	{
		std::cerr <<
		"Missing attribute (require ixx, ixy, ixz, iyy, iyz, izz) from inertia tag when parsing definition of entity (" <<
		name << " in file " << fileName << ")" << std::endl;
		exit(1);
	}

	// We now know that all inertia values are present so we can extract their values
	extractFromString(inertiaElement->GetAttribute("ixx"), link.ixx);
	extractFromString(inertiaElement->GetAttribute("ixy"), link.ixy);
	extractFromString(inertiaElement->GetAttribute("ixz"), link.ixz);
	extractFromString(inertiaElement->GetAttribute("iyy"), link.iyy);
	extractFromString(inertiaElement->GetAttribute("iyz"), link.iyz);
	extractFromString(inertiaElement->GetAttribute("izz"), link.izz);
}

/*
 * Parse a visual or collision element which contains geometry definitions and possibly also materials
 */
void MultibodyDefinition::handleVisualOrCollision(ticpp::Element *definitionElement, Link &link, MaterialPrototypeStack& materialStack, bool asVisual)
{
	// Add a new level to the material stack
	materialStack.pushLevel();

	// Check the type of geometry specified
	// @TODO: ErrorCheck
	Element *geometryElement = definitionElement->FirstChildElement("geometry", false);
	if(!geometryElement)
	{
		std::cerr<<"No geometry provided for "<<(asVisual ? "visual" : "collision")<<" definition of link ("<<link.name<<") when parsing entity (" <<
		name << " in file " << fileName << ")" << std::endl;
		exit(1);
	}
	Element *shapeDefinition = geometryElement->FirstChildElement(false);
	if(!shapeDefinition)
	{
		std::cerr<<"No shape provided to describe "<<(asVisual ? "visual" : "collision")<<" geometry of link ("<<link.name<<") when parsing entity (" <<
		name << " in file " << fileName << ")" << std::endl;
		exit(1);
	}

	// Extract the type of shape from the DOM
	GeometrySpecification spec;

	// Extract the origin
	Element *originElement = definitionElement->FirstChildElement("origin", false);
	extractLocationAttributes(originElement, spec.originX, spec.originY, spec.originZ, spec.roll, spec.pitch, spec.yaw);

	if(shapeDefinition->Value() == "box")
		spec.type = GeometryType::Box;
	else if(shapeDefinition->Value() == "cylinder")
		spec.type = GeometryType::Cylinder;
	else if(shapeDefinition->Value() == "sphere")
		spec.type = GeometryType::Sphere;
	else if(shapeDefinition->Value() == "mesh")
		spec.type = GeometryType::Mesh;
	else
	{
		std::cerr << "Unsupported geometry type " << shapeDefinition->Value() <<
		" when parsing definition of entity (" << name << " in file " << fileName << ")" << std::endl;
		exit(1);
	}

	// Extract (only) the required attributes
	switch(spec.type)
	{
		case Box:
		{
			extractFromString(shapeDefinition->GetAttribute("size"), spec.box.x, spec.box.y, spec.box.z);
			break;
		}
		case Cylinder:
		{
			extractFromString(shapeDefinition->GetAttribute("radius"), spec.cylinder.radius);
			extractFromString(shapeDefinition->GetAttribute("length"), spec.cylinder.length);
			break;
		}
		case Sphere:
		{
			extractFromString(shapeDefinition->GetAttribute("radius"), spec.sphere.radius);
			break;
		}
		case Mesh:
		{
			// Check the file name of the mesh is provided
			if(!shapeDefinition->HasAttribute("filename"))
			{
				std::cerr << "Mesh geometry specified but no filename given when parsing definition of entity (" <<
				name << " in file " << fileName << ")" << std::endl;
				exit(1);
			}

			// Check if this mesh has already been loaded
			std::string meshFileName = shapeDefinition->GetAttribute("filename");

			// If not we need to load it
			if(meshIndices.find(meshFileName) == meshIndices.end())
			{
				// Check if we can handle it
				// Since many formats are text based we will use file extensions to determine type
				if(endsWith(meshFileName, ".obj"))
				{
					// If we can then we should load it, record its location, and add its definition
					MeshInfo info;
					info.LoadFromFile(meshFileName);
					meshIndices[meshFileName] = (int)meshes.size();
					meshes.push_back(info);
				}
				else
				{
					// We can't handle this yet, report an error
					std::cerr << "Unsupported mesh file (" << meshFileName <<
					") when parsing definition of entity (" << name <<
					" in file " << fileName << ")" << std::endl;
					exit(1);
				}
			}

			//The mesh is now guaranteed to be in memory so get its index
			spec.mesh.mesh = &(meshes[meshIndices[meshFileName]]);

			// Extract a scale if one is provided
			extractFromString(shapeDefinition->GetAttributeOrDefault("scale", "1 1 1"), spec.mesh.sx, spec.mesh.sy,
							  spec.mesh.sz);
			break;
		}
	}

	// Try to determine which material to use
	if(asVisual)
	{
		// Do we have a material to use?
		Element *materialElement = definitionElement->FirstChildElement("material", false);
		if (!materialElement)
		{
			// Try to get the parent material
			Node *pNode = definitionElement->Parent(false);
			if (pNode)
			{
				materialElement = pNode->ToElement()->FirstChildElement("material", false);
			}
		}

		if (materialElement)
		{
			// Add any additional materials
			materialStack.pushLevel();
			getMaterialsForElement(definitionElement, materialStack);

			spec.materialColour = materialStack.getMaterialDefinition(materialElement->GetAttribute("name"))->colour;

			// Discard any materials added
			materialStack.popLevel();
		}
		else
		{
			// If no material definition is found then bail out
			std::cerr << "Failed to find material for " << (asVisual ? "visual" : "collision") <<
			" definition of link (" << link.name << ") when parsing definition of entity (" << name <<
			" in file " << fileName << ")" << std::endl;
			exit(1);
		}
	}

	// Add to the required vector
	(asVisual ? link.visual : link.collision).push_back(spec);

	// Remove locally parsed materials
	materialStack.popLevel();
}

/*
 * Static definitions
 */
std::vector<MeshInfo> MultibodyDefinition::meshes;
std::map<std::string, int> MultibodyDefinition::meshIndices;
