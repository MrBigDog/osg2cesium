#pragma once
#include <Windows.h>
#include <osg/Array>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Image>
#include <osg/Texture2D>
#include <osg/Texture>
//测试模型生成类
class GeometryCreator
{
public:
	GeometryCreator();
	~GeometryCreator();
	static osg::Node* createBox(osg::Vec3 center, osg::Vec3 size, osg::Image* img);
	static osg::Geometry* createBox(osg::Vec3 center, osg::Vec3 size, osg::Vec3 color);
	static osg::Geode* createBoxArray(float size);
};

