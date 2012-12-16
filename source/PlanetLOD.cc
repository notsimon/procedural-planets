#include "PlanetLOD.hh"
#include "Planet.hh"
#include "PlanetTile.hh"

#include <osgUtil/CullVisitor>

#include <iostream>
#include <stack>
#include <cassert>
#include <climits>

void PlanetLOD::GenerateQuadtree(Planet& planet) {
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

	PlanetLOD* trees[6];
	for (uchar i = 0; i < 6; i++)
		trees[i] = new PlanetLOD(i);

	const uchar neighborhood[6][4] = {
	   // W, E, N, S
		{ 3, 2, 5, 4 },
		{ 3, 2, 4, 5 },
		{ 4, 5, 0, 1 },
		{ 5, 4, 0, 1 },
		{ 3, 2, 0, 1 },
		{ 2, 3, 0, 1 } 
	};

	for (uchar i = 0; i < 6; i++) {
		PlanetLOD* node = trees[i];

		// link to neighbors
		for (int n = 0; n < 4; n++)
			node->_neighbors[n] = trees[neighborhood[node->_side][n]];

		// add root to the planet's childs
		planet.addChild(node);
	}

	trees[0]->split();
	trees[0]->merge();
}

PlanetLOD::PlanetLOD(uchar side)
	: osg::Group(),
	  _side(side),
	  _depth(0),
	  _origin(0, 0),
	  _center(projectOnSphere(0, 0)),
	  _tile(new PlanetTile(*this, DefaultResolution)),
	  _parent(0) {
	for (int i = 0; i < 4; i++) {
		_children[i] = 0;
		_neighbors[i] = 0;
	}
}

PlanetLOD::PlanetLOD(PlanetLOD* parent, const Vec2f origin)
	: osg::Group(),
	  _side(parent->_side),
	  _depth(parent->_depth + 1),
	  _origin(origin),
	  _center(projectOnSphere(origin.x(), origin.y())),
	  _tile(new PlanetTile(*this, DefaultResolution)),
	  _parent(parent) {
	for (int i = 0; i < 4; i++) {
		_children[i] = 0;
		_neighbors[i] = 0;
	}
}

PlanetLOD::PlanetLOD()
	: osg::Group(),
	  _side(UCHAR_MAX),
	  _depth(0),
	  _origin(0, 0),
	  _center(projectOnSphere(0, 0)),
	  _parent(0) {
	for (int i = 0; i < 4; i++) {
		_children[i] = 0;
		_neighbors[i] = 0;
	}
}

PlanetLOD::PlanetLOD(const PlanetLOD& copy, const osg::CopyOp& copyop)
	: osg::Group(copy, copyop),
	  _side(copy._side),
	  _depth(copy._depth),
	  _origin(copy._origin),
	  _center(copy._center),
	  _parent(0) {
	for (int i = 0; i < 4; i++) {
		_children[i] = 0;
		_neighbors[i] = 0;
	}
}

#define PATCH_LOD_DISTANCE 2
#define PATCH_MAX_LOD 8

void PlanetLOD::traverse(osg::NodeVisitor& nv) {
	osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
	if (cv) {
		float dist = cv->getDistanceToViewPoint(_center, true);
		if (isLeaf()) { // leaf node
			if (_depth < PATCH_MAX_LOD // maximal depth
			&& dist < PATCH_LOD_DISTANCE * size() /** radius_*/) { // too close, must be broken!
				//std::cout << "split (" << dist << ") : " << &node << std::endl;
				split();

				//for (Quadtree* child : node.childs)
		        //    addPatch(*child);

		        //removePatch(node);
			}
		}
		else {
			if (dist > PATCH_LOD_DISTANCE * size()/* * radius_*/) { // to far, must be merged
				/*for (int c = 0; c < 4; c++) {
					DepthTraversal traversal(neib);
					PlanetLOD* node;
					while ((node = traversal.next()))
				}
				for (Quadtree* child : node.childs)
					child->depthTraversal ([=] (Quadtree& node) {
						removePatch(node);
					});*/

				merge();
				//addPatch(node);
			}
		}
	}

	// continue propagation

	if (isLeaf())
		_tile->accept(nv);
	else
		for (int i = 0; i < 4; i++)
			_children[i]->accept(nv);

	osg::Group::traverse(nv);
}

enum Quadrant {
	NW = 0,
	NE,
	SW,
	SE,
	Na = 255,
};

void PlanetLOD::split() {
	assert(isLeaf());

	// child nodes
	float quarter = size() / 4;

	_children[NW] = new PlanetLOD(
							this,
							_origin + Vec2f(-quarter, quarter)
						);
	
	_children[NE] = new PlanetLOD(
							this,
							_origin + Vec2f(quarter, quarter)
						);

	_children[SW] = new PlanetLOD(
							this,
							_origin + Vec2f(-quarter, -quarter)
						);

	_children[SE] = new PlanetLOD(
							this,
							_origin + Vec2f(quarter, -quarter)
						);

	// neiborhood inside the branch

	static const Quadrant inner[4][4] = {
			/* W,   E,   N,   S  */
	/* NW */ { Na,  NE,  Na,  SW },
	/* NE */ { NW,  Na,  Na,  SE },
	/* SW */ { Na,  SE,  NW,  Na },
	/* SE */ { SW,  Na,  NE,  Na },
	};

	for (int c = 0; c < 4; c++)
		for (int d = 0; d < 4; d++) {
			if (inner[c][d] == Na) continue;
			_children[c]->_neighbors[d] = _children[inner[c][d]];
		}


	// neighbors from different branches

	static const Quadrant outer_reg[4][4] = {
	   		/* W,   E,   N,   S  */
	/* NW */ { NE,  Na,  SW,  Na },
	/* NE */ { Na,  NW,  SE,  Na },
	/* SW */ { SE,  Na,  Na,  NW },
	/* SE */ { Na,  SW,  Na,  NE },
	};

	static const Quadrant outer_sided[6][4][4] = {
		{ /* SIDE 0 */
		   		/* W,   E,   N,   S  */
		/* NW */ { NW,  Na,  NE,  Na },
		/* NE */ { Na,  NE,  NW,  Na },
		/* SW */ { NE,  Na,  Na,  NW },
		/* SE */ { Na,  NW,  Na,  NE },
		},
		{ /* SIDE 1 */
		   		/* W,   E,   N,   S  */
		/* NW */ { SE,  Na,  SW,  Na },
		/* NE */ { Na,  SW,  SE,  Na },
		/* SW */ { SW,  Na,  Na,  SE },
		/* SE */ { Na,  SE,  Na,  SW },
		},
		{ /* SIDE 2 */
		   		/* W,   E,   N,   S  */
		/* NW */ { NE,  Na,  SE,  Na },
		/* NE */ { Na,  NW,  NE,  Na },
		/* SW */ { SE,  Na,  Na,  NE },
		/* SE */ { Na,  SW,  Na,  SE },
		},
		{ /* SIDE 3 */
		   		/* W,   E,   N,   S  */
		/* NW */ { NE,  Na,  NW,  Na },
		/* NE */ { Na,  NW,  SW,  Na },
		/* SW */ { SE,  Na,  Na,  SW },
		/* SE */ { Na,  SW,  Na,  NW },
		},
		{ /* SIDE 4 */
		   		/* W,   E,   N,   S  */
		/* NW */ { NE,  Na,  SW,  Na },
		/* NE */ { Na,  NW,  SE,  Na },
		/* SW */ { SE,  Na,  Na,  NW },
		/* SE */ { Na,  SW,  Na,  NE },
		},
		{ /* SIDE 5 */
		   		/* W,   E,   N,   S  */
		/* NW */ { NE,  Na,  NE,  Na },
		/* NE */ { Na,  NW,  NW,  Na },
		/* SW */ { SE,  Na,  Na,  SE },
		/* SE */ { Na,  SW,  Na,  SW },
		}
	};

	for (int c = 0; c < 4; c++) { // for every child
		for (int d = 0; d < 4; d++) { // for every direction
			if (outer_reg[c][d] == Na) continue; // Naat the same place in both 'outers' tables

			if (_neighbors[d]->isLeaf())
				_children[c]->_neighbors[d] = _neighbors[d];
			else { // associate with a node of the same depth
				if (_neighbors[d]->_side == _side) // same cube side
					_children[c]->_neighbors[d] = _neighbors[d]->_children[outer_reg[c][d]];
				else // sides MUST be taken into account
					_children[c]->_neighbors[d] = _neighbors[d]->_children[outer_sided[_side][c][d]];
				
				// relink neighbor's neighbors
				DepthTraversal traversal(_children[c]->_neighbors[d]);
				PlanetLOD* node;
				while ((node = traversal.next())) {
					for (int n = 0; n < 4; n++) {
						if (node->_neighbors[n] == this)
							node->_neighbors[n] = _children[c];
					}
				}
			}
		}
	}
}

void PlanetLOD::merge () {
	assert(!isLeaf());
    
    // Merge descendants
    for (int c = 0; c < 4; c++) {
        assert(_children[c] != 0);
        
        if (!_children[c]->isLeaf())
        	_children[c]->merge();

        assert(_children[c]->isLeaf());
    }
    
    // Update neighbors links
	// browse every childs' neibs and set 'this' as
    // neighbor if required 

    for (int n = 0; n < 4; n++) {
    	PlanetLOD* neib = _neighbors[n];
    	if (neib != 0) {
			DepthTraversal traversal(neib);
			PlanetLOD* node;
			while ((node = traversal.next()))
				for (int n = 0; n < 4; n++)
					for (int c = 0; c < 4; c++)
						if (node->_neighbors[n] == _children[c].get())
							node->_neighbors[n] = this;
    	}
    }

	for (int c = 0; c < 4; c++) {
		_children[c] = 0;
	}
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

#if 0
void PlanetLOD::depthTraversal (std::function<void (PlanetLOD&)> pre) {
    std::stack<PlanetLOD*> stack;
    stack.push(this);

    while (!stack.empty()) {
    	PlanetLOD* node = stack.top();
		stack.pop();

	    pre(*node);
	    
	    if (!node->isLeaf())
	        for (PlanetLOD* child : node->_children) {
	        	assert(child != 0);
	            stack.push(child);
	        }
    }
}
#endif

PlanetLOD::DepthTraversal::DepthTraversal(PlanetLOD* node) {
	_stack.push(node);
}

PlanetLOD* PlanetLOD::DepthTraversal::next() {
	if (_stack.empty())
		return 0;

	PlanetLOD* node = _stack.top();
	_stack.pop();

	if (!node->isLeaf())
		for (int i = 0; i < 4; i++) {
			assert(node->_children[i] != 0);
			_stack.push(node->_children[i]);
		}

	return node;
}