#include "Planet.hh"
#include "PlanetLOD.hh"

Planet::Planet()
	: osg::Group() {
	PlanetLOD::GenerateQuadtree(*this);
}

Planet::Planet(const Planet& copy, const osg::CopyOp& copyop)
	: osg::Group(copy, copyop) {
}