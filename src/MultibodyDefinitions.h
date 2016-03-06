//
// Created by rar500 on 03/11/15.
//

#ifndef ARGOS3_BULLET_URDF_STRUCTS_H
#define ARGOS3_BULLET_URDF_STRUCTS_H

#include "MeshInfo.h"
#include "NumericalHelpers.h"
#include <vector>
#include <argos3/core/simulator/simulator.h>

/**
 * Material properties
 * Contains all physical attributes (non-visual)
 */
class MaterialProperties
{
public:
	MaterialProperties() : friction(NaN), stiffness(NaN), dampening(NaN) {}
	float friction, stiffness, dampening;
};

/**
 * Material visual properties (colour)
 */
class MaterialColour
{
public:
	MaterialColour() : red(NaN), green(NaN), blue(NaN), alpha(NaN) {}
	float red, green, blue, alpha;
};

/**
 * Material instance, contains specific values for all physical
 * and visual attributes including any overloads
 */
class MaterialInstance
{
public:
	MaterialProperties properties;
	MaterialColour colour;
};

/**
 * Material blueprint, contains default values of the material
 * and the associated variance for each property
 */
class MaterialPrototype
{
public:
	MaterialProperties mean, std_dev;
	MaterialColour colour;

	MaterialInstance* getInstance() const
	{
		auto rng = argos::CSimulator::GetInstance().GetRNG();
		MaterialInstance* ret = new MaterialInstance;
		ret->properties = mean;
		ret->colour = colour;

		ret->properties.dampening += rng->Gaussian(std_dev.dampening);
		ret->properties.stiffness += rng->Gaussian(std_dev.stiffness);
		ret->properties.friction += rng->Gaussian(std_dev.friction);

		return ret;
	}
};

/**
 * Maintains a stack of materials allowing specific attributes to be overriden
 */
class MaterialPrototypeStack
{
public:
	MaterialPrototypeStack()
	{
		pushLevel();
	}

	/**
	 * Push a new layer of materials onto the stack
	 */
	void pushLevel()
	{
		materialStack.push_back(std::map<std::string, MaterialPrototype>{});
	}

	/**
	 * Remove the last layer of materials
	 */
	void popLevel()
	{
		materialStack.pop_back();
	}

	/**
	 * Return the current definition associated with the provided name
	 */
	const MaterialPrototype* getMaterialDefinition(std::string name)
	{
		// Start at the last map added to the stack
		for(auto it = materialStack.rbegin(); it != materialStack.rend(); ++it)
			if(it->find(name) != it->end())		// If the material is found
				return &(*it)[name];			// Return it

		return nullptr;							// Otherwise return null
	}

	/**
	 * Return the current definition associated with the provided name
	 */
	const MaterialInstance* getMaterialInstance(std::string name)
	{
		// Start at the last map added to the stack
		for(auto it = materialStack.rbegin(); it != materialStack.rend(); ++it)
			if(it->find(name) != it->end())			// If the material is found
				return (*it)[name].getInstance();	// Instantiate it and return

		return nullptr;								// Otherwise return null
	}

	/**
	 * Allows new materials to be defined or specific properties to be overridden.
	 */
	void setMaterial(std::string name, MaterialPrototype material)
	{
		// See if we have a material with the same name already
		const MaterialPrototype* proto = getMaterialDefinition(name);
		if(proto)
		{
			// If so then we only override what we have an inherit the rest
			material.mean.dampening = (isnan(material.mean.dampening) ? proto->mean.dampening : material.mean.dampening);
			material.mean.friction = (isnan(material.mean.friction) ? proto->mean.friction : material.mean.friction);
			material.mean.stiffness = (isnan(material.mean.stiffness) ? proto->mean.stiffness : material.mean.stiffness);

			material.std_dev.dampening = (isnan(material.std_dev.dampening) ? proto->std_dev.dampening : material.std_dev.dampening);
			material.std_dev.friction = (isnan(material.std_dev.friction) ? proto->std_dev.friction : material.std_dev.friction);
			material.std_dev.stiffness = (isnan(material.std_dev.stiffness) ? proto->std_dev.stiffness : material.std_dev.stiffness);

			material.colour.alpha = (isnan(material.colour.alpha) ? proto->colour.alpha : material.colour.alpha);
			material.colour.red = (isnan(material.colour.red) ? proto->colour.red : material.colour.red);
			material.colour.green = (isnan(material.colour.green) ? proto->colour.green : material.colour.green);
			material.colour.blue = (isnan(material.colour.blue) ? proto->colour.blue : material.colour.blue);
		}
		else
		{
			// If this is the first time this material has been defined then use sensible defaults
			if(isnan(material.mean.friction)) material.mean.friction = 0.5;
			if(isnan(material.mean.stiffness)) material.mean.stiffness = 0.5;
			if(isnan(material.mean.dampening)) material.mean.dampening = 0.5;

			if(isnan(material.std_dev.friction)) material.std_dev.friction = 0;
			if(isnan(material.std_dev.stiffness)) material.std_dev.stiffness = 0;
			if(isnan(material.std_dev.dampening)) material.std_dev.dampening = 0;

			if(isnan(material.colour.red)) material.colour.red = 0.5;
			if(isnan(material.colour.green)) material.colour.green = 0.5;
			if(isnan(material.colour.blue)) material.colour.blue = 0.5;
			if(isnan(material.colour.alpha)) material.colour.alpha = 0.5;
		}
		materialStack.back()[name] = material;
	}
private:
	std::vector<std::map<std::string, MaterialPrototype>> materialStack;
};

/*
 * Bunch of structs which store physical attributes of geometries
 */

/**
 * Attributes for a solid box/cuboid
 */
struct BoxAttributes
{
	float x;
	float y;
	float z;
};

/**
 * Attributes for a cylinder (radius and length, assumed to be in Y)
 */
struct CylinderAttributes
{
	float radius;
	float length;
};

/**
 * Sphere attributes
 */
struct SphereAttributes
{
	float radius;
};

/**
 * Trimesh attributes including mesh info and scale factors in all 3 orientations
 */
struct MeshAttributes
{
	MeshInfo* mesh;
	float sx, sy, sz;
};

/**
 * Types of geometry supported
 */
enum GeometryType
{
	Mesh,
	Box,
	Cylinder,
	Sphere
};

/**
 * Specification of a geometry which stores the type and any attributes along with
 * material parameters
 */
struct GeometrySpecification
{
	GeometryType type;
	MaterialColour materialColour;
	union
	{
		MeshAttributes mesh;
		BoxAttributes box;
		CylinderAttributes cylinder;
		SphereAttributes sphere;
	};
	float originX, originY, originZ;
	float roll, pitch, yaw;
};

/**
 * Each link is defined by its location, inertia parameters, visuals and collision
 * shapes.
 */
class Link
{
public:
	Link() : originX(0), originY(0), originZ(0), roll(0), pitch(0), yaw(0),
			 cogX(0), cogY(0), cogZ(0), mass(1), inertiaProvided(false), parent(nullptr),
			 ixx(2.0/3), ixy(-0.25), ixz(-0.25), iyy(2.0/3), iyz(-0.25), izz(2.0/3) {}

	// Shape definitions
	std::vector<GeometrySpecification> visual;

	// Each link can have multiple geometry components but only 1 material at present
	std::vector<GeometrySpecification> collision;
	MaterialInstance collisionMaterial;

	// Name
	std::string name;

	// Location
	float originX, originY, originZ;
	float roll, pitch, yaw;

	// Center of gravity
	float cogX, cogY, cogZ;

	// Inertia parameters
	float mass;
	float ixx, ixy, ixz, iyy, iyz, izz;		// Defaults for a 1kg, 1m^3 cube
	bool inertiaProvided;

	// Which link is this one's parent (via some joint) - may be null for the root element
	Link* parent;
};

/**
 * Various joint types. Currently only continuous supported
 */
enum JointType
{
	Revolute,		// @TODO: Implement
	Continuous,
	Prismatic,		// @TODO: Implement
	Fixed,			// @TODO: Implement
	Floating,		// @TODO: Implement
	Planar			// @TODO: Implement
};

/**
 * Attributes of a joint.
 */
struct JointDefinition
{
	std::string parent;
	std::string child;
	std::string name;
	JointType type;

	float originX, originY, originZ;
	float originRoll, originPitch, originYaw;

	float axisX, axisY, axisZ;
	float dynamicsDamping, dynamicsFriction;
	float limitLower, limitUpper, limitEffort, limitVelocity;
};

#endif //ARGOS3_BULLET_URDF_STRUCTS_H
