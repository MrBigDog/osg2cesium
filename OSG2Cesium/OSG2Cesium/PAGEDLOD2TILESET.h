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
	//模型局部坐标系进行变换的矩阵
	osg::Matrix m_localTransform;
	PagedLOD2Tiles();
	~PagedLOD2Tiles();
	//把带PagedLOD的osg节点转换为3D Tile
	//outdir:输出文件夹路径
	void toTileSet(osg::Node* node, std::string outdir);
	//把带包含PagedLOD文件夹转换为3D Tile。
	//indir是包含PagedLOD文件夹路径
	//indir包含若干子文件夹，每个子文件夹下包含一个以与文件夹同名的PagedLOD节点文件(后缀为.osgb)
	//outdir:输出文件夹路径
	void toTileSet(std::string indir, std::string outdir);
	//设置地理坐标和局部转换矩阵
	//lat:纬度
	//lon:经度
	//height:高度 （可能是绝对海拔高度）
	//localTransform：局部坐标系进行变换的矩阵,可以对3D Tiles进行整体的缩放平移旋转,例如setTransform(50.1979, 127.59859, 60, osg::Matrix::scale(0.3048, 0.3048, 0.3048) * osg::Matrix::translate(0,0,500));
	void setTransform(double lat,double lon, double height,osg::Matrixd localTransform = osg::Matrixd::identity());
	////是否从Z轴朝上翻转为Y朝上，默认为TRUE
	//void setFlipAxis(bool flip);
	osg::Matrixd getTransform();
private:
	////是否从Z轴朝上翻转为Y朝上，默认为TRUE
	//bool m_flipAxis;
	//osg::Matrixd m_flipAxisTransform;
	osg::CesiumEllipsoidModel m_ellipsoidModel;
	osg::Matrixd m_local2world;
	osg::Vec3d m_latlonheight;
	std::string m_outputRoot;
private:
	Json::Value createTransformNode(osg::Matrixd& mat);
	Json::Value createBoxNode(osg::BoundingBox bb);
	Json::Value createRegionNode(osg::BoundingBoxd localbb);
	osg::BoundingBox flip(osg::BoundingBox bb);
	//计算3D Tile的Geometric error参数值
	//目前只支持PIXEL_SIZE_ON_SCREEN模式的range
	float calGeometricError(float radius, float screenPixels);
	void findPagedLOD(osg::Node* parent, std::vector<osg::PagedLOD*>& pagedLODList);
	int createNode(std::string filename,osg::Node* node2, std::string outname,std::string outdir, Json::Value& newJsonNode, Json::Value& parentJsonNode,float geometricError, float& maxGeometricError);
	void toTileSet(osg::Node* node, std::string outdir, float& maxGeometricError);
};


