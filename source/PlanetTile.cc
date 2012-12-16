#include "PlanetTile.hh"
#include "common.hh"
#include "PlanetLOD.hh"

#include <osg/Geometry>
#include <osg/PrimitiveSet>
#include <osgUtil/SmoothingVisitor>

#include <iostream>

PlanetTile::PlanetTile()
	: osg::Geode() {
}

PlanetTile::PlanetTile(const PlanetLOD& node, const uint resolution)
	: osg::Geode() {

	// elements
	osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_STRIP);
	indices->reserveElements(6 * (resolution - 1) * (resolution - 1));
	for (uint i = 0; i < resolution - 1; i++)
		for (uint j = 0; j < resolution - 1; j++) {
			indices->addElement(i * resolution + j);
			indices->addElement(i * resolution + j + 1);
			indices->addElement((i + 1) * resolution + j);
			
			indices->addElement(i * resolution + j + 1);
			indices->addElement((i + 1) * resolution + j + 1);
			indices->addElement((i + 1) * resolution + j);
		}

	// vertices positions
	osg::ref_ptr<osg::Vec3Array> positions = new osg::Vec3Array(resolution * resolution);
	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(resolution * resolution);

	const float step = (2.0 / (resolution - 1));
	float x, y;
	x = -1.0;
	for (uint i = 0; i < resolution; i++, x += step) {
		y = -1.0;

		for (uint j = 0; j < resolution; j++, y += step) {
			const uint idx = i * resolution + j;
			
			// position
			positions->at(idx) = node.projectOnSphere(x, y);
			
			// normal
			normals->at(idx) = positions->at(idx);
			normals->at(idx).normalize();
		}
	}

	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	geometry->setVertexArray(positions.get());
	geometry->setNormalArray(normals.get());
	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	geometry->addPrimitiveSet(indices.get());

	addDrawable(geometry.get());
}

PlanetTile::PlanetTile(const PlanetTile& copy, const osg::CopyOp& copyop)
	: osg::Geode(copy, copyop) {
}