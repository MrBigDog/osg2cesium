#include "OSG2GLTF.h"
#include "osgDB/ReadFile" 
void OSG2GLTF::getMaterial(osg::StateSet* stateset, osg::Material*& mat, osg::Texture2D*& tex)
{
	if (!stateset)
		return;
	osg::Material* thismat = dynamic_cast<osg::Material*>(stateset->getAttribute(osg::StateAttribute::Type::MATERIAL));
	if (thismat)
	{
		mat = thismat;
	}

	osg::Texture2D* thistex = dynamic_cast<osg::Texture2D*>(stateset->getTextureAttribute(0, osg::StateAttribute::Type::TEXTURE));
	if (thistex)
	{
		tex = thistex;
	}

}

Json::Value OSG2GLTF::createMaterialNode(osg::Material* mat, std::string name, std::string texname)
{

	Json::Value matnode;
	matnode["name"] = name;
	matnode["technique"] = "technique0";
	Json::Value values;
	Json::Value diffuse(Json::arrayValue);
	Json::Value specular(Json::arrayValue);
	Json::Value emission(Json::arrayValue);
	osg::Vec4 diffuseColor(1, 1, 1, 1);
	osg::Vec4 specularColor(0.2, 0.2, 0.2, 1);
	float shininess = 256;
	if (mat)
	{
		diffuseColor = mat->getDiffuse(osg::Material::FRONT);
		//if (diffuseColor.b() != diffuseColor.g())
		//printf("%f,%f,%f", diffuseColor.r(), diffuseColor.g(), diffuseColor.b());
		specularColor = mat->getSpecular(osg::Material::FRONT);
		shininess = mat->getShininess(osg::Material::FRONT);
		diffuse.append(diffuseColor.r()); diffuse.append(diffuseColor.g()); diffuse.append(diffuseColor.b()); diffuse.append(diffuseColor.a());
		specular.append(specularColor.r()); specular.append(specularColor.g()); specular.append(specularColor.b()); specular.append(specularColor.a());

		//osg::Vec4 emissionColor = mat->getEmission(osg::Material::FRONT);
		//emission.append(emissionColor.r()); emission.append(emissionColor.g()); emission.append(emissionColor.b()); emission.append(emissionColor.a());
	}
	else
	{
		diffuse.append(1); diffuse.append(1); diffuse.append(1); diffuse.append(1);
		specular.append(0.2); specular.append(0.2); specular.append(0.2); specular.append(1);
	}

	values["diffuse"] = diffuse;
	values["diffuseTex"] = texname;
	values["specular"] = specular;
	//values["emission"] = emission;
	values["shininess"] = shininess;
	matnode["values"] = values;

	return matnode;

}
Json::Value OSG2GLTF::createNode(osg::Node* node, osg::StateSet* parentStateSet)
{

	///getMaterial(node->getStateSet(),mat,tex);

	//node->getOrCreateStateSet()->setAttributeAndModes(material, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
	osg::ref_ptr<osg::StateSet> curStateSet = new osg::StateSet(*node->getOrCreateStateSet());
	if (parentStateSet)
	{
		curStateSet->merge(*parentStateSet);
	}
	osg::MatrixTransform* matnode = dynamic_cast<osg::MatrixTransform*>(node);
	osg::Group* groupnode = dynamic_cast<osg::Group*>(node);
	osg::Geode* geode = dynamic_cast<osg::Geode*>(node);
	Json::Value me;

	if (node->getName() == "")
	{
		node->setName(getPointer(node));
	}
	me["name"] = node->getName();

	if (geode)
	{
		Json::Value jsmeshes(Json::arrayValue);
		for (int i = 0; i < geode->getNumDrawables(); i++)
		{
			osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode->getDrawable(i));
			if (!geom)
				continue;
			osg::ref_ptr<osg::StateSet> geomStateSet = new osg::StateSet(*geom->getOrCreateStateSet());
			geomStateSet->merge(*curStateSet);

			unsigned int GL_BLEND_ENABLED = geomStateSet->getMode(GL_BLEND);
			bool blendEnabled = false;
			if (GL_BLEND_ENABLED & osg::StateAttribute::Values::ON)
			    blendEnabled = true;
			osg::Material* mat = NULL;
			osg::Texture2D* tex = NULL;
			getMaterial(geom->getStateSet(), mat, tex);
			std::string materialname = "";
			if (geom->getName() == "")
			{
				geom->setName(getPointer(geom));
			}
			if (tex && tex->getImage(0))
			{
				materialname = tex->getImage(0)->getName();

				if (m_materials.find(materialname) == m_materials.end())
				{
					m_materials[materialname] = createMaterialNode(NULL, materialname, materialname);
				}
			}
			else if (mat)
			{
				materialname = getPointer(mat);
				if (m_materials.find(materialname) == m_materials.end())
				{
					m_materials[materialname] = createMaterialNode(mat, materialname);
				}
			}
			if (blendEnabled)
			{
				m_materials[materialname]["technique"] = "technique1";
				m_TransparentTechnique = createTechniqueNode(true, m_type);
				m_techniques["technique1"] = m_TransparentTechnique;
			}
			else
			{
				m_materials[materialname]["technique"] = "technique0";
				m_Technique = createTechniqueNode(false, m_type);
				m_techniques["technique0"] = m_Technique;
			}
			if (m_meshes.find(geom->getName()) == m_meshes.end())
			{
				bool usebatch = false;
				if (m_type == B3DM)
					usebatch = true;
				m_meshes[geom->getName()] = m_GeometryVisitor.m_GeometryMap[geom]->createJsonMeshNode(geom->getName(), materialname, m_accessors, m_indexBufferData, m_vertexBufferData, m_batchIdBufferData);
			}
			//printf("%s\n", geom->getName().data());
			jsmeshes.append(geom->getName());
		}
		me["meshes"] = jsmeshes;
	}
	if (groupnode && !geode)
	{
		Json::Value jschildren(Json::arrayValue);
		for (int ichild = 0; ichild < groupnode->getNumChildren(); ichild++)
		{
			if (groupnode->getChild(ichild)->getName() == "")
			{
				groupnode->getChild(ichild)->setName(getPointer(groupnode->getChild(ichild)));
			}
			jschildren.append(groupnode->getChild(ichild)->getName());
			createNode(groupnode->getChild(ichild), curStateSet);
		}
		me["children"] = jschildren;
	}
	if (matnode)
	{
		const osg::Matrix& mat = matnode->getMatrix();
		if (!mat.isIdentity())
		{

			Json::Value jsmatrix(Json::arrayValue);
			for (int col = 0; col < 4; col++)
			{
				for (int row = 0; row < 4; row++)
				{
					jsmatrix.append(mat(row, col));
				}
			}
			me["matrix"] = jsmatrix;
		}
	}

	m_nodes[node->getName()] = me;
	m_nodelist.push_back(std::pair<std::string, Json::Value>(node->getName(), me));
	return me;
}
void OSG2GLTF::attachNode(Json::Value& root, std::string name1, Json::Value val)
{
	root[name1] = val;
}
void OSG2GLTF::attachNode(Json::Value& root, std::string name1, std::string name2, Json::Value val)
{
	Json::Value node;
	node[name2] = val;
	root[name1] = node;
}
void OSG2GLTF::attachNode(Json::Value& root, std::string name1, std::vector<std::string> name2, std::vector<Json::Value> val)
{
	Json::Value node;
	for (size_t i = 0; i < name2.size(); i++)
	{
		node[name2[i]] = val[i];
	}
	root[name1] = node;
}
Json::Value OSG2GLTF::createTechniqueNode(bool istransparent, FileType type)
{
	Json::Value node;

	Json::Value attributes;

	attributes["a_normal"] = Json::Value("normal");
	attributes["a_position"] = Json::Value("position");
	attributes["a_color"] = Json::Value("color");
	attributes["a_texcoord0"] = Json::Value("texcoord0");
	if (type == B3DM /*&& m_batchIdBufferData.size() > 0*/)
	{
		//"a_batchId":"batchId"
		attributes["a_batchId"] = Json::Value("batchId");
	}
	node["attributes"] = attributes;

	Json::Value parameters;
	attachNode(parameters, "diffuse",
		"type", Json::Value(35666));

	attachNode(parameters, "diffuseTex",
		"type", Json::Value(35678));
	std::vector<std::string> strings;
	std::vector<Json::Value> jsonvalues;

	strings.clear();
	jsonvalues.clear();
	strings.push_back("semantic"); strings.push_back("type");
	jsonvalues.push_back("MODELVIEW"); jsonvalues.push_back(35676);
	attachNode(parameters, "modelViewMatrix", strings, jsonvalues);

	strings.clear();
	jsonvalues.clear();
	strings.push_back("semantic"); strings.push_back("type");
	jsonvalues.push_back("NORMAL"); jsonvalues.push_back(35665);
	attachNode(parameters, "normal", strings, jsonvalues);

	strings.clear();
	jsonvalues.clear();
	strings.push_back("semantic"); strings.push_back("type");
	jsonvalues.push_back("MODELVIEWINVERSETRANSPOSE"); jsonvalues.push_back(35675);
	attachNode(parameters, "normalMatrix", strings, jsonvalues);

	strings.clear();
	jsonvalues.clear();
	strings.push_back("semantic"); strings.push_back("type");
	jsonvalues.push_back("POSITION"); jsonvalues.push_back(35665);
	attachNode(parameters, "position", strings, jsonvalues);

	strings.clear();
	jsonvalues.clear();
	strings.push_back("semantic"); strings.push_back("type");
	jsonvalues.push_back("PROJECTION"); jsonvalues.push_back(35676);
	attachNode(parameters, "projectionMatrix", strings, jsonvalues);

	attachNode(parameters, "shininess",
		"type", Json::Value(5126));

	attachNode(parameters, "specular",
		"type", Json::Value(35666));

	// "color": {
	//  "semantic": "COLOR",
	//  "type": 35665
	//},

	strings.clear();
	jsonvalues.clear();
	strings.push_back("semantic"); strings.push_back("type");
	jsonvalues.push_back("COLOR"); jsonvalues.push_back(35666);
	attachNode(parameters, "color", strings, jsonvalues);

	strings.clear();
	jsonvalues.clear();
	strings.push_back("semantic"); strings.push_back("type");
	jsonvalues.push_back("TEXCOORD_0"); jsonvalues.push_back(35664);
	attachNode(parameters, "texcoord0", strings, jsonvalues);
	if (type == B3DM /*&& m_batchIdBufferData.size() > 0*/)
	{
		strings.clear();
		jsonvalues.clear();
		strings.push_back("semantic"); strings.push_back("type");
		jsonvalues.push_back("_BATCHID"); jsonvalues.push_back(5123);
		attachNode(parameters, "batchId", strings, jsonvalues);
	}

	node["parameters"] = parameters;

	node["program"] = "program_0";
	Json::Value states;
	Json::Value states_enable(Json::arrayValue);
	states_enable.append(2929);
	states_enable.append(2884);
	if (istransparent)
	{
		states_enable.append(3042);
	}
	states["enable"] = states_enable;
	node["states"] = states;
	if (istransparent)
	{
		Json::Value functions;
		Json::Value blendEquationSeparate(Json::arrayValue);
		Json::Value blendFuncSeparate(Json::arrayValue);
		blendEquationSeparate.append(32774); blendEquationSeparate.append(32774);
		//[768, 774, 770, 771]

		blendFuncSeparate.append(768); blendFuncSeparate.append(774); blendFuncSeparate.append(770); blendFuncSeparate.append(771);
		functions["blendEquationSeparate"] = blendEquationSeparate;
		functions["blendFuncSeparate"] = blendFuncSeparate;
		node["functions"] = functions;
	}

	//"program": "program_0",
	//	"states" : {
	//	"enable": [
	//		2929,
	//			2884
	//	]
	//},


	Json::Value uniforms;
	uniforms["u_diffuse"] = Json::Value("diffuse");
	uniforms["u_diffuseTex"] = Json::Value("diffuseTex");
	uniforms["u_modelViewMatrix"] = Json::Value("modelViewMatrix");
	uniforms["u_normalMatrix"] = Json::Value("normalMatrix");
	uniforms["u_projectionMatrix"] = Json::Value("projectionMatrix");
	uniforms["u_shininess"] = Json::Value("shininess");
	uniforms["u_specular"] = Json::Value("specular");


	node["uniforms"] = uniforms;
	return node;
	//root[_name] = node;
}
Json::Value OSG2GLTF::createBuffer(std::string outdir, std::string buffername, FileType type)
{

	Json::Value buffer;
	buffer["byteLength"] = m_indexBufferData.size() + m_vertexBufferData.size() + m_imageBufferData.size() + m_shaderBufferData.size();
	buffer["type"] = "arraybuffer";


	buffer["uri"] = buffername + ".bin";
	if (type != GLTF)
	{
		buffer["uri"] = "data:,";
	}

	return buffer;
}
Json::Value OSG2GLTF::createIndexBufferView(std::string buffername)
{

	Json::Value bufferViewIndex;
	std::string bufferViewIndexName = "bufferViewIndex";
	int indexBufferViewOffset = 0;
	//int vertexBufferViewOffset = vertexBufferData.size();
	bufferViewIndex["buffer"] = buffername;
	bufferViewIndex["byteLength"] = m_indexBufferData.size();
	bufferViewIndex["byteOffset"] = 0;
	bufferViewIndex["target"] = 34963;//34963 (ELEMENT_ARRAY_BUFFER)
	return bufferViewIndex;
}
Json::Value OSG2GLTF::createVertexBufferView(std::string buffername)
{

	Json::Value bufferViewVertex;
	bufferViewVertex["buffer"] = buffername;
	bufferViewVertex["byteLength"] = m_vertexBufferData.size();
	bufferViewVertex["byteOffset"] = m_indexBufferData.size();
	bufferViewVertex["target"] = 34962;//34963 (ARRAY_BUFFER)

	return bufferViewVertex;
}

Json::Value OSG2GLTF::createbatchIdBufferView(std::string buffername)
{

	Json::Value bufferViewVertex;
	bufferViewVertex["buffer"] = buffername;
	bufferViewVertex["byteLength"] = m_batchIdBufferData.size();
	bufferViewVertex["byteOffset"] = m_vertexBufferData.size() + m_indexBufferData.size();
	bufferViewVertex["target"] = 34962;

	return bufferViewVertex;
}


void OSG2GLTF::writeImages(std::string outdir)
{
	std::map<std::string, osg::Image* >::iterator iter = m_GeometryVisitor.m_ImageMap.begin();

	while (iter != m_GeometryVisitor.m_ImageMap.end())
	{
		std::string outname = outdir + iter->first;
		//if (iter->second->getPixelFormat() == GL_RGBA)
		//{
		iter->second->flipVertical();
		//}
		osgDB::writeImageFile(*iter->second, outname);
		//	osgDB::writeImageFile(*iter->second, "B:/cesium-3d-tiles/Specs/Data/Cesium3DTiles/Tilesets/osgdata59/007f6230-36dd-11e7-05ff-3401306d147.jpg");

		iter++;
	}

}
char* OSG2GLTF::readBinary(std::string filename, size_t& len)
{
	std::ifstream is;
	is.open(filename, std::ios::binary);
	// get length of file:
	is.seekg(0, is.end);
	len = is.tellg();
	is.seekg(0, is.beg);
	// allocate memory:
	char * buffer = new char[len];
	// read data as a block:
	is.read(buffer, len);
	is.close();
	return buffer;
}
Json::Value OSG2GLTF::createImages(std::string outdir, std::string buffername, FileType type)
{
	writeImages(outdir);
	Json::Value images;
	std::map<std::string, Json::Value>::iterator iter = m_GeometryVisitor.m_Images.begin();
	size_t imagebufOffset = m_vertexBufferData.size() + m_indexBufferData.size();
	if (type == B3DM && m_batchIdBufferData.size() > 0)
		imagebufOffset += m_batchIdBufferData.size();
	char * buffer;
	while (iter != m_GeometryVisitor.m_Images.end())
	{
		std::string name = iter->first;
		if (type == GLTF)
		{
			images[name] = iter->second;
			iter++;
			continue;
		}
		std::string imagefilename = outdir + name;
		size_t length;
		buffer = readBinary(imagefilename, length);
		for (size_t i = 0; i < length; i++)
		{
			m_imageBufferData.push_back(buffer[i]);
		}
		delete[] buffer;
		Json::Value imageNode;

		imageNode["name"] = name;
		imageNode["uri"] = "data:,";
		Json::Value dataNode;
		Json::Value bufferViewImage;
		std::string bufferViewImageName = "bufferView_image_" + name;
		bufferViewImage["buffer"] = buffername;
		bufferViewImage["byteLength"] = length;
		bufferViewImage["byteOffset"] = imagebufOffset;
		m_bufferViews[bufferViewImageName] = bufferViewImage;
		imagebufOffset += length;
		Json::Value image_extensions;
		Json::Value image_KHR_binary_glTF;
		image_KHR_binary_glTF["bufferView"] = bufferViewImageName;
		osg::Image* img = m_GeometryVisitor.m_ImageMap[name];
		if (img->getPixelFormat() == GL_RGB)
		{
			image_KHR_binary_glTF["mimeType"] = "image/jpeg";
		}
		else
		{
			image_KHR_binary_glTF["mimeType"] = "image/png";
		}
		image_KHR_binary_glTF["width"] = img->s();
		image_KHR_binary_glTF["height"] = img->t();
		image_extensions["KHR_binary_glTF"] = image_KHR_binary_glTF;
		imageNode["extensions"] = image_extensions;


		//"mimeType":"image/jpeg",
		//	"width" : 211,
		//	"height" : 211
		images[name] = imageNode;
		iter++;
	}

	return images;
}
Json::Value OSG2GLTF::createShader(std::string outdir, std::string uridir, std::string shaderFile, const std::string& shaderSource, int shaderType, std::string buffername, FileType type)
{

	Json::Value shader;
	shader["type"] = shaderType;
	if (type == GLTF || outdir != "")
	{
		shader["uri"] = uridir + shaderFile;
		std::ofstream ofs(outdir + shaderFile);
		ofs << shaderSource;
		ofs.close();
	}
	else
	{

		//char * buffer;
		//size_t length;
		//buffer = readBinary(m_resourcesDir + shaderFile, length);
		//for (size_t i = 0; i < length; i++)
		//{
		//	m_shaderBufferData.push_back(buffer[i]);
		//}
		//delete[] buffer;


		const char * buffer = shaderSource.data();
		size_t length = shaderSource.size();
		for (size_t i = 0; i < length; i++)
		{
			m_shaderBufferData.push_back(buffer[i]);
		}
		shader["uri"] = "data:,";
		Json::Value dataNode;
		Json::Value bufferView;
		std::string bufferViewName = "bufferView_shader_" + shaderFile;
		bufferView["buffer"] = buffername;
		bufferView["byteLength"] = length;
		int offset = m_vertexBufferData.size() + m_indexBufferData.size() + m_imageBufferData.size() + m_shaderBufferData.size() - length;
		if (type == B3DM && m_batchIdBufferData.size() > 0)
			offset += m_batchIdBufferData.size();
		bufferView["byteOffset"] = offset;

		m_bufferViews[bufferViewName] = bufferView;

		Json::Value extensions;
		Json::Value KHR_binary_glTF;
		KHR_binary_glTF["bufferView"] = bufferViewName;
		extensions["KHR_binary_glTF"] = KHR_binary_glTF;
		shader["extensions"] = extensions;

	}


	return shader;


	//"uri":"data:,",
	//	"extensions" : {
	//	"KHR_binary_glTF":{
	//		"bufferView":"binary_bufferView1"
	//	}
	//}
	//gltf["images"] = images;
	//"extensionsUsed":[
	//	"KHR_binary_glTF"
	//]
}
void OSG2GLTF::writeGLTF(std::string outdir, std::string outname, std::string buffername, Json::Value& gltf, FileType type)
{
	Json::StyledWriter sw;
	std::string gltfContent = sw.write(gltf);

	if (type == GLTF)
	{
		std::ofstream ofs_gltf(outdir + outname + ".gltf");
		ofs_gltf << gltfContent;
		ofs_gltf.close();
		std::ofstream ofs_bin(outdir + outname + ".bin", std::ios::out | std::ios::binary);
		if (m_indexBufferData.size() > 0)
			ofs_bin.write(&m_indexBufferData[0], m_indexBufferData.size());
		if (m_vertexBufferData.size() > 0)
			ofs_bin.write(&m_vertexBufferData[0], m_vertexBufferData.size());
		ofs_bin.close();
		return;
	}

	std::string outbinaryfile = outdir + outname + ".glb";
	if (type == B3DM)
		outbinaryfile = outdir + outname + ".b3dm";
	GLB_HEADER glb_header = GLB_HEADER();
	size_t gltlen = gltfContent.size();
	const char* gltfbuf = gltfContent.data();
	glb_header.contentLength = gltlen;

	char pad[4];
	memset(pad, ' ', 4);
	int numpads = 0;
	while ((20 + gltlen + numpads) % 4 != 0)
	{
		numpads++;
	}

	size_t bodylen = m_indexBufferData.size() + m_vertexBufferData.size() + m_imageBufferData.size() + m_shaderBufferData.size();
	if (type == B3DM && m_batchIdBufferData.size() > 0)
	{
		bodylen += m_batchIdBufferData.size();
	}
	glb_header.contentLength = gltlen + numpads;
	glb_header.length = 20 + glb_header.contentLength + bodylen;

	std::ofstream ofs(outbinaryfile, std::ios::out | std::ios::binary);

	if (type == B3DM)
	{
		Batched3DModelHeader b3dm_header;
		std::string featureTB = "";
		std::string batchTB = "";
		if (m_GeometryVisitor.m_Batches.size() > 0)
		{
			featureTB = sw.write(createFeatureTable());
			batchTB = sw.write(createBatchTable());
			b3dm_header.featureTableJSONByteLength = featureTB.size();
			b3dm_header.batchTableJSONByteLength = batchTB.size();
		}

		b3dm_header.byteLength = glb_header.length + 28 + b3dm_header.featureTableJSONByteLength + b3dm_header.batchTableJSONByteLength;
		ofs.write((char*)&b3dm_header, 28);
		if (b3dm_header.featureTableJSONByteLength > 0)
		{
			ofs.write(featureTB.data(), featureTB.size());
			ofs.write(batchTB.data(), batchTB.size());
		}
		//std::string gltfContent = sw.write(gltf);
	}
	ofs.write((char*)&glb_header, 20);
	ofs.write(gltfbuf, gltlen);
	if (numpads > 0)
	{
		ofs.write(pad, numpads);
	}
	if (m_indexBufferData.size() > 0)
		ofs.write(&m_indexBufferData[0], m_indexBufferData.size());
	if (m_vertexBufferData.size() > 0)
		ofs.write(&m_vertexBufferData[0], m_vertexBufferData.size());
	if (type == B3DM && m_batchIdBufferData.size() > 0)
		ofs.write(&m_batchIdBufferData[0], m_batchIdBufferData.size());
	if (m_imageBufferData.size() > 0)
		ofs.write(&m_imageBufferData[0], m_imageBufferData.size());
	if (m_shaderBufferData.size() > 0)
		ofs.write(&m_shaderBufferData[0], m_shaderBufferData.size());
	//ofs.write(bodybuf, bodylen);
	ofs.close();
	std::map<std::string, osg::Image*>::iterator iter = m_GeometryVisitor.m_ImageMap.begin();

	while (iter != m_GeometryVisitor.m_ImageMap.end())
	{
		std::string outimagefile = outdir + iter->first;
		remove(outimagefile.data());
		iter++;
	}
}
Json::Value OSG2GLTF::createBatchTable()
{
	Json::Value batchtb;

	std::map<std::string, BatchInfo> masterTableByName = GeometryVisitor::getMasterBatchTable();
	std::map<int, BatchInfo> masterTableByID;

	std::map<std::string, BatchInfo>::iterator iter = masterTableByName.begin();
	int maxID = 0;
	while (iter != masterTableByName.end())
	{
		masterTableByID[iter->second.uniqueID] = iter->second;
		if (maxID < iter->second.uniqueID)
			maxID = iter->second.uniqueID;
		iter++;
	}
	Json::Value desc(Json::arrayValue);
	Json::Value root(Json::arrayValue);
	Json::Value component(Json::arrayValue);
	Json::Value id(Json::arrayValue);
	int numbatches = maxID + 1;
	for (int n = 0; n < numbatches; n++)
	{
		std::map<int, BatchInfo>::iterator iterByID = masterTableByID.find(n);
		if (iterByID == masterTableByID.end())
		{
			root.append("");
			component.append("");
			desc.append("");
			id.append("");
		}
		else
		{
			BatchInfo info = iterByID->second;
			id.append(info.rootName + "_" + info.parentName);
			desc.append(info.description);
			root.append(info.rootName);
			component.append(info.parentName);
		}
	}

	//id.append("");
	batchtb["id"] = id;
	batchtb["desc"] = desc;
	batchtb["root"] = root;
	batchtb["component"] = component;
	return batchtb;
}

Json::Value OSG2GLTF::createFeatureTable()
{
	Json::Value featuretb;
	std::map<std::string, BatchInfo> masterTable = GeometryVisitor::getMasterBatchTable();
	featuretb["BATCH_LENGTH"] = masterTable.size();
	return featuretb;
}


std::string OSG2GLTF::getPointer(void* ptr)
{
	std::stringstream ss;
	ss << ptr;
	std::string str = ss.str();
	return str;
}

void OSG2GLTF::toGLTF(osg::Node* osgNode, std::string outdir, std::string outname, FileType type)
{
	osg::ref_ptr<osg::Node> dummyNode;
	/*if (m_flipAxis)
	{
		dummyNode = new osg::MatrixTransform;
		((osg::MatrixTransform*)dummyNode->asNode())->addChild(osgNode);
		((osg::MatrixTransform*)dummyNode->asNode())->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(90.0),osg::Vec3(1,0,0)));
	}
	else
	{*/
		dummyNode = osgNode;
	//}

	if (dummyNode->getName() == "")
	{
		dummyNode->setName(outname);
	}
	m_type = type;
	m_indexBufferData.clear();
	m_vertexBufferData.clear();
	m_batchIdBufferData.clear();
	m_materials.clear();
	m_bufferViews.clear();
	m_accessors.clear();
	m_buffers.clear();
	m_meshes.clear();
	m_nodes.clear();
	m_imageBufferData.clear();
	m_shaderBufferData.clear();
	m_techniques.clear();
	GeometryVisitor::clearBatchTable();
	std::string buffername = outname;
	if (type != GLTF)
		buffername = "binary_glTF";
	dummyNode->accept(m_GeometryVisitor);

	//if (m_GeometryVisitor.m_ImageMap.size() > 0)
	//	return;
	m_Technique = createTechniqueNode(false, type);
	m_TransparentTechnique = createTechniqueNode(true, type);
	Json::Value gltf;
	Json::Value rootNode = createNode(dummyNode);

	Json::Value accessors;
	std::map<std::string, Json::Value>::iterator iter = m_accessors.begin();
	while (iter != m_accessors.end())
	{
		accessors[iter->first] = iter->second;
		iter++;
	}
	gltf["accessors"] = accessors;

	Json::Value bufferViews;
	std::string indexBufferViewName = "bufferView_index";
	std::string vertexBufferViewName = "bufferView_vertex";
	std::string batchIdBufferViewName = "bufferView_batchId";
	if (m_indexBufferData.size() > 0)
		m_bufferViews[indexBufferViewName] = createIndexBufferView(buffername);
	m_bufferViews[vertexBufferViewName] = createVertexBufferView(buffername);
	if (type == B3DM && m_batchIdBufferData.size() > 0)
	{
		m_bufferViews[batchIdBufferViewName] = createbatchIdBufferView(buffername);
	}

	Json::Value imagesNode = createImages(outdir, buffername, type);
	Json::Value shaders;
	shaders[m_vsShaderFileName] = createShader(outdir + m_externalShaderPath, m_externalShaderPath, m_vsShaderFileName, VertexShader_Textured, 35633, buffername, type);
	shaders[m_fsShaderFileName] = createShader(outdir + m_externalShaderPath, m_externalShaderPath, m_fsShaderFileName, FragmentShader_Textured, 35632, buffername, type);


	iter = m_bufferViews.begin();
	while (iter != m_bufferViews.end())
	{
		bufferViews[iter->first] = iter->second;
		iter++;
	}

	gltf["bufferViews"] = bufferViews;


	Json::Value buffers;
	/*iter = m_buffers.begin();
	while (iter != m_buffers.end())
	{
	buffers[iter->first] = iter->second;
	iter++;
	}*/

	buffers[buffername] = createBuffer(outdir, buffername, type);
	gltf["buffers"] = buffers;
	if (m_GeometryVisitor.m_Images.size() > 0)
		gltf["images"] = imagesNode;

	Json::Value materials;
	iter = m_materials.begin();
	while (iter != m_materials.end())
	{
		materials[iter->first] = iter->second;
		iter++;
	}
	gltf["materials"] = materials;

	Json::Value meshes;
	iter = m_meshes.begin();
	while (iter != m_meshes.end())
	{
		meshes[iter->first] = iter->second;
		iter++;
	}
	gltf["meshes"] = meshes;

	Json::Value nodes;
	iter = m_nodes.begin();
	for (size_t i = 0; i < m_nodelist.size(); i++)
	{
		nodes[m_nodelist[i].first] = m_nodelist[i].second;
	}
	//{
	//	nodes[iter->first] = iter->second;
	//	iter++;
	//}
	gltf["nodes"] = nodes;

	Json::Value samplers;
	iter = m_GeometryVisitor.m_Samplers.begin();
	while (iter != m_GeometryVisitor.m_Samplers.end())
	{
		samplers[iter->first] = iter->second;
		iter++;
	}
	if (m_GeometryVisitor.m_Samplers.size() > 0)
		gltf["samplers"] = samplers;

	Json::Value programs;
	Json::Value program_0;

	Json::Value programs_attributes(Json::arrayValue);
	programs_attributes.append("a_normal");
	programs_attributes.append("a_position");
	programs_attributes.append("a_color");
	programs_attributes.append("a_texcoord0");
	if (type == B3DM && m_batchIdBufferData.size() > 0)
	{
		programs_attributes.append("a_batchId");
	}
	program_0["attributes"] = programs_attributes;
	program_0["vertexShader"] = Json::Value(m_vsShaderFileName);
	program_0["fragmentShader"] = Json::Value(m_fsShaderFileName);
	programs["program_0"] = program_0;
	gltf["programs"] = programs;

	gltf["scene"] = "defaultScene";

	Json::Value scenes;
	Json::Value defaultScene;
	Json::Value defaultSceneNodes(Json::arrayValue);
	//defaultSceneNodes.append(osgNode->getName());
	defaultSceneNodes.append(dummyNode->getName());
	defaultScene["nodes"] = defaultSceneNodes;
	scenes["defaultScene"] = defaultScene;
	gltf["scenes"] = scenes;

	gltf["shaders"] = shaders;

	//TexturedVS
	Json::Value techniques;

	Json::Value technique0 = createTechniqueNode(false, type);
	iter = m_techniques.begin();
	while (iter != m_techniques.end())
	{
		techniques[iter->first] = iter->second;
		iter++;
	}
	//techniques["technique0"] = technique0;
	gltf["techniques"] = techniques;

	Json::Value textures;
	iter = m_GeometryVisitor.m_Textures.begin();
	while (iter != m_GeometryVisitor.m_Textures.end())
	{
		textures[iter->first] = iter->second;
		iter++;
	}
	if (m_GeometryVisitor.m_Textures.size() > 0)
		gltf["textures"] = textures;
	if (type != GLTF)
	{
		Json::Value extensionsUsed(Json::arrayValue);
		extensionsUsed.append("KHR_binary_glTF");
		gltf["extensionsUsed"] = extensionsUsed;
	}

	writeGLTF(outdir, outname, buffername, gltf, type);

	std::map<osg::Geometry*, GeometryWrapper*>::iterator iterGeoms = m_GeometryVisitor.m_GeometryMap.begin();
	while (iterGeoms != m_GeometryVisitor.m_GeometryMap.end())
	{
		if (iterGeoms->second->tri_positions.size() > 65535)
		{
			printf("warning: %s顶点个数大于65535\n", (outdir + outname).data());
		}
		iterGeoms++;
	}
}
void OSG2GLTF::toGLTF(std::string filename, std::string outdir, std::string outname, FileType type)
{
	m_type = type;
	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(filename);
	if (node.valid())
	{
		toGLTF(node, outdir, outname, type);
	}
}

//void OSG2GLTF::setFlipAxis(bool flip)
//{
//	m_flipAxis = flip;
//}

void OSG2GLTF::setExternalShaderPath(std::string shaderPath)
{
	m_externalShaderPath = shaderPath;
}
