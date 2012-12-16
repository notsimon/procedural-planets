/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "common.hh"

#include "Planet.hh"

#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

int main (int argc, char** argv) {
	//osg::setNotifyLevel(osg::INFO);

	osg::ArgumentParser args (&argc, argv);
	
	osgViewer::Viewer viewer;
	viewer.setUpViewInWindow((1280 - 800) / 2, (800 - 600) / 2, 800, 600);

	osg::ref_ptr<osg::Group> root = new osg::Group;

	//std::string filename = "cessna.osg";
	//args.read("--model", filename);
	//osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(filename);
	//osg::ref_ptr<osg::MatrixTransform> m1 = new osg::MatrixTransform;
	//m1->setMatrix(osg::Matrix::translate(-25, 0, 0));
	//m1->addChild(model);
	
	osg::ref_ptr<Planet> planet = new Planet();
	root->addChild(planet);

	viewer.setSceneData(root.get());

	return viewer.run();
}