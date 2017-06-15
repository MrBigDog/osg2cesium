#include "PAGEDLOD2TILESET.h"
#include "Utils.h"
PagedLOD2Tiles::PagedLOD2Tiles()
{
	//setFlipAxis(true);
}
void PagedLOD2Tiles::setTransform(double lat,double lon, double height,osg::Matrixd localTransform)
{
		m_latlonheight = osg::Vec3d(lat, lon,height);
		m_ellipsoidModel.computeLocalToWorldTransformFromLatLongHeight(osg::DegreesToRadians(lat), osg::DegreesToRadians(lon), height, m_local2world);
		m_localTransform = localTransform;
}
PagedLOD2Tiles::~PagedLOD2Tiles()
{

}
//void PagedLOD2Tiles::setFlipAxis(bool flip)
//{
//	/*m_flipAxis = flip;
//	if (flip)
//	{
//		m_flipAxisTransform = osg::Matrixd::rotate(osg::DegreesToRadians(90.0), osg::Vec3(-1, 0, 0));
//	}
//	else
//	{*/
//		//m_flipAxisTransform = osg::Matrixd::identity();
//	//}
//}

osg::Matrixd PagedLOD2Tiles::getTransform()
{
	return m_localTransform /** m_flipAxisTransform * */ * m_local2world;
}

Json::Value PagedLOD2Tiles::createTransformNode(osg::Matrixd& mat)
{
	Json::Value transform(Json::arrayValue);
	for (size_t row = 0; row < 4; row++)
	{
		for (size_t col = 0; col < 4; col++)
		{
			//printf("%f\n", local2world(row, col));
			transform.append(mat(row, col));
		}
	}
	return transform;
}
Json::Value PagedLOD2Tiles::createBoxNode(osg::BoundingBox bb)
{
	osg::Vec3 center = bb.center();
	osg::Vec3 xaxis((bb.xMax() - bb.xMin())*0.5, 0, 0);
	osg::Vec3 yaxis(0, (bb.yMax() - bb.yMin())*0.5, 0);
	osg::Vec3 zaxis(0, 0, (bb.zMax() - bb.zMin())*0.5);

	Json::Value box(Json::arrayValue);
	box.append(center.x()); box.append(center.y()); box.append(center.z());
	box.append(xaxis.x()); box.append(xaxis.y()); box.append(xaxis.z());
	box.append(yaxis.x()); box.append(yaxis.y()); box.append(yaxis.z());
	box.append(zaxis.x()); box.append(zaxis.y()); box.append(zaxis.z());
	return box;
}
Json::Value PagedLOD2Tiles::createRegionNode(osg::BoundingBoxd localbb)
{
	Json::Value region(Json::arrayValue);
	osg::BoundingBoxd geobb;
	geobb.init();
	for (size_t i = 0; i < 8; i++)
	{
		osg::Vec3d world = localbb.corner(i) * m_localTransform /** m_flipAxisTransform*/ * m_local2world;
		double lat, lon, height;
		m_ellipsoidModel.convertXYZToLatLongHeight(world.x(), world.y(), world.z(), lat, lon, height);
		geobb.expandBy(lon, lat, height);
	}
	region.append(geobb.xMin());
	region.append(geobb.yMin());
	region.append(geobb.xMax());
	region.append(geobb.yMax());
	region.append(geobb.zMin());
	region.append(geobb.zMax());
	return region;
}
 
float PagedLOD2Tiles::calGeometricError(float radius, float screenPixels)
{ 
	return (3.1415926 * radius * radius) / screenPixels /*/ 10*/;
	
	// (radius * 2 / screenPixels) * 2;
}

void PagedLOD2Tiles::findPagedLOD(osg::Node* parent, std::vector<osg::PagedLOD*>& pagedLODList)
{
	osg::PagedLOD* lod = dynamic_cast<osg::PagedLOD*>(parent);
	osg::Group* group = dynamic_cast<osg::Group*>(parent);
	if (lod)
	{
		pagedLODList.push_back(lod);
	}
	else if(group && !dynamic_cast<osg::Geode*>(parent))
	{
		for (size_t i = 0; i < group->getNumChildren(); i++)
		{
			findPagedLOD(group->getChild(i), pagedLODList);
		}
	}
}

int PagedLOD2Tiles::createNode(std::string filename,osg::Node* node2, std::string outname,std::string outdir, Json::Value& newJsonNode, Json::Value& parentJsonNode,float geometricError, float& maxGeometricError)
{
	osg::ref_ptr<osg::Node> node;
	if(node2)
	{
		node = node2;
	}
	else
	{
	    node = osgDB::readNodeFile(filename);
		printf("%s\n", filename.data());
	}
	printf("%s\n", filename.data());
	if (!node || !node.valid())
		return 2;
	osg::ref_ptr<osg::PagedLOD> lod = dynamic_cast<osg::PagedLOD*>(node->asNode());
	osg::ref_ptr<osg::Group> group = dynamic_cast<osg::Group*>(node->asNode());
	std::vector<osg::PagedLOD*> childLODS;
	if (lod && lod.valid())
	{
		childLODS.push_back(lod.get());
	}
	else
	{
		findPagedLOD(node, childLODS);
	}

	osg::BoundingBox bb;
	osg::ComputeBoundsVisitor cv;
	node->accept(cv);
	bb = cv.getBoundingBox();
	float range = 0;
	float radius = 0;
	if (childLODS.size() > 0 && childLODS[0]->getRangeList().size() > 1)
	{
		range = childLODS[0]->getRangeList()[0].second;
		radius = childLODS[0]->getRadius();
		geometricError = calGeometricError(radius, range);

		if (maxGeometricError < geometricError)
		{
			//获取节点最大geometricError,用于根节点包围盒使用
			maxGeometricError = geometricError;
		}
	}
	newJsonNode["geometricError"] = geometricError;
	newJsonNode["refine"] = "replace";
	Json::Value content;
	content["url"] = outname + ".b3dm";
	Json::Value boundingVolume;
	//boundingVolume["box"] = createBoxNode(bb);
	boundingVolume["region"] = createRegionNode(bb);
	//content["boundingVolume"] = boundingVolume;
	newJsonNode["boundingVolume"] = boundingVolume;
	newJsonNode["content"] = content;
	if (node.valid() /*&& !node2*/)
	{
		OSG2GLTF osg2gltf;
		//osg2gltf.setFlipAxis(false);
		//osg2gltf.m_flipAxis = m_flipAxis;
		//检测文件是否存在。如存在就不覆盖
		//std::ifstream file(outdir + outname + ".b3dm");
		//if (!file) {
		//	file.close();
		//	osg2gltf.toGLTF(node, outdir, outname, OSG2GLTF::B3DM);
		//}
		//else
		//{
		//  file.close();
		//}
		osg2gltf.setExternalShaderPath("../");
		osg2gltf.toGLTF(node, outdir, outname, OSG2GLTF::B3DM);

	}
	Json::Value children(Json::arrayValue);
	bool hasChild = false;
	for (size_t i = 0; i < childLODS.size(); i++)
	{

		if (childLODS[i]->getNumFileNames() < 2)
			continue;
		hasChild = true;
		std::string childFileName = childLODS[i]->getFileName(1);
		std::string databasepath = childLODS[i]->getDatabasePath();

		Json::Value childJSNode;
		std::string childname = childFileName;
		std::string ext;
		std::string dir;
		Utils::splitFilepath(childFileName, dir, childname, ext);
		dir = databasepath;
		if (databasepath != "")
		{
			childFileName = databasepath + childFileName;
		}
	
		if (createNode(childFileName, NULL, childname, outdir, childJSNode, newJsonNode,geometricError*0.5,maxGeometricError) != 2)
		{
			children.append(childJSNode);
		}
	
	}
	if(hasChild)
		newJsonNode["children"] = children;
	return 1;

}

void PagedLOD2Tiles::toTileSet(osg::Node* node, std::string outdir)
{
	m_outputRoot = outdir;
	Json::Value tilset;
	Json::Value asset;
	asset["version"] = "0.0";
	asset["tilesetVersion"] = "1.2.3";
	Json::Value properties;
	tilset["asset"] = asset;
	tilset["properties"] = properties;
	Json::Value root;
	root["transform"] = createTransformNode(/*m_flipAxisTransform **/ m_localTransform * m_local2world);
	float geometricError = calGeometricError(node->getBound().radius(), 0.5);
	float maxGeometricError = -1;
	//maxGeometricError获取子节点GeometricError最大值，用于根节点包围盒
	createNode("", node, "root", outdir, root, root, geometricError, maxGeometricError);

	tilset["root"] = root;
	Json::StyledWriter sw;
	std::string tilesetContent = sw.write(tilset);
	std::ofstream ofs(outdir + "tileset.json");
	ofs << tilesetContent;
	ofs.close();

}
void PagedLOD2Tiles::toTileSet(osg::Node* node, std::string outdir, float& maxGeometricError)
{
	Json::Value tilset;
	Json::Value asset;
	asset["version"] = "0.0";
	asset["tilesetVersion"] = "1.2.3";
	Json::Value properties;
	tilset["asset"] = asset;
	tilset["properties"] = properties;
	Json::Value root;
	//root["transform"] = createTransformNode(m_flipAxisTransform);
	float geometricError = calGeometricError(node->getBound().radius(), 0.5);
	//maxGeometricError获取子节点GeometricError最大值，用于根节点包围盒
	createNode("", node, "root", outdir, root, root, geometricError, maxGeometricError);
	root["geometricError"] = maxGeometricError * 1.5;
	tilset["root"] = root;
	Json::StyledWriter sw;
	std::string tilesetContent = sw.write(tilset);
	std::ofstream ofs(outdir + "tileset.json");
	ofs << tilesetContent;
	ofs.close();

}
void PagedLOD2Tiles::toTileSet(std::string indir, std::string outdir)
{
	m_outputRoot = outdir;
	Json::Value tilset;
	Json::Value asset;
	asset["version"] = "0.0";
	asset["tilesetVersion"] = "1.2.3";
	Json::Value properties;
	tilset["asset"] = asset;
	tilset["properties"] = properties;
	Json::Value root;
	root["transform"] = createTransformNode(/*m_flipAxisTransform **/  m_localTransform * m_local2world);

	std::vector<std::string> dirs = Utils::findSubdirs(indir + "*");
	osg::BoundingBox bound;
	bound.init();
	CreateDirectoryA(outdir.data(), NULL);
	Json::Value children(Json::arrayValue);
	//获取子节点GeometricError最大值，用于根节点包围盒
	float maxGeometricError = -1;
	for (size_t i = 0; i < dirs.size(); i++)
	{
		if (dirs[i] == "." || dirs[i] == "..")
			continue;
		std::string dirname = dirs[i];
		std::string nodefile = indir + dirs[i] + "/" + dirs[i] + ".osgb";
		osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(nodefile);
		if (!node || !node.valid())
			continue;
		std::string subdir = outdir + dirname + "/";
		CreateDirectoryA(subdir.data(), NULL);
		osg::BoundingBox bb;
		osg::ComputeBoundsVisitor cv;
		node->accept(cv);
		bb = cv.getBoundingBox();
		bound.expandBy(bb);
		Json::Value childnode;
		float geometricError = -1;
		//createNode("", group->getChild(i), childname, outdir, childnode, childnode, geometricError * 0.5, maxGeometricError);
		toTileSet(node, subdir, geometricError);
		childnode["refine"] = "add";
		childnode["geometricError"] = geometricError;
		//获取子节点GeometricError最大值，用于根节点包围盒
		if (maxGeometricError < geometricError)
			maxGeometricError = geometricError;
		Json::Value content;
		content["url"] = dirname + "/tileset.json";
		Json::Value boundingVolume;
		//boundingVolume["box"] = createBoxNode(bb);
		boundingVolume["region"] = createRegionNode(bb);
		//content["boundingVolume"] = boundingVolume;
		childnode["boundingVolume"] = boundingVolume;
		childnode["content"] = content;
		children.append(childnode);

	}
/*

	float geometricError = calGeometricError(group->getBound().radius(), 0.5);
	float maxGeometricError = -1;

	for (size_t i = 0; i < group->getNumChildren(); i++)
	{
		std::string childname;
		std::string ext;
		std::string dir;
		Utils::splitFilepath(group->getChild(i)->getName(), dir, childname, ext);
		Json::Value childnode;
		createNode("", group->getChild(i), childname, outdir, childnode, childnode, geometricError * 0.5, maxGeometricError);
		childnode["refine"] = "replace";
		children.append(childnode);
	}*/
	root["geometricError"] = maxGeometricError * 2;
	Json::Value boundingVolume;
	boundingVolume["region"] = createRegionNode(bound);
	root["boundingVolume"] = boundingVolume;
	root["refine"] = "add";

	root["children"] = children;
	tilset["root"] = root;
	Json::StyledWriter sw;
	std::string tilesetContent = sw.write(tilset);
	std::ofstream ofs(outdir + "tileset.json");
	ofs << tilesetContent;
	ofs.close();

}
