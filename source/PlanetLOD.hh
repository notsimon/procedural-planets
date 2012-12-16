#pragma once

#include "common.hh"

#include <osg/Group>

class Planet;

class PlanetLOD : public osg::Group {
public:
	static void GenerateQuadtree(Planet& planet);

	META_Node(osg, PlanetLOD);

	PlanetLOD(uchar side);
	PlanetLOD(PlanetLOD* parent, const Vec2f origin);

	Vec3f projectOnSphere(float x, float y) const;

	float size() const { return 2.0 / (1 << _depth); };

protected:
	osg::ref_ptr<PlanetLOD> _parent;
	const uchar _side;
	const Vec2f _origin;
	const uint _depth;

private:
	PlanetLOD();
	PlanetLOD(const PlanetLOD& copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
};