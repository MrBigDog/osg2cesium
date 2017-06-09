#pragma once
#include <Windows.h>
#include <osg/NodeVisitor>
#include <osg/Matrix>
#include <osg/Geometry>
#include <osg/Transform>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgUtil/Export>
#include <map>
#include <set>
#include <osg/Geode>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osg/PagedLOD>
#include <osg/ComputeBoundsVisitor>
#include "json/json.h"
#include <string>
#include <sstream>
#include "OSG2GLTF.h"
#include "EllipsoidModel.h"
//带PagedLOD的OSG场景节点到3D Tile的转换类
class PagedLOD2Tiles
{
public:
	//是否从Z轴朝上翻转为Y朝上，默认为TRUE
	bool m_flipAxis;
	//模型局部坐标系进行变换的矩阵
	osg::Matrix m_localTransform;
	PagedLOD2Tiles();
	~PagedLOD2Tiles();
	//把带PagedLOD的osg节点文件转换为3D Tile
	//outdir:输出文件夹路径
	void toTileSet(std::string pagelodfile,  std::string outdir);
	//把带PagedLOD的osg节点转换为3D Tile
	//outdir:输出文件夹路径
	void toTileSet(osg::Node* node, std::string outdir);
	//把带PagedLOD的osg节点转换为3D Tile。group节点本身不含几何内容，但包含若干PagedLOD子节点，因此group将变成一个没有content的3D Tile节点。
	//outdir:输出文件夹路径
	void toTileSet(osg::Group* group, std::string outdir);
	//设置地理坐标和局部转换矩阵
	//lat:纬度
	//lon:经度
	//height:高度 （可能是绝对海拔高度）
	//localTransform：模型局部坐标系进行变换的矩阵
	void setTransform(double lat,double lon, double height,osg::Matrix localTransform = osg::Matrix::identity());

private:
	osg::CesiumEllipsoidModel m_ellipsoidModel;
	osg::Matrixd m_local2world;
	osg::Vec3d m_latlonheight;
private:
	Json::Value createTransformNode(osg::Matrixd& mat);
	Json::Value createBoxNode(osg::BoundingBox bb);
	Json::Value createRegionNode(osg::BoundingBoxd localbb);
	osg::BoundingBox flip(osg::BoundingBox bb);
	//计算3D Tile的Geometric error参数值
	//目前只支持PIXEL_SIZE_ON_SCREEN模式的range
	float calGeometricError(float radius, float screenPixels);
	void findPagedLOD(osg::Node* parent, std::vector<osg::PagedLOD*>& pagedLODList);
	int createNode(std::string filename,osg::Node* node2, std::string outname,std::string outdir, Json::Value& newJsonNode, Json::Value& parentJsonNode,float geometricError, float& geometricErrorOfFirstLevel);
};


