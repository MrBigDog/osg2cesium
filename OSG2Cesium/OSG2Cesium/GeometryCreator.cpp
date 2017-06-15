#include "GeometryCreator.h"

 

GeometryCreator::GeometryCreator()
{

}


GeometryCreator::~GeometryCreator()
{

}

//osg::Node * GeometryCreator::createBox(osg::Vec3 center, osg::Vec3 size, osg::Image* img)
//{
//
//	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
//	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
//	std::vector<osg::Vec3> facenormals;
//	std::vector<osg::Vec3> faceverts;
//	osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
//
//	std::vector<osg::Vec3> x_quad;
//	x_quad.push_back(osg::Vec3(0.5, -0.5, -0.5));
//	x_quad.push_back(osg::Vec3(0.5, -0.5, 0.5));
//	x_quad.push_back(osg::Vec3(0.5, 0.5, 0.5));
//	x_quad.push_back(osg::Vec3(0.5, 0.5, -0.5));
//
//	std::vector<osg::Vec3> y_quad;
//	y_quad.push_back(osg::Vec3(0.5, 0.5, -0.5));
//	y_quad.push_back(osg::Vec3(0.5, 0.5, 0.5));
//	y_quad.push_back(osg::Vec3(-0.5, 0.5, 0.5));
//	y_quad.push_back(osg::Vec3(-0.5, 0.5, -0.5));
//
//	std::vector<osg::Vec3> z_quad;
//	z_quad.push_back(osg::Vec3(-0.5, -0.5, 0.5));
//	z_quad.push_back(osg::Vec3(-0.5, 0.5, 0.5));
//	z_quad.push_back(osg::Vec3(0.5, 0.5, 0.5));
//	z_quad.push_back(osg::Vec3(0.5, -0.5, 0.5));
//
//	for (int i = 0; i < 4; i++)
//	{
//		faceverts.push_back(x_quad[i]);
//	    facenormals.push_back(osg::Vec3(1, 0, 0));
//	}
//
//	for (int i = 3; i >= 0; i--)
//	{
//		faceverts.push_back(x_quad[i] + osg::Vec3(-1, 0, 0));	
//		facenormals.push_back(osg::Vec3(-1, 0, 0));
//	}
//
//
//	for (int i = 0; i < 4; i++)
//	{
//		faceverts.push_back(y_quad[i]);
//	    facenormals.push_back(osg::Vec3(0, 1, 0));
//	}
//
//
//	for (int i = 3; i >= 0; i--)
//	{
//		faceverts.push_back(y_quad[i] + osg::Vec3(0, -1, 0));
//	    facenormals.push_back(osg::Vec3(0, -1, 0));
//	}
//
//	for (int i = 0; i < 4; i++)
//	{
//		faceverts.push_back(z_quad[i]);
//	    facenormals.push_back(osg::Vec3(0, 0, 1));
//	}
//
//	for (int i = 3; i >= 0; i--)
//	{
//		faceverts.push_back(z_quad[i] + osg::Vec3(0, 0, -1));
//	    facenormals.push_back(osg::Vec3(0, 0, -1));
//	}
//
//	for (int i = 0; i < faceverts.size(); i++)
//	{
//		vertices->push_back(center + osg::componentMultiply(faceverts[i], size));
//		normals->push_back(facenormals[i]);
//	}
//
//	for (int i = 0; i < 6; i++)
//	{
//		texcoords->push_back(osg::Vec2(0, 1));
//		texcoords->push_back(osg::Vec2(1, 0));
//		texcoords->push_back(osg::Vec2(1, 1));
//		texcoords->push_back(osg::Vec2(0, 0));
//	}
//	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
//	geom->setVertexArray(vertices.get());
//	geom->setNormalArray(normals.get());
//	geom->setTexCoordArray(0,texcoords.get());
//	geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
//	osg::ref_ptr<osg::Texture2D> tex = new osg::Texture2D;
//	geom->getOrCreateStateSet()->setTextureAttributeAndModes
//		(0, tex.get(), osg::StateAttribute::ON);
//	// Associate this state set with the Geode that contains
//	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, vertices->size()));
//	tex->setImage(img);
//	osg::Geode* geode = new osg::Geode;
//	geode->addChild(geom.get());
//
//	return geode;
//}

osg::Node * GeometryCreator::createBox(osg::Vec3 center, osg::Vec3 size, osg::Image* img)
{

	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
	std::vector<osg::Vec3> facenormals;
	std::vector<osg::Vec3> faceverts;
	osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;

	std::vector<osg::Vec3> x_quad;
	x_quad.push_back(osg::Vec3(0.5, 0.5, 0.5));
	x_quad.push_back(osg::Vec3(0.5, -0.5, 0.5));
	x_quad.push_back(osg::Vec3(0.5, -0.5, -0.5));

	x_quad.push_back(osg::Vec3(0.5, 0.5, -0.5));
	x_quad.push_back(osg::Vec3(0.5, 0.5, 0.5));
	x_quad.push_back(osg::Vec3(0.5, -0.5, -0.5));

	std::vector<osg::Vec3> y_quad;
	y_quad.push_back(osg::Vec3(-0.5, 0.5, 0.5));
	y_quad.push_back(osg::Vec3(0.5, 0.5, 0.5));
	y_quad.push_back(osg::Vec3(0.5, 0.5, -0.5));

	y_quad.push_back(osg::Vec3(-0.5, 0.5, -0.5));
	y_quad.push_back(osg::Vec3(-0.5, 0.5, 0.5));
	y_quad.push_back(osg::Vec3(0.5, 0.5, -0.5));

	std::vector<osg::Vec3> z_quad;
	z_quad.push_back(osg::Vec3(0.5, 0.5, 0.5));
	z_quad.push_back(osg::Vec3(-0.5, 0.5, 0.5));
	z_quad.push_back(osg::Vec3(-0.5, -0.5, 0.5));

	z_quad.push_back(osg::Vec3(0.5, -0.5, 0.5));
	z_quad.push_back(osg::Vec3(0.5, 0.5, 0.5));
	z_quad.push_back(osg::Vec3(-0.5, -0.5, 0.5));

	for (int i = 0; i < 6; i++)
	{
		faceverts.push_back(x_quad[i]);
		facenormals.push_back(osg::Vec3(1, 0, 0));
	}

	for (int i = 5; i >= 0; i--)
	{
		faceverts.push_back(x_quad[i] + osg::Vec3(-1, 0, 0));
		facenormals.push_back(osg::Vec3(-1, 0, 0));
	}


	for (int i = 0; i < 6; i++)
	{
		faceverts.push_back(y_quad[i]);
		facenormals.push_back(osg::Vec3(0, 1, 0));
	}


	for (int i = 5; i >= 0; i--)
	{
		faceverts.push_back(y_quad[i] + osg::Vec3(0, -1, 0));
		facenormals.push_back(osg::Vec3(0, -1, 0));
	}

	for (int i = 0; i < 6; i++)
	{
		faceverts.push_back(z_quad[i]);
		facenormals.push_back(osg::Vec3(0, 0, 1));
	}

	for (int i = 5; i >= 0; i--)
	{
		faceverts.push_back(z_quad[i] + osg::Vec3(0, 0, -1));
		facenormals.push_back(osg::Vec3(0, 0, -1));
	}

	for (int i = 0; i < faceverts.size(); i++)
	{
		vertices->push_back(center + osg::componentMultiply(faceverts[i], size));
		normals->push_back(facenormals[i]);
	}

	for (int i = 0; i < 6; i++)
	{
		texcoords->push_back(osg::Vec2(0, 1));
		texcoords->push_back(osg::Vec2(1, 0));
		texcoords->push_back(osg::Vec2(1, 1));

		texcoords->push_back(osg::Vec2(0, 1));
		texcoords->push_back(osg::Vec2(1, 1));
		texcoords->push_back(osg::Vec2(0, 0));
	}
	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
	geom->setVertexArray(vertices.get());
	geom->setNormalArray(normals.get());
	geom->setTexCoordArray(0, texcoords.get());
	geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	osg::ref_ptr<osg::Texture2D> tex = new osg::Texture2D;
	geom->getOrCreateStateSet()->setTextureAttributeAndModes
		(0, tex.get(), osg::StateAttribute::ON);
	// Associate this state set with the Geode that contains
	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, vertices->size()));
	tex->setImage(img);
	osg::Geode* geode = new osg::Geode;
	geode->addChild(geom.get());

	return geode;
}
#include "osgDB/ReadFile"
osg::Geometry * GeometryCreator::createBox(osg::Vec3 center, osg::Vec3 size, osg::Vec3 color)
{
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> colors = new osg::Vec3Array;
	std::vector<osg::Vec3> facenormals;
	std::vector<osg::Vec3> faceverts;

	std::vector<osg::Vec3> x_quad;
	x_quad.push_back(osg::Vec3(0.5, 0.5, 0.5));
	x_quad.push_back(osg::Vec3(0.5, -0.5, 0.5));
	x_quad.push_back(osg::Vec3(0.5, -0.5, -0.5));

	x_quad.push_back(osg::Vec3(0.5, 0.5, -0.5));
	x_quad.push_back(osg::Vec3(0.5, 0.5, 0.5));
	x_quad.push_back(osg::Vec3(0.5, -0.5, -0.5));

	std::vector<osg::Vec3> y_quad;
	y_quad.push_back(osg::Vec3(-0.5, 0.5, 0.5));
	y_quad.push_back(osg::Vec3(0.5, 0.5, 0.5));
	y_quad.push_back(osg::Vec3(0.5, 0.5, -0.5));

	y_quad.push_back(osg::Vec3(-0.5, 0.5, -0.5));
	y_quad.push_back(osg::Vec3(-0.5, 0.5, 0.5));
	y_quad.push_back(osg::Vec3(0.5, 0.5, -0.5));

	std::vector<osg::Vec3> z_quad;
	z_quad.push_back(osg::Vec3(0.5, 0.5, 0.5));
	z_quad.push_back(osg::Vec3(-0.5, 0.5, 0.5));
	z_quad.push_back(osg::Vec3(-0.5, -0.5, 0.5));

	z_quad.push_back(osg::Vec3(0.5, -0.5, 0.5));
	z_quad.push_back(osg::Vec3(0.5, 0.5, 0.5));
	z_quad.push_back(osg::Vec3(-0.5, -0.5, 0.5));

	for (int i = 0; i < 6; i++)
	{
		faceverts.push_back(x_quad[i]);
		facenormals.push_back(osg::Vec3(1, 0, 0));
	}

	for (int i = 5; i >= 0; i--)
	{
		faceverts.push_back(x_quad[i] + osg::Vec3(-1, 0, 0));
		facenormals.push_back(osg::Vec3(-1, 0, 0));
	}


	for (int i = 0; i < 6; i++)
	{
		faceverts.push_back(y_quad[i]);
		facenormals.push_back(osg::Vec3(0, 1, 0));
	}


	for (int i = 5; i >= 0; i--)
	{
		faceverts.push_back(y_quad[i] + osg::Vec3(0, -1, 0));
		facenormals.push_back(osg::Vec3(0, -1, 0));
	}

	for (int i = 0; i < 6; i++)
	{
		faceverts.push_back(z_quad[i]);
		facenormals.push_back(osg::Vec3(0, 0, 1));
	}

	for (int i = 5; i >= 0; i--)
	{
		faceverts.push_back(z_quad[i] + osg::Vec3(0, 0, -1));
		facenormals.push_back(osg::Vec3(0, 0, -1));
	}

	for (int i = 0; i < faceverts.size(); i++)
	{
		vertices->push_back(center + osg::componentMultiply(faceverts[i], size));
		normals->push_back(facenormals[i]);
		colors->push_back(color);
	}
	osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
	for (int i = 0; i < 6; i++)
	{
		texcoords->push_back(osg::Vec2(0, 1));
		texcoords->push_back(osg::Vec2(1, 0));
		texcoords->push_back(osg::Vec2(1, 1));

		texcoords->push_back(osg::Vec2(0, 1));
		texcoords->push_back(osg::Vec2(1, 1));
		texcoords->push_back(osg::Vec2(0, 0));
	}


	osg::Geometry* geom = new osg::Geometry;
	geom->setVertexArray(vertices.get());
	geom->setTexCoordArray(0, texcoords.get());
	geom->setNormalArray(normals.get());
	geom->setColorArray(colors.get());
	geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	osg::ref_ptr<osg::Texture2D> tex = new osg::Texture2D;
	tex->setImage(osgDB::readImageFile("B:/cesium-3d-tiles/Specs/Data/Cesium3DTiles/book_texture.jpg"));
	geom->getOrCreateStateSet()->setTextureAttributeAndModes
		(0, tex.get(), osg::StateAttribute::ON);

	// Associate this state set with the Geode that contains
	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, vertices->size()));


	return geom;
}
#include <sstream>
std::string createDest(std::string name1, std::string name2, int geomid, int start, int end)
{
	std::stringstream ss;
	ss << name1 << "," << name2 << "," << geomid << "," << start << "," << end;
	return ss.str();
}
osg::Geode* GeometryCreator::createBoxArray(float size)
{
	int nrows = 2;
	int ncols = 2;
	osg::Vec3 origin(0,0,0);
	double half = size * 0.5;
	osg::Geode* geode = new osg::Geode;
	geode->addChild(createBox(osg::Vec3(-half, half, 0), osg::Vec3(size, size, size), osg::Vec3(1, 0, 0)));
	geode->addChild(createBox(osg::Vec3(half, half, 0), osg::Vec3(size, size, size), osg::Vec3(0, 1, 0)));
	geode->addChild(createBox(osg::Vec3(half, -half, 0), osg::Vec3(size, size, size), osg::Vec3(0, 0, 1)));
	geode->addChild(createBox(osg::Vec3(-half, -half, 0), osg::Vec3(size, size, size), osg::Vec3(1, 1, 0)));
	geode->addDescription(createDest("box array", "box1", 0, 0, 35));
	geode->addDescription(createDest("box array", "box2", 1, 0, 35));
	geode->addDescription(createDest("box array", "box3", 2, 0, 35));
	geode->addDescription(createDest("box array", "box4", 3, 0, 35));
	geode->getChild(0)->setName("box1");
	geode->getChild(1)->setName("box2");
	geode->getChild(2)->setName("box3");
	geode->getChild(3)->setName("box4");
	geode->setName("ljm");
	return geode;
}