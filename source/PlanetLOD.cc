#include "PlanetLOD.hh"
#include "Planet.hh"
#include "PlanetTile.hh"

void PlanetLOD::GenerateQuadtree(Planet& planet) {
	PlanetLOD* roots[6];

	// QUADTREE INTIALIZATION
	//
	// Cube's faces convention:
	//    *---------*
	//   /|   0    /|  n is front-face
	//  / |       / | [n] is a backface
	// *---------*  |
	// |  |  [5] |  |
	// |[3]      | 2|   y          
	// |  * -4- -|- *   ^  z
	// | /       | /    | /
	// |/   [1]  |/     |/
	// *---------*      z--->x
	//
	// Neighbors links depends on the faces orientation used

	for (uchar i = 0; i < 6; i++)
		roots[i] = new PlanetLOD(i);

	/*const uchar neighborhood[6][4] = {
	   // W, E, N, S
		{ 3, 2, 5, 4 },
		{ 3, 2, 4, 5 },
		{ 4, 5, 0, 1 },
		{ 5, 4, 0, 1 },
		{ 3, 2, 0, 1 },
		{ 2, 3, 0, 1 } 
	};*/

	// Roots linking and initial patches creation
	//for (auto& tree : _quadtree) {
	//	for (int n = 0; n < 4; n++)
	//		tree._neighbors[n] = &_quadtree[neighborhood[tree.getSide()][n]];
	//}

	for (uchar i = 0; i < 6; i++) {
		roots[i]->addChild(new PlanetTile(*roots[i]));
		planet.addChild(roots[i]);
	}

	// TODO : check roots ref counters
}

PlanetLOD::PlanetLOD(uchar side)
	: osg::Group(),
	  _parent(0),
	  _side(side),
	  _origin(0, 0),
	  _depth(0) {
}

PlanetLOD::PlanetLOD(PlanetLOD* parent, const Vec2f origin)
	: osg::Group(),
	  _parent(parent),
	  _side(parent->_side),
	  _origin(origin),
	  _depth(parent->_depth + 1) {
}

PlanetLOD::PlanetLOD()
	: osg::Group(),
	  _parent(0),
	  _side(UCHAR_MAX),
	  _origin(0, 0),
	  _depth(0) {
}

PlanetLOD::PlanetLOD(const PlanetLOD& copy, const osg::CopyOp& copyop)
	: osg::Group(copy, copyop),
	  _parent(copy._parent),
	  _side(copy._side),
	  _origin(copy._origin),
	  _depth(copy._depth) {
}

Vec3f PlanetLOD::projectOnSphere(float x, float y) const {
	static const Matrix4f p_matrices[] = {
		//        x-axis,        y-axis,        origin_
		Matrix4f( 1, 0,  0, 0,   0, 0,  1, 0,   0,  1,  0, 0,   0, 0, 0, 0),
		Matrix4f( 1, 0,  0, 0,   0, 0, -1, 0,   0, -1,  0, 0,   0, 0, 0, 0),
		Matrix4f( 0, 0,  1, 0,   0, 1,  0, 0,   1,  0,  0, 0,   0, 0, 0, 0),
		Matrix4f( 0, 0, -1, 0,   0, 1,  0, 0,  -1,  0,  0, 0,   0, 0, 0, 0),
		Matrix4f( 1, 0,  0, 0,   0, 1,  0, 0,   0,  0, -1, 0,   0, 0, 0, 0),
		Matrix4f(-1, 0,  0, 0,   0, 1,  0, 0,   0,  0,  1, 0,   0, 0, 0, 0)
	};

	x *= size() / 2;
	y *= size() / 2;
	Vec4f r = p_matrices[_side] * Vec4f(_origin.x() + x, _origin.y() + y, 1, 0);

	return Vec3f( // mathproofs.blogspot.fr fair distribution
		r.x() * sqrtf(1 - r.y()*r.y() / 2 - r.z()*r.z() / 2 + r.y()*r.y() * r.z()*r.z() / 3),
		r.y() * sqrtf(1 - r.z()*r.z() / 2 - r.x()*r.x() / 2 + r.z()*r.z() * r.x()*r.x() / 3),
		r.z() * sqrtf(1 - r.x()*r.x() / 2 - r.y()*r.y() / 2 + r.x()*r.x() * r.y()*r.y() / 3)
	);
}