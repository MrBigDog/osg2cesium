/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#ifndef _GeometryVisitor
#define _GeometryVisitor
#include <Windows.h>
#include <osg/NodeVisitor>
#include <osg/Matrix>
#include <osg/Geometry>
#include <osg/Transform>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgUtil/Export>

#include <set>
#include <osg/Geode>
#include <osg/PagedLOD>
#include "json/json.h"
#include <string>
#include <sstream>
#include "osg/Material"
//#include "glTF.h"
//WebGL仅支持unsigned short类型顶点索引
//用于表达unsigned short类型顶点索引
class Vec3ushort
{
public:

	/** Type of Vec class.*/
	typedef unsigned short value_type;

	/** Number of vector components. */
	enum { num_components = 3 };

	/** Vec member varaible. */
	value_type _v[3];

	Vec3ushort() { _v[0] = 0; _v[1] = 0; _v[2] = 0; }

	Vec3ushort(osg::Vec3i vec3i) { _v[0] = (unsigned short)vec3i.x(); _v[1] = (unsigned short)vec3i.y(); _v[2]  = (unsigned short)vec3i.z(); }
	Vec3ushort(value_type r, value_type g, value_type b) { _v[0] = r; _v[1] = g; _v[2] = b; }

	inline bool operator == (const Vec3ushort& v) const { return _v[0] == v._v[0] && _v[1] == v._v[1] && _v[2] == v._v[2]; }
	inline bool operator != (const Vec3ushort& v) const { return _v[0] != v._v[0] || _v[1] != v._v[1] || _v[2] != v._v[2]; }
	inline bool operator <  (const Vec3ushort& v) const
	{
		if (_v[0]<v._v[0])      return true;
		else if (_v[0]>v._v[0]) return false;
		else if (_v[1]<v._v[1]) return true;
		else if (_v[1]>v._v[1]) return false;
		else return (_v[2]<v._v[2]);
	}

	inline value_type* ptr() { return _v; }
	inline const value_type* ptr() const { return _v; }

	inline void set(value_type r, value_type g, value_type b)
	{
		_v[0] = r; _v[1] = g; _v[2] = b;
	}

	inline void set(const Vec3ushort& rhs)
	{
		_v[0] = rhs._v[0]; _v[1] = rhs._v[1]; _v[2] = rhs._v[2];
	}

	inline value_type& operator [] (unsigned int i) { return _v[i]; }
	inline value_type operator [] (unsigned int i) const { return _v[i]; }

	inline value_type& x() { return _v[0]; }
	inline value_type& y() { return _v[1]; }
	inline value_type& z() { return _v[2]; }

	inline value_type x() const { return _v[0]; }
	inline value_type y() const { return _v[1]; }
	inline value_type z() const { return _v[2]; }

	inline value_type& r() { return _v[0]; }
	inline value_type& g() { return _v[1]; }
	inline value_type& b() { return _v[2]; }

	inline value_type r() const { return _v[0]; }
	inline value_type g() const { return _v[1]; }
	inline value_type b() const { return _v[2]; }

	/** Multiply by scalar. */
	inline Vec3ushort operator * (value_type rhs) const
	{
		return Vec3ushort(_v[0] * rhs, _v[1] * rhs, _v[2] * rhs);
	}

	inline Vec3ushort operator / (value_type rhs) const
	{
		return Vec3ushort(_v[0] / rhs, _v[1] / rhs, _v[2] / rhs);
	}

	inline Vec3ushort operator + (value_type rhs) const
	{
		return Vec3ushort(_v[0] + rhs, _v[1] + rhs, _v[2] + rhs);
	}

	inline Vec3ushort operator - (value_type rhs) const
	{
		return Vec3ushort(_v[0] - rhs, _v[1] - rhs, _v[2] - rhs);
	}

	inline Vec3ushort operator + (const Vec3ushort& rhs) const
	{
		return Vec3ushort(_v[0] + rhs._v[0], _v[1] + rhs._v[1], _v[2] + rhs._v[2]);
	}

	inline Vec3ushort operator - (const Vec3ushort& rhs) const
	{
		return Vec3ushort(_v[0] - rhs._v[0], _v[1] - rhs._v[1], _v[2] - rhs._v[2]);
	}

	inline Vec3ushort operator * (const Vec3ushort& rhs) const
	{
		return Vec3ushort(_v[0] * rhs._v[0], _v[1] * rhs._v[1], _v[2] * rhs._v[2]);
	}
};

//osg节点中description到Batch Table信息的转换类
struct BatchInfo
{
	//根节点名字
	std::string rootName;
	//父节点名字
	std::string parentName;
	//geode节点中读取的description
	std::string description;
	//按顺序生成的BatchId
	unsigned short sequentialID;
	//osg::Geometry的ID
	unsigned short geometryID;
	//osg::Geometry顶点开始索引
	int startIndex;
	//osg::Geometry顶点结束索引
	int endIndex;
	osg::Vec3 color;
	unsigned short uniqueID;
	BatchInfo()
	{

	}
	bool create(std::string _description)
	{
		description = _description;
		if (_description == "")
			return false;
		if (_description.size() < 5)
			return false;
		std::vector<std::string> splits;
		int lastidx = 0;
		for (size_t i = 1; i < _description.size(); i++)
		{
			if (_description[i] == ',')
			{
				splits.push_back(_description.substr(lastidx, i - lastidx));
				lastidx = +i + 1;
			}
			else if (i == _description.size() - 1)
			{
				splits.push_back(_description.substr(lastidx, i - lastidx + 1));
			}
		}
		if (splits.size() < 4)
			return false;
		rootName = splits[0];
		parentName = splits[splits.size() - 4];
		geometryID = atoi(splits[splits.size() - 3].data());
		startIndex = atoi(splits[splits.size() - 2].data());
		endIndex   = atoi(splits[splits.size() - 1].data());
		color = osg::Vec3((rand() % 255) / 255.0, (rand() % 255) / 255.0, (rand() % 255) / 255.0);
		/*for (size_t i = 0; i < splits.size() - 3; i++)
		{
			names += splits[i];
			if (i != splits.size() - 3 - 1)
				names += ",";
		}*/
		return true;
	}
	BatchInfo(std::string infostr)
	{
		create(infostr);
	}

};

//osg::Geometry的包装类
//保存3D Tile的Mesh数据
class GeometryWrapper
{
public:

	std::vector<osg::Vec3i> tri_indices;
	std::vector<osg::Vec3> tri_positions;
	std::vector<osg::Vec3> tri_normals;
	std::vector<osg::Vec2> tri_texcoords0;
	std::vector<osg::Vec2> tri_texcoords1;
	std::vector<osg::Vec4> tri_colors;
	std::vector<unsigned short> tri_batchIds;
	std::vector<BatchInfo> batches;
	osg::BoundingBox bound;
	Json::Value createJsonMeshNode(
		std::string name, 
		std::string materialName,
		std::map<std::string, Json::Value>& accessorsJS,
		std::vector<char>& indexBufferData, 
		std::vector<char>& vertexBufferData,
		std::vector<char>& batchIdBufferData);
	
};

//遍历osg节点，搜集所有几何,纹理和矩阵等数据
class GeometryVisitor : public osg::NodeVisitor
{
public:
	static std::map<std::string, BatchInfo> getMasterBatchTable();
	static void clearBatchTable();
	GeometryVisitor() :osg::NodeVisitor(osg::NodeVisitor::NODE_VISITOR, TRAVERSE_ALL_CHILDREN)
	{
		setNodeMaskOverride(0xffffffff); 
		//m_flipAxis = true;
	}
	~GeometryVisitor();
	virtual void apply(osg::Geode& node);
	virtual void apply(osg::Node& node);
	//纹理集合
	std::map<std::string, osg::Image* > m_ImageMap;
	//几何集合:GeometryWrapper对应一个osg::Geometry
	std::map<osg::Geometry*, GeometryWrapper*> m_GeometryMap;
	//3D Tile纹理采样器集合
	std::map<std::string, Json::Value> m_Samplers;
	//3D Tile纹理器集合
	std::map<std::string, Json::Value> m_Textures;
	//3D Tile纹理器集合
	std::map<std::string, Json::Value> m_Images;
	std::vector<BatchInfo> m_Batches;
	//std::map<GeometryWrapper*, std::vector<BatchInfo>> m_Batches;
	void gatherTexture(osg::Texture* tex);
	//bool m_flipAxis;
private:
	osg::BoundingBox g_mExtent;

};


#endif
