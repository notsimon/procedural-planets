#pragma once

#include "common.hh"

#include <osg/Group>
#include <osg/NodeVisitor>

class Planet;
class PlanetTile;

class PlanetLOD : public osg::Group {
public:
	static const int DefaultResolution = 8;
	static void GenerateQuadtree(Planet& planet);

	META_Node(osg, PlanetLOD);

	PlanetLOD(uchar side);
	PlanetLOD(PlanetLOD* parent, const Vec2f origin);

	virtual void traverse(osg::NodeVisitor& nv);

	/**
	 * project a point to the sphere segment associated to the node
	 * @param x lives in [-1, 1]
	 * @param y lives in [-1, 1]
	 */
	Vec3f projectOnSphere(float x, float y) const;

	float size() const { return 2.0 / (1 << _depth); };
	bool isLeaf() const { return _children[0] == 0; };

protected:
	const uchar _side;
	const Vec2f _origin;
	const uint _depth;
	osg::ref_ptr<PlanetTile> _tile;
	osg::ref_ptr<PlanetLOD> _parent;
	osg::ref_ptr<PlanetLOD> _children[4];
	osg::ref_ptr<PlanetLOD> _neighbors[4];

private:
	PlanetLOD();
	PlanetLOD(const PlanetLOD& copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
};