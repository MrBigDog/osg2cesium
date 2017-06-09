#include "GeometryVisitor.h"
#include "osgDB\ReadFile"
#include <map>
#include <osg/Geometry>
#include <osgUtil/SmoothingVisitor>
std::map<std::string, BatchInfo> m_masterBatchTable;

class FaceVisitor
{
public:
	FaceVisitor(osg::PrimitiveSet* pset)
	{
		pSet = pset;
	}
	std::vector<unsigned int> getFaceIndices()
	{

		std::vector<unsigned int> indices;
		unsigned int idx = pSet->getNumIndices();
		unsigned int numofprims;


		if (pSet->getMode() == osg::PrimitiveSet::TRIANGLE_FAN)
		{
			numofprims = idx - 2;
			for (unsigned int i = 0; i<numofprims; i++)
			{
				indices.push_back(pSet->index(0));
				indices.push_back(pSet->index(i + 1));
				indices.push_back(pSet->index(i + 2));
			}
		}
		else if (pSet->getMode() == osg::PrimitiveSet::TRIANGLES)
		{
			for (unsigned int i = 0; i<idx; i++)
			{
				indices.push_back(pSet->index(i));
			}
		}
		else if (pSet->getMode() == osg::PrimitiveSet::TRIANGLE_STRIP)
		{
			numofprims = (idx - 2) / 2;
			for (unsigned int i = 0; i<numofprims; i++)
			{
				indices.push_back(pSet->index(i * 2));
				indices.push_back(pSet->index(i * 2 + 1));
				indices.push_back(pSet->index(i * 2 + 2));

				indices.push_back(pSet->index(i * 2 + 2));
				indices.push_back(pSet->index(i * 2 + 1));
				indices.push_back(pSet->index(i * 2 + 3));
			}

			if (numofprims + 2 % 2 != 0)
			{
				indices.push_back(pSet->index(idx - 3));
				indices.push_back(pSet->index(idx - 2));
				indices.push_back(pSet->index(idx - 1));
			}
		}
		else if (pSet->getMode() == osg::PrimitiveSet::QUADS)
		{
			numofprims = idx / 4;
			for (unsigned int i = 0; i< numofprims; i++)
			{
				indices.push_back(pSet->index(i * 4));
				indices.push_back(pSet->index(i * 4 + 1));
				indices.push_back(pSet->index(i * 4 + 2));

				indices.push_back(pSet->index(i * 4));
				indices.push_back(pSet->index(i * 4 + 2));
				indices.push_back(pSet->index(i * 4 + 3));
			}

		}


		return indices;
	}
	std::vector<osg::Vec3i> getFaceIndicesVec3i()
	{

		std::vector<osg::Vec3i> indices;
		unsigned int idx = pSet->getNumIndices();
		unsigned int numofprims;


		if (pSet->getMode() == osg::PrimitiveSet::TRIANGLE_FAN)
		{
			numofprims = idx - 2;
			for (unsigned int i = 0; i<numofprims; i++)
			{
				indices.push_back(osg::Vec3i(pSet->index(0), pSet->index(i + 1), pSet->index(i + 2)));
			}
		}
		else if (pSet->getMode() == osg::PrimitiveSet::TRIANGLES)
		{
			for (unsigned int i = 0; i < idx/3 ; i++)
			{
				indices.push_back(osg::Vec3i(pSet->index(i * 3), pSet->index(i * 3 + 1), pSet->index(i * 3 + 2)));
			}
		}
		else if (pSet->getMode() == osg::PrimitiveSet::TRIANGLE_STRIP)
		{
			numofprims = (idx - 2) / 2;
			for (unsigned int i = 0; i<numofprims; i++)
			{
				indices.push_back(osg::Vec3i(pSet->index(i * 2), pSet->index(i * 2 + 1), pSet->index(i * 2 + 2)));
				indices.push_back(osg::Vec3i(pSet->index(i * 2 + 2), pSet->index(i * 2 + 1), pSet->index(i * 2 + 3)));
			}

			//if (numofprims + 2 % 2 != 0)
			//{
			//	indices.push_back(osg::Vec3i(pSet->index(idx - 3), pSet->index(idx - 2), pSet->index(idx - 1)));
			//}
		}
		else if (pSet->getMode() == osg::PrimitiveSet::QUADS)
		{
			numofprims = idx / 4;
			for (unsigned int i = 0; i< numofprims; i++)
			{
				indices.push_back(osg::Vec3i(pSet->index(i * 4), pSet->index(i * 4 + 1), pSet->index(i * 4 + 2)));
				indices.push_back(osg::Vec3i(pSet->index(i * 4), pSet->index(i * 4 + 2), pSet->index(i * 4 + 3)));
			}

		}
		return indices;
	}
private:
	osg::PrimitiveSet* pSet;
};

osg::Vec3 getVec3(osg::Array* arr, int i)
{
	if (arr->getType() == osg::Array::Vec3ArrayType)
	{
		return (*((osg::Vec3Array *)arr))[i];
	}
	else if (arr->getType() == osg::Array::Vec3dArrayType)
	{
		return (*((osg::Vec3dArray *)arr))[i];
	}
	//else if (arr->getDataType() == osg::Array::Vec4ArrayType)
	
	osg::Vec4 v = (*((osg::Vec4Array *)arr))[i];
	return osg::Vec3(v.x(), v.y(), v.z());
	//}

	
}

osg::Vec4 getVec4(osg::Array* arr, int i)
{

	if (arr->getType() == osg::Array::Vec3ArrayType)
	{
		return osg::Vec4(((osg::Vec3Array *)arr)->at(i),1);
	}
	else if (arr->getType() == osg::Array::Vec3dArrayType)
	{
		return osg::Vec4(((osg::Vec3dArray *)arr)->at(i), 1);
	}
	else if (arr->getType() == osg::Array::Vec4ArrayType)
	{
		return ((osg::Vec4Array *)arr)->at(i);
	}
	else if (arr->getType() == osg::Array::Vec4dArrayType)
	{
		return ((osg::Vec4dArray *)arr)->at(i);
	}

	return osg::Vec4(1,1,1,1);

}

std::map<std::string, BatchInfo> GeometryVisitor::getMasterBatchTable()
{
	return m_masterBatchTable;
}
void GeometryVisitor::clearBatchTable()
{
	m_masterBatchTable.clear();
}
void GeometryVisitor::apply(osg::Node& node)
{
	//统计纹理
	osg::Texture2D* tempTex = dynamic_cast<osg::Texture2D*>(node.getOrCreateStateSet()->getTextureAttribute(
		0, osg::StateAttribute::Type::TEXTURE));
	if (tempTex != NULL)
	{
		gatherTexture(tempTex);
	}
	traverse(node);
}
GeometryVisitor::~GeometryVisitor()
{
	std::map<osg::Geometry*, GeometryWrapper*>::iterator iter = m_GeometryMap.begin();
	while (iter != m_GeometryMap.end())
	{
		delete iter->second;
		iter++;
	}

}

void GeometryVisitor::apply(osg::Geode& node)
{

	osg::MatrixList matlist = node.getWorldMatrices();
	osg::Matrix matWorld = osg::Matrix::identity();
	
	/*计算goede的坐标变换 ？*/
	for (unsigned int i = 0; i<matlist.size(); i++)
	{
		matWorld = matlist[i] * matWorld;
	}
	std::vector<GeometryWrapper*> wrappers;
	for (int i = 0; i < node.getNumDrawables(); i++)
	{
		GeometryWrapper* wrapper = new GeometryWrapper;
		wrappers.push_back(wrapper);
	}

	for (int i = 0; i < node.getNumDescriptions(); i++)
	{
		std::string desc = node.getDescription(i);
		BatchInfo batchinfo;
		if (batchinfo.create(desc))
		{
			std::string batchid = batchinfo.rootName + "_" + batchinfo.parentName;
			std::map<std::string, BatchInfo>::iterator iter = m_masterBatchTable.find(batchid);
			if (iter == m_masterBatchTable.end())
			{
				batchinfo.uniqueID = m_masterBatchTable.size();
				m_masterBatchTable[batchid] = batchinfo;
			}
			else
			{
				batchinfo.color = iter->second.color;
				batchinfo.uniqueID = iter->second.uniqueID;
			}
			batchinfo.sequentialID = batchinfo.uniqueID;
			m_Batches.push_back(batchinfo);
			int geomidx = batchinfo.geometryID;
			osg::Geometry* geom = dynamic_cast<osg::Geometry*>(node.getDrawable(geomidx));
			osg::Array* colors = geom->getColorArray();
			osg::Vec3Array* colors3 = dynamic_cast<osg::Vec3Array*>(geom->getColorArray());
			osg::Vec4Array* colors4 = dynamic_cast<osg::Vec4Array*>(geom->getColorArray());
			osg::PrimitiveSet* primitiveSet = geom->getPrimitiveSet(0);
			unsigned int idx = primitiveSet->getNumIndices();
			osg::Array* vertices = geom->getVertexArray();
			int numvertices = vertices->getNumElements();
			GeometryWrapper* wrapper = wrappers[batchinfo.geometryID];
			wrapper->batches.push_back(batchinfo);
			wrapper->tri_batchIds.resize(numvertices);
			for (size_t n = batchinfo.startIndex; n < batchinfo.endIndex; n++)
			{
				std::vector<unsigned int> indices;
				unsigned int idx = primitiveSet->index(n);
				//if (colors3)
				//{
				//	(*colors3)[idx] = batchinfo.color;
				//}
				//else if (colors4)
				//{
				//	(*colors4)[idx] = osg::Vec4(batchinfo.color, (*colors4)[idx].a());
				//}
				wrapper->tri_batchIds[idx] = batchinfo.uniqueID;
			}
		}

	}


	for (int i = 0; i<node.getNumDrawables(); i++)
	{
		osg::Geometry* geom = dynamic_cast<osg::Geometry*>(node.getDrawable(i));
		if (!geom) 
			continue;
		osg::Texture2D* tempTex = dynamic_cast<osg::Texture2D*>(geom->getOrCreateStateSet()->getTextureAttribute(
			0, osg::StateAttribute::Type::TEXTURE));
		if (tempTex != NULL)
		{
			gatherTexture(tempTex);
		}

		osg::Array* vertices = geom->getVertexArray();
		osg::Array* normals = geom->getNormalArray();
		if (!normals)
		{
			osgUtil::SmoothingVisitor sv;
			node.accept(sv);
			normals = geom->getNormalArray();
		}
		osg::Vec2Array* texcoords0 = (osg::Vec2Array *) geom->getTexCoordArray(0);
		osg::Vec2Array* texcoords1 = (osg::Vec2Array *) geom->getTexCoordArray(1);
		osg::Array* colors = geom->getColorArray();

		GeometryWrapper* wrapper = wrappers[i];
		wrapper->bound.init();
		int numvertices = vertices->getNumElements();
		for (size_t j= 0; j < numvertices; j++)
		{
			osg::Vec3 pos = getVec3(vertices, j);
			if (m_flipAxis)
				pos = osg::Vec3(pos.x(), pos.z(), -pos.y());
			wrapper->bound.expandBy(pos);
			wrapper->tri_positions.push_back(pos);
		}
		if (texcoords0)
		{
			for (size_t j = 0; j < texcoords0->size(); j++)
			{
				wrapper->tri_texcoords0.push_back((*texcoords0)[j]);
			}
			for (size_t j = 0; j < numvertices - texcoords0->size(); j++)
			{
				wrapper->tri_texcoords0.push_back(osg::Vec2(0,0));
			}
		}
		if (texcoords1)
		{
			for (size_t j = 0; j < numvertices; j++)
			{
				wrapper->tri_texcoords0.push_back((*texcoords1)[j]);
			}
		}

		if (geom->getNormalBinding() == osg::Geometry::BIND_OVERALL)
		{
			for (size_t j = 0; j <numvertices; j++)
			{
				osg::Vec3 normal = getVec3(normals, 0);
				if (m_flipAxis)
					normal = -osg::Vec3(normal.x(), normal.z(), -normal.y());
				wrapper->tri_normals.push_back(normal);
			}

		}
		else if (geom->getNormalBinding() == osg::Geometry::BIND_PER_VERTEX)
		{
			for (size_t j = 0; j < numvertices; j++)
			{
				osg::Vec3 normal = getVec3(normals, j);
				if (m_flipAxis)
					normal = osg::Vec3(normal.x(), normal.z(), -normal.y());
				wrapper->tri_normals.push_back(normal);
			}

		}

		osg::Vec4 diffuseColor(1, 1, 1, 1);
		if(colors && colors->getNumElements() > 0)
		{
			if (geom->getColorBinding() == osg::Geometry::BIND_OVERALL)
			{
				for (size_t j = 0; j < numvertices; j++)
				{
					wrapper->tri_colors.push_back(getVec4(colors, 0));
				}

			}
			else if (geom->getColorBinding() == osg::Geometry::BIND_PER_VERTEX && colors->getNumElements() == numvertices)
			{
				for (size_t j = 0; j < numvertices; j++)
				{
					wrapper->tri_colors.push_back(getVec4(colors, j));
				}

			}
		}
		std::vector<unsigned int> tri_indices;
		std::vector<osg::Vec3> tri_positions;
		bool hasIndexBuffer = false;
		for (int j = 0; j<geom->getNumPrimitiveSets(); ++j)
		{
			osg::PrimitiveSet* primitiveSet = geom->getPrimitiveSet(j);

			FaceVisitor visitor(primitiveSet);
			std::vector<osg::Vec3i> indices = visitor.getFaceIndicesVec3i();
			if (!dynamic_cast<osg::DrawArrays*>(primitiveSet))
			{
				hasIndexBuffer = true;
			}
			for (unsigned int k = 0; k< indices.size(); k++)
			{
				wrapper->tri_indices.push_back(indices[k]);
			}
		}

		m_GeometryMap[geom] = wrapper;
	}


}
void GeometryVisitor::gatherTexture(osg::Texture* tex)
{

	if (!tex || !tex->getImage(0))
		return;
	osg::Image* img = tex->getImage(0);
	if (img->s() == 0 || img->t() == 0)
	{
		printf("");
		return;
	}

	//std::stringstream ss;
	//ss << img;
	std::string filename = img->getFileName();
	int lastdot = -1;
	int lastslash = -1;
	for (int i = filename.size() - 1; i >= 0; i--)
	{
		if (lastslash == -1 && (filename[i] == '/' || filename[i] == '\\'))
		{
			lastslash = i;
			break;
		}

		if (lastdot == -1 && filename[i] == '.')
		{
			lastdot = i;
		}
	}
	std::string ext = filename.substr(lastdot + 1, filename.size() - (lastdot + 1));
	if (ext == "tga")
	{
		if (img->getPixelFormat() == GL_RGBA)
		{
			ext = "png";
		}
		else
		{
			ext = "jpg";
		}
		for (int i = 0; i < 3; i++)
		{
			filename[filename.size() - 3 + i] = ext[i];
		}
	}
	std::string name = filename.substr(lastslash + 1, filename.size() - (lastslash + 1));
	//std::string imageNodeName = "image_" + name;
	//std::string samplerNodeName = "sampler_" + name;
	//std::string textureNodeName = "texture_" + name;
	std::string imageNodeName = name;
	std::string samplerNodeName = name;
	std::string textureNodeName = name;
	img->setName(name);
	m_ImageMap[name] = img;

	Json::Value jsImage;
	jsImage["name"] = name;
	jsImage["uri"] = name;

	Json::Value jsSsampler;

	//9728 (NEAREST), 
	//9729 (LINEAR),
	//9984 (NEAREST_MIPMAP_NEAREST),
	//9985 (LINEAR_MIPMAP_NEAREST), 
	//9986 (NEAREST_MIPMAP_LINEAR), 
	//9987 (LINEAR_MIPMAP_LINEAR).
	osg::Texture::FilterMode magFilter = tex->getFilter(osg::Texture::FilterParameter::MAG_FILTER);
	osg::Texture::FilterMode minFilter = tex->getFilter(osg::Texture::FilterParameter::MIN_FILTER);

	//33071 (CLAMP_TO_EDGE), 33648 (MIRRORED_REPEAT), and 10497 (REPEAT).
	osg::Texture::WrapMode wrapS = tex->getWrap(osg::Texture::WRAP_S);
	if (wrapS == osg::Texture::CLAMP || wrapS == osg::Texture::CLAMP_TO_BORDER)
		wrapS = osg::Texture::CLAMP_TO_EDGE;
	osg::Texture::WrapMode wrapT = tex->getWrap(osg::Texture::WRAP_T);
	if (wrapT == osg::Texture::CLAMP || wrapT == osg::Texture::CLAMP_TO_BORDER)
		wrapT = osg::Texture::CLAMP_TO_EDGE;

	if (magFilter == osg::Texture::FilterMode::LINEAR ||
		magFilter == osg::Texture::FilterMode::LINEAR_MIPMAP_LINEAR ||
		magFilter == osg::Texture::FilterMode::LINEAR_MIPMAP_NEAREST)
	{
		jsSsampler["magFilter"] = 9729;
	}
	else
	{
		jsSsampler["magFilter"] = 9728;
	}

	jsSsampler["minFilter"] = minFilter;
	jsSsampler["wrapS"] = wrapS;
	jsSsampler["wrapT"] = wrapT;

	Json::Value jsTexture;
	jsTexture["format"] = img->getPixelFormat();
	jsTexture["sampler"] = samplerNodeName;
	jsTexture["source"] = imageNodeName;
	jsTexture["target"] = 3553;//GL_UNSIGNED_SHORT
	jsTexture["type"] = 5121;//GL_UNSIGNED_SHORT
	m_Images[name] = jsImage;
	m_Textures[name] = jsTexture;
	m_Samplers[name] = jsSsampler;
}

Json::Value GeometryWrapper::createJsonMeshNode(
	std::string name,
	std::string materialName,
	std::map<std::string, Json::Value>& accessorsJS,
	std::vector<char>& indexBufferData,
	std::vector<char>& vertexBufferData,
	std::vector<char>& batchIdBufferData)
{
	std::vector<std::vector<Vec3ushort>> tri_indices_parts;

	if (tri_positions.size() > 65535)
	{

		std::vector<osg::Vec3> positions;
		if (tri_indices.size() == 0)
		{
			for (size_t i = 0; i < tri_positions.size() / 3; i++)
			{
				tri_indices.push_back(osg::Vec3i(i * 3, i * 3 + 1, i * 3 + 2));
			}
		}


		for (size_t itri = 0; itri < tri_indices.size(); itri++)
		{
			osg::Vec3i& tri = tri_indices[itri];
			positions.push_back(tri_positions[tri.x()]);
			positions.push_back(tri_positions[tri.y()]);
			positions.push_back(tri_positions[tri.z()]);
			
		}
		tri_positions = positions;
	
		if (tri_normals.size() > 0)
		{
			std::vector<osg::Vec3> normals;
			for (size_t itri = 0; itri < tri_indices.size(); itri++)
			{
				osg::Vec3i& tri = tri_indices[itri];
				normals.push_back(tri_normals[tri.x()]);
				normals.push_back(tri_normals[tri.y()]);
				normals.push_back(tri_normals[tri.z()]);
			}
			tri_normals = normals;
		}

		if (tri_colors.size() > 0)
		{
			std::vector<osg::Vec4> colors;
			for (size_t itri = 0; itri < tri_indices.size(); itri++)
			{
				osg::Vec3i& tri = tri_indices[itri];
				colors.push_back(tri_colors[tri.x()]);
				colors.push_back(tri_colors[tri.y()]);
				colors.push_back(tri_colors[tri.z()]);
			}
			tri_colors = colors;
		}

		if (tri_texcoords0.size() > 0)
		{
			std::vector<osg::Vec2> texcoords0;
			for (size_t itri = 0; itri < tri_indices.size(); itri++)
			{
				osg::Vec3i& tri = tri_indices[itri];
				texcoords0.push_back(tri_texcoords0[tri.x()]);
				texcoords0.push_back(tri_texcoords0[tri.y()]);
				texcoords0.push_back(tri_texcoords0[tri.z()]);
			}
			tri_texcoords0 = texcoords0;
		}

		if (tri_texcoords1.size() > 0)
		{
			std::vector<osg::Vec2> texcoords1;
			for (size_t itri = 0; itri < tri_indices.size(); itri++)
			{
				osg::Vec3i& tri = tri_indices[itri];
				texcoords1.push_back(tri_texcoords1[tri.x()]);
				texcoords1.push_back(tri_texcoords1[tri.y()]);
				texcoords1.push_back(tri_texcoords1[tri.z()]);
			}
			tri_texcoords1 = texcoords1;
		}

		if (tri_batchIds.size() > 0)
		{
			std::vector<unsigned short> batchIds;
			for (size_t itri = 0; itri < batches.size(); itri++)
			{
				osg::Vec3i& tri = tri_indices[itri];
				batchIds.push_back(tri_batchIds[tri.x()]);
				batchIds.push_back(tri_batchIds[tri.y()]);
				batchIds.push_back(tri_batchIds[tri.z()]);
			}
			tri_batchIds = batchIds;
		}

	}
	else if(tri_indices.size() > 0)
	{
		tri_indices_parts.push_back(std::vector<Vec3ushort>());
		std::vector<Vec3ushort>& tri_indices_part = tri_indices_parts[tri_indices_parts.size() - 1];
		for (size_t itri = 0; itri < tri_indices.size(); itri++)
		{
			osg::Vec3i& tri = tri_indices[itri];
			tri_indices_part.push_back(Vec3ushort(tri));
		}

	}

	int indexbufsize = 0;
	for (int i = 0; i < tri_indices_parts.size(); i++)
	{
		indexbufsize += (tri_indices_parts[i].size() * sizeof(unsigned short) * 3);
	}

	int posbufsize = tri_positions.size() * 3 * sizeof(float);
	int normalbufsize = tri_normals.size() * 3 * sizeof(float);
	int tex0bufsize = tri_texcoords0.size() * 2 * sizeof(float);
	int tex1bufsize = tri_texcoords1.size() * 2 * sizeof(float);
	int colorbufsize = tri_colors.size() * 4 * sizeof(float);
	int batchIdbufsize = tri_batchIds.size() * sizeof(unsigned short);

	std::string indexBufferViewName = "bufferView_index";
	std::string vertexBufferViewName = "bufferView_vertex";
	std::string batchIdBufferViewName = "bufferView_batchId";
	int vertexBufferViewOffset = vertexBufferData.size();
	int indexAccessorOffset = indexBufferData.size();
	int batchIdAccessorOffset = batchIdBufferData.size();
	std::vector<std::pair<std::string, Json::Value>> indexAccessors;
	for (int i = 0; i < tri_indices_parts.size(); i++)
	{
		std::vector<Vec3ushort>& tri_indices_part = tri_indices_parts[i];
		Json::Value accessorIndx;
		std::stringstream ss;
		ss << "accessorIndx_" + name + "_" << i;
		std::string accessorIndxName = ss.str();
		accessorIndx["bufferView"] = indexBufferViewName;
		accessorIndx["byteOffset"] = indexAccessorOffset;
		accessorIndx["byteStride"] = 0;
		accessorIndx["componentType"] = 5123;//UNSIGNED_SHORT
		accessorIndx["count"] = tri_indices_part.size() * 3;
		accessorIndx["type"] = "SCALAR";
		indexAccessors.push_back(std::pair<std::string, Json::Value>(accessorIndxName, accessorIndx));
		int indexparoffset = (tri_indices_part.size() * 3 * sizeof(unsigned short));
		indexAccessorOffset += indexparoffset;
		accessorsJS[accessorIndxName] = accessorIndx;

		char* pIndex = (char*)(&tri_indices_part[0]);
		for (int nchar = 0; nchar < indexparoffset; nchar++)
		{
			indexBufferData.push_back(pIndex[nchar]);
		}

	}
	osg::BoundingBox bb;

	Json::Value accessorPosition;
	std::string accessorPositionName = "accessorPosition_" + name;
	accessorPosition["bufferView"] = vertexBufferViewName;
	accessorPosition["byteOffset"] = vertexBufferViewOffset;
	accessorPosition["byteStride"] = 12;
	accessorPosition["componentType"] = 5126;//FLOAT
	accessorPosition["count"] = tri_positions.size();
	bb.init();
	for (int i = 0; i < tri_positions.size(); i++)
	{
		bb.expandBy(tri_positions[i]);
	}
	Json::Value jsMax(Json::arrayValue);
	jsMax.append(bb.xMax());
	jsMax.append(bb.yMax());
	jsMax.append(bb.zMax());
	Json::Value jsMin(Json::arrayValue);
	jsMin.append(bb.xMin());
	jsMin.append(bb.yMin());
	jsMin.append(bb.zMin());
	accessorPosition["max"] = jsMax;
	accessorPosition["min"] = jsMin;
	accessorPosition["type"] = "VEC3";
	vertexBufferViewOffset += posbufsize;
	accessorsJS[accessorPositionName] = accessorPosition;
	char* pVertex = (char*)(&tri_positions[0]);
	for (int nchar = 0; nchar < posbufsize; nchar++)
	{
		vertexBufferData.push_back(pVertex[nchar]);
	}

	Json::Value accessorNormal;
	std::string accessorNormalName = "accessorNormal_" + name;
	if (normalbufsize > 0)
	{

		accessorNormal["bufferView"] = vertexBufferViewName;
		accessorNormal["byteOffset"] = vertexBufferViewOffset;
		accessorNormal["byteStride"] = 12;
		accessorNormal["componentType"] = 5126;//FLOAT
		accessorNormal["count"] = tri_normals.size();
		bb.init();
		for (int i = 0; i < tri_normals.size(); i++)
		{
			bb.expandBy(tri_normals[i]);
		}
		jsMax = Json::Value(Json::arrayValue);
		jsMax.append(bb.xMax());
		jsMax.append(bb.yMax());
		jsMax.append(bb.zMax());
		jsMin = Json::Value(Json::arrayValue);
		jsMin.append(bb.xMin());
		jsMin.append(bb.yMin());
		jsMin.append(bb.zMin());
		accessorNormal["max"] = jsMax;
		accessorNormal["min"] = jsMin;

		accessorNormal["type"] = "VEC3";
		vertexBufferViewOffset += normalbufsize;
		accessorsJS[accessorNormalName] = accessorNormal;

		pVertex = (char*)(&tri_normals[0]);
		for (int nchar = 0; nchar < normalbufsize; nchar++)
		{
			vertexBufferData.push_back(pVertex[nchar]);
		}

	}

	Json::Value accessorTexcoord0;
	std::string accessorTexcoord0Name = "accessorTexcoord0_" + name;
	if (tex0bufsize > 0)
	{
		accessorTexcoord0["bufferView"] = vertexBufferViewName;
		accessorTexcoord0["byteOffset"] = vertexBufferViewOffset;
		accessorTexcoord0["byteStride"] = 8;
		accessorTexcoord0["componentType"] = 5126;//FLOAT
		accessorTexcoord0["count"] = tri_texcoords0.size();
		bb.init();
		for (int i = 0; i < tri_texcoords0.size(); i++)
		{
			bb.expandBy(tri_texcoords0[i].x(), tri_texcoords0[i].y(), 1);
		}
		jsMax = Json::Value(Json::arrayValue);
		jsMax.append(bb.xMax());
		jsMax.append(bb.yMax());
		jsMin = Json::Value(Json::arrayValue);
		jsMin.append(bb.xMin());
		jsMin.append(bb.yMin());
		accessorTexcoord0["max"] = jsMax;
		accessorTexcoord0["min"] = jsMin;
		accessorTexcoord0["type"] = "VEC2";
		vertexBufferViewOffset += tex0bufsize;
		accessorsJS[accessorTexcoord0Name] = accessorTexcoord0;

		pVertex = (char*)(&tri_texcoords0[0]);
		for (int nchar = 0; nchar < tex0bufsize; nchar++)
		{
			vertexBufferData.push_back(pVertex[nchar]);
		}
	}

	Json::Value accessorTexcoord1;
	std::string accessorTexcoord1Name = "accessorTexcoord1_" + name;
	if (tex1bufsize > 0)
	{
		accessorTexcoord1["bufferView"] = vertexBufferViewName;
		accessorTexcoord1["byteOffset"] = vertexBufferViewOffset;
		accessorTexcoord1["byteStride"] = 8;
		accessorTexcoord1["componentType"] = 5126;//FLOAT
		accessorTexcoord1["count"] = tri_texcoords1.size();
		bb.init();
		for (int i = 0; i < tri_texcoords1.size(); i++)
		{
			bb.expandBy(tri_texcoords1[i].x(), tri_texcoords1[i].y(), 1);
		}
		jsMax = Json::Value(Json::arrayValue);
		jsMax.append(bb.xMax());
		jsMax.append(bb.yMax());
		jsMin = Json::Value(Json::arrayValue);
		jsMin.append(bb.xMin());
		jsMin.append(bb.yMin());
		accessorTexcoord1["max"] = jsMax;
		accessorTexcoord1["min"] = jsMin;
		accessorTexcoord1["type"] = "VEC2";
		vertexBufferViewOffset += tex0bufsize;
		accessorsJS[accessorTexcoord1Name] = accessorTexcoord1;

		pVertex = (char*)(&tri_texcoords1[0]);
		for (int nchar = 0; nchar < tex1bufsize; nchar++)
		{
			vertexBufferData.push_back(pVertex[nchar]);
		}
	}
	Json::Value accessorColor;
	std::string accessorColorName = "accessorColor_" + name;
	if (colorbufsize > 0)
	{
		accessorColor["bufferView"] = vertexBufferViewName;
		accessorColor["byteOffset"] = vertexBufferViewOffset;
		accessorColor["byteStride"] = 16;
		accessorColor["componentType"] = 5126;//UNSIGNED_SHORT
		accessorColor["count"] = tri_colors.size();
		accessorColor["type"] = "VEC4";
		vertexBufferViewOffset += colorbufsize;
		accessorsJS[accessorColorName] = accessorColor;

		pVertex = (char*)(&tri_colors[0]);
		for (int nchar = 0; nchar < colorbufsize; nchar++)
		{
			vertexBufferData.push_back(pVertex[nchar]);
		}
	}
	std::string accessorBatchIdName = "accessorBatchId_" + name;
	if (tri_batchIds.size() > 0)
	{
		Json::Value accessorBatchId;
		accessorBatchId["bufferView"] = batchIdBufferViewName;
		accessorBatchId["byteOffset"] = batchIdAccessorOffset;
		accessorBatchId["byteStride"] = 0;
		accessorBatchId["componentType"] = 5123;
		accessorBatchId["count"] = tri_batchIds.size();
		std::map<std::string, BatchInfo> masterTableByName = GeometryVisitor::getMasterBatchTable();
		std::map<std::string, BatchInfo>::iterator iter = masterTableByName.begin();
		int maxID = 0;
		while (iter != masterTableByName.end())
		{
			if (maxID < iter->second.uniqueID)
				maxID = iter->second.uniqueID;
			iter++;
		}

		Json::Value jsMin(Json::arrayValue);
		jsMin.append(0);
		Json::Value jsMax(Json::arrayValue);
		jsMax.append(maxID);
		accessorBatchId["min"] = jsMin;
		accessorBatchId["max"] = jsMax;
		accessorBatchId["type"] = "SCALAR";
		accessorsJS[accessorBatchIdName] = accessorBatchId;
		char* pBatchId = (char*)(&tri_batchIds[0]);
		for (int nchar = 0; nchar < batchIdbufsize; nchar++)
		{
			batchIdBufferData.push_back(pBatchId[nchar]);
		}
	}
	Json::Value geometryJS;
	geometryJS["name"] = name;
	geometryJS["primitives"] = Json::arrayValue;
	if (indexAccessors.size() > 0)
	{
		for (int i = 0; i < indexAccessors.size(); i++)
		{
			Json::Value primitiveSet;
			Json::Value attributesJS;
			attributesJS["POSITION"] = accessorPositionName;
			if (normalbufsize > 0)
				attributesJS["NORMAL"] = accessorNormalName;
			if (tex0bufsize > 0)
				attributesJS["TEXCOORD_0"] = accessorTexcoord0Name;
			if (tex1bufsize > 1)
				attributesJS["TEXCOORD_1"] = accessorTexcoord1Name;
			if (colorbufsize > 0)
				attributesJS["COLOR"] = accessorColorName;
			if (tri_batchIds.size() > 0)
				attributesJS["_BATCHID"] = accessorBatchIdName;
			primitiveSet["attributes"] = attributesJS;
			primitiveSet["indices"] = indexAccessors[i].first;
			primitiveSet["attributes"] = attributesJS;
			primitiveSet["material"] = materialName;
			primitiveSet["mode"] = 4;
			geometryJS["primitives"].append(primitiveSet);
		}
	}
	else
	{
		Json::Value primitiveSet;
		Json::Value attributesJS;

		attributesJS["POSITION"] = accessorPositionName;
		if (normalbufsize > 0)
			attributesJS["NORMAL"] = accessorNormalName;
		if (tex0bufsize > 0)
			attributesJS["TEXCOORD_0"] = accessorTexcoord0Name;
		if (tex1bufsize > 1)
			attributesJS["TEXCOORD_1"] = accessorTexcoord1Name;
		if (colorbufsize > 0)
			attributesJS["COLOR"] = accessorColorName;
		primitiveSet["attributes"] = attributesJS;
		primitiveSet["attributes"] = attributesJS;
		primitiveSet["material"] = materialName;
		primitiveSet["mode"] = 4;
		geometryJS["primitives"].append(primitiveSet);
	}

	return geometryJS;
}