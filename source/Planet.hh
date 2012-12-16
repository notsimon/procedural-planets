#pragma once

#include <osg/Group>

class Planet : public osg::Group {
public:
	META_Node(osg, Planet);

	Planet();
	Planet(const Planet& copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
};