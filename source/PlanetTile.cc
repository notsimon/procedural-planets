#include "PlanetTile.hh"
#include "common.hh"
#include "PlanetLOD.hh"

#include <osg/Geometry>
#include <osg/PrimitiveSet>

PlanetTile::PlanetTile()
	: osg::Geode() {
}

PlanetTile::PlanetTile(const PlanetLOD& node)
	: osg::Geode() {

	// elements
	osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);
	indices->reserveElements(6 * (PATCH_GRID_SIZE - 1) * (PATCH_GRID_SIZE - 1));
	for (int i = 0; i < PATCH_GRID_SIZE - 1; i++)
		for (int j = 0; j < PATCH_GRID_SIZE - 1; j++) {
			indices->addElement(i * PATCH_GRID_SIZE + j);
			indices->addElement(i * PATCH_GRID_SIZE + j + 1);
			indices->addElement((i + 1) * PATCH_GRID_SIZE + j);
			
			indices->addElement(i * PATCH_GRID_SIZE + j + 1);
			indices->addElement((i + 1) * PATCH_GRID_SIZE + j + 1);
			indices->addElement((i + 1) * PATCH_GRID_SIZE + j);
		}

	// vertices positions
	osg::ref_ptr<osg::Vec3Array> positions = new osg::Vec3Array(PATCH_GRID_SIZE * PATCH_GRID_SIZE);
	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(PATCH_GRID_SIZE * PATCH_GRID_SIZE);

	const float step = (2.0 / (PATCH_GRID_SIZE - 1));
	float x, y;
	x = -1.0;
	for (uint i = 0; i < PATCH_GRID_SIZE; i++, x += step) {
		y = -1.0;

		for (uint j = 0; j < PATCH_GRID_SIZE; j++, y += step) {
			const uint idx = i * PATCH_GRID_SIZE + j;
			
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
	//osgUtil::SmoothingVisitor::smooth(*geometry);

	addDrawable(geometry.get());
}

PlanetTile::PlanetTile(const PlanetTile& copy, const osg::CopyOp& copyop)
	: osg::Geode(copy, copyop) {
}