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

#ifndef _OSG2GLTF
#define _OSG2GLTF
#include <Windows.h>
#include <osg/NodeVisitor>
#include <osg/Matrix>
#include <osg/Geometry>
#include <osg/Transform>
#include <osg/Texture2D>
#include <osgDB/WriteFile>
#include <osg/MatrixTransform>
#include <osgUtil/Export>
#include <map>
#include <set>
#include <osg/Geode>
#include <osg/PagedLOD>
#include "json/json.h"
#include <string>
#include <sstream>
#include "osg/Material"
#include "GeometryVisitor.h"
#include <fstream>

//二进制gltf(.glb)文件头
struct GLB_HEADER
{
	unsigned char magic[4];
	unsigned int version;
	unsigned int length;
	unsigned int contentLength;
	unsigned int contentFormat;
	GLB_HEADER()
	{
		std::string magicstr = "glTF";
		memcpy(magic, magicstr.data(), 4);
		version = 1;
		contentFormat = 0;
	}
};

//Batched 3D Model文件头
struct Batched3DModelHeader
{
	unsigned char magic[4];
	unsigned int version;
	unsigned int byteLength;
	unsigned int featureTableJSONByteLength;
	unsigned int featureTableBinaryByteLength;
	unsigned int batchTableJSONByteLength;
	unsigned int batchTableBinaryByteLength;
	Batched3DModelHeader()
	{
		std::string magicstr = "b3dm";
		memcpy(magic, magicstr.data(), 4);
		version = 1;
		featureTableJSONByteLength = 0;
		featureTableBinaryByteLength = 0;
		batchTableJSONByteLength = 0;
		batchTableBinaryByteLength = 0;

	}
};

//顶点着色GLSL
static const char *VertexShader_Textured = {
"precision highp float;\n"
"attribute vec3 a_position;\n"
"attribute vec3 a_normal;\n"
"attribute vec4 a_color;\n"
"attribute float a_batchId;\n"
"varying vec3 v_normal;\n"
"varying float v_batchId;\n"
"varying vec4 v_color;\n"
"uniform mat3 u_normalMatrix;\n"
"uniform mat4 u_modelViewMatrix;\n"
"uniform mat4 u_projectionMatrix;\n"
"attribute vec2 a_texcoord0;\n"
"varying vec2 v_texcoord0;\n"

"void main(void)\n"
"{\n"
"    vec4 pos = u_modelViewMatrix * vec4(a_position,1.0);\n"
"    v_normal = u_normalMatrix * a_normal;\n"
"    v_texcoord0 = a_texcoord0;\n"
"    v_color = a_color;\n"
"    v_batchId = a_batchId;\n"
"    gl_Position = u_projectionMatrix * pos;\n"
"}\n"
};

//像素着色GLSL
static const char *FragmentShader_Textured = {
"precision highp float;\n"
"varying vec3 v_normal;\n"
"varying float v_batchId;\n"
"varying vec2 v_texcoord0;\n"
"varying vec4 v_color;\n"
"uniform vec4 u_diffuse;\n"
"uniform sampler2D u_diffuseTex;\n"
"uniform vec4 u_specular;\n"
"uniform float u_shininess;\n"

"void main(void)\n"
"{\n"
"    vec3 normal = normalize(v_normal);\n"
"    vec4 color = vec4(0., 0., 0., 0.);\n"
"    vec4 diffuse = vec4(0., 0., 0., 1.);\n"
"    vec4 specular = u_specular;\n"
"    diffuse = texture2D(u_diffuseTex, v_texcoord0);\n"
"    diffuse.rgb = v_color.a * diffuse.rgb * u_diffuse.rgb;\n"
"    if(length(v_color.rgb) > 0.0001)\n"
"       diffuse.rgb = diffuse.rgb * v_color.rgb;\n"
"    //diffuse.rgb *= max(dot(normal,vec3(0.,0.,1.)), 0.);\n"
"    //diffuse.a = diffuse.a * u_diffuse.a * v_color.a;\n"
"   //color.xyz += diffuse.xyz;\n"
"   //color = vec4(color.rgb * diffuse.a, diffuse.a);\n"
"   gl_FragColor = diffuse;\n"
"}\n"
};

//OSGZ->GLTF转换类
class OSG2GLTF
{
public:
	OSG2GLTF()
	{
		m_vsShaderFileName = "VertexShader.glsl";
		m_fsShaderFileName = "FragmentShader.glsl";
		m_externalShaderPath = "";
		m_flipAxis = true;
		//m_resourcesDir = "B:/cesium-3d-tiles/Apps/SampleData/models/OSG2GLTF/";
	}
	~OSG2GLTF()
	{

	}
	//文件类型
	enum FileType
	{
		GLTF = 0,//文本GLTF
		GLB = 1,//二进制GLB
		B3DM = 2//3D Tile使用的Batched 3D Model
	};
	//把osg节点转换为GLTF,GLB或者B3DM
	//outdir:输出文件夹路径
	//outname:不带后缀的文件名
	void toGLTF(osg::Node* osgNode, std::string outdir, std::string outname, FileType type = FileType::GLTF);
	//把osg节点文件转换为GLTF,GLB或者B3DM
	void toGLTF(std::string filename, std::string outdir, std::string outname, FileType type = FileType::GLTF);
	//是否从Z轴朝上翻转为Y朝上，默认为TRUE
	void setFlipAxis(bool flip);
	//设置shader输出路径
	void setExternalShaderPath(std::string shaderPath);
private:
	std::map<std::string, Json::Value> m_materials;
	std::map<std::string, Json::Value> m_bufferViews;
	std::map<std::string, Json::Value> m_accessors;
	std::map<std::string, Json::Value> m_buffers;
	std::map<std::string, Json::Value> m_meshes;
	std::map<std::string, Json::Value> m_nodes;
	std::vector<std::pair<std::string, Json::Value> > m_nodelist;
	std::map<std::string, Json::Value> m_techniques;
	std::vector<char>  m_indexBufferData;
	std::vector<char>  m_vertexBufferData;
	std::vector<char>  m_batchIdBufferData;
	std::vector<char>  m_imageBufferData;
	std::vector<char>  m_shaderBufferData;
	FileType m_type;
	//非透明材质Json Technique节点
	Json::Value m_Technique;
	//透明材质Json Technique节点
	Json::Value m_TransparentTechnique;
	Json::Value m_extensionsUsed;
	std::string m_fsShaderFileName;
	std::string m_vsShaderFileName;
	std::string m_externalShaderPath;
	bool m_flipAxis;
	//std::string m_resourcesDir;
	GeometryVisitor m_GeometryVisitor;
	std::string getPointer(void* ptr);
	void getMaterial(osg::StateSet* stateset, osg::Material*& mat, osg::Texture2D*& tex);
	Json::Value createMaterialNode(osg::Material* mat, std::string name, std::string texname = "");
	Json::Value createNode(osg::Node* node, osg::StateSet* parentStateSet = NULL);
	void attachNode(Json::Value& root, std::string name1, Json::Value val);
	void attachNode(Json::Value& root, std::string name1, std::string name2, Json::Value val);
	void attachNode(Json::Value& root, std::string name1, std::vector<std::string> name2, std::vector<Json::Value> val);
	Json::Value createTechniqueNode(bool istransparent, FileType type);
	Json::Value createBuffer(std::string outdir,std::string buffername,FileType type);
	Json::Value createIndexBufferView(std::string buffername);
	Json::Value createVertexBufferView(std::string buffername);
	Json::Value createbatchIdBufferView(std::string buffername);
	void writeImages(std::string outdir);
	static char* readBinary(std::string filename, size_t& len);
	Json::Value createImages(std::string outdir,std::string buffername, FileType type = FileType::GLTF);
	Json::Value createShader(std::string outdir,std::string shaderFile,const std::string& shaderSource, int shaderType, std::string buffername, FileType type = FileType::GLTF);
	void writeGLTF(std::string outdir, std::string outname,std::string buffername,Json::Value& gltf, FileType type = FileType::GLTF);
	Json::Value createBatchTable();
	Json::Value createFeatureTable();

};


#endif
