#pragma once

#include "common.hh"

#include <osg/Group>
#include <osg/NodeVisitor>

#include <stack>

class Planet;
class PlanetTile;

class PlanetLOD : public osg::Group {
public:
	static const uint DefaultResolution = 16;
	static void GenerateQuadtree(Planet& planet);

	META_Node(osg, PlanetLOD);

	PlanetLOD(uchar side);
	PlanetLOD(PlanetLOD* parent, const Vec2f origin);

	virtual void traverse(osg::NodeVisitor& nv);

	//~PlanetLOD() {
		// TODO delete childs and verify neighbors
	//}

	/**
 	 * Split a leaf-node into four
 	 */
	void split();

	/**
	 * Transform an inner-node into a leaf-node by merging its
	 * descendants
	 */
	void merge();

	/**
	 * project a point to the sphere segment associated to the node
	 * @param x lives in [-1, 1]
	 * @param y lives in [-1, 1]
	 */
	Vec3f projectOnSphere(float x, float y) const;
	//void depthTraversal (std::function<void (PlanetLOD&)> pre);

	float size() const { return 2.0 / (1 << _depth); };
	bool isLeaf() const { return _children[0] == 0; };

	class DepthTraversal {
	public:
		DepthTraversal(PlanetLOD* node);
		PlanetLOD* next();

	private:
		std::stack<PlanetLOD*> _stack;
	};

protected:
	const uchar _side;
	const uint _depth;
	const Vec2f _origin;
	const Vec3f _center;
	osg::ref_ptr<PlanetTile> _tile;
	PlanetLOD* _parent;
	osg::ref_ptr<PlanetLOD> _children[4];
	PlanetLOD* _neighbors[4];

private:
	PlanetLOD();
	PlanetLOD(const PlanetLOD& copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
};