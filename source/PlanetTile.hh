#pragma once

#include <osg/Geode>

class PlanetLOD;

class PlanetTile : public osg::Geode {
public:
	META_Node(osg, PlanetTile);

	PlanetTile(const PlanetLOD& node);

private:
	PlanetTile();
	PlanetTile(const PlanetTile& copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
};