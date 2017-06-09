#include "PAGEDLOD2TILESET.h"
#include "Utils.h"
PagedLOD2Tiles::PagedLOD2Tiles()
{
	m_flipAxis = true;
}
void PagedLOD2Tiles::setTransform(double lat,double lon, double height,osg::Matrix localTransform)
{
		m_latlonheight = osg::Vec3d(lat, lon,height);
		//m_referenceCenter = osg::Vec3(-368.594970703125f,
		//	18.727205276489258f,
		//	1614.8956298828125f);
		m_ellipsoidModel.computeLocalToWorldTransformFromLatLongHeight(osg::DegreesToRadians(lat), osg::DegreesToRadians(lon), height, m_local2world);
		m_localTransform = localTransform;
}
PagedLOD2Tiles::~PagedLOD2Tiles()
{

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
		osg::Vec3d world = localbb.corner(i) * m_localTransform * m_local2world;
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
osg::BoundingBox PagedLOD2Tiles::flip(osg::BoundingBox bb)
{
	osg::Vec3 bbmin(bb.xMin(), bb.zMin(), -bb.yMin());
	osg::Vec3 bbmax(bb.xMax(), bb.zMax(), -bb.yMax());
	osg::BoundingBox newbb(bbmin, bbmax);

	return newbb;
	//pos = osg::Vec3(pos.x(), pos.z(), -pos.y());
}
float PagedLOD2Tiles::calGeometricError(float radius, float screenPixels)
{
	return (3.1415926 * radius * radius) / screenPixels;// (radius * 2 / screenPixels) * 2;
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

int PagedLOD2Tiles::createNode(std::string filename,osg::Node* node2, std::string outname,std::string outdir, Json::Value& newJsonNode, Json::Value& parentJsonNode,float geometricError, float& geometricErrorOfFirstLevel)
{
	osg::ref_ptr<osg::Node> node;
	if(node2)
	{
		node = node2;
	}
	else
	{
	    node = osgDB::readNodeFile(filename);
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
	/*	for (size_t i = 0; i < group->getNumChildren(); i++)
		{
			if(dynamic_cast<osg::PagedLOD*>(group->getChild(i)))
				childLODS.push_back((osg::PagedLOD*)group->getChild(i));
		}*/
		findPagedLOD(node, childLODS);
	}

	osg::BoundingBox bb;
	osg::ComputeBoundsVisitor cv;
	node->accept(cv);
	bb = cv.getBoundingBox();
	//if (m_flipAxis)
	//	bb = flip(bb);
	float range = 0;
	float radius = 0;
	//float geometricError = 0;
	if (childLODS.size() > 0 && childLODS[0]->getRangeList().size() > 1)
	{
		range = childLODS[0]->getRangeList()[0].second;
		radius = childLODS[0]->getRadius();
		geometricError = calGeometricError(radius, range);
		printf("%f,%f,%f\n",radius,range,geometricError);
		if (geometricErrorOfFirstLevel <= 0)
		{
			geometricErrorOfFirstLevel = geometricError;
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
		osg2gltf.m_flipAxis = m_flipAxis;
		//检测文件是否存在。如存在就不覆盖
		/*std::ifstream file(outdir + outname + ".b3dm");
		if (!file) {
			file.close();
			osg2gltf.toGLTF(node, outdir, outname, OSG2GLTF::B3DM);
		}
		else
		{
		  file.close();
		}*/
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
	
		if (createNode(childFileName, NULL, childname, outdir, childJSNode, newJsonNode,geometricError*0.5,geometricErrorOfFirstLevel) != 2)
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
	Json::Value tilset;
	Json::Value asset;
	asset["version"] = "0.0";
	asset["tilesetVersion"] = "1.2.3";
	Json::Value properties;
	tilset["asset"] = asset;
	tilset["properties"] = properties;
	Json::Value root;
	//Json::Value transform(Json::arrayValue);
	//transform.append(0.9686356343768792); transform.append(0.24848542777253735); transform.append(0.0); transform.append(0.0);
	//transform.append(-0.15986460744966327); transform.append(0.623177611820219); transform.append(0.765567091384559); transform.append(0.0);
	//transform.append(0.19023226619126932); transform.append(-0.7415555652213445); transform.append(0.6433560667227647); transform.append(0.0);
	//transform.append(1215011.9317263428); transform.append(-4736309.3434217675); transform.append(4081602.0044800863); transform.append(1.0);
	root["transform"] = createTransformNode(m_localTransform * m_local2world);
	float geometricError = calGeometricError(node->getBound().radius(), 0.5);
	float geometricErrorOfFirstLevel = -1;
	createNode("", node, "root", outdir, root, root, geometricError, geometricErrorOfFirstLevel);

	tilset["root"] = root;
	Json::StyledWriter sw;
	std::string tilesetContent = sw.write(tilset);
	std::ofstream ofs(outdir + "tileset.json");
	ofs << tilesetContent;
	ofs.close();

}
void PagedLOD2Tiles::toTileSet(std::string pagelodfile,  std::string outdir)
{

	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(pagelodfile);
	//std::string name;
	//std::string ext;
	//std::string dir;
	//splitFilepath(pagelodfile, dir, name, ext);
	toTileSet(node, outdir);
	//Json::Value tilset;
	//Json::Value asset;
	//asset["version"] = "0.0";
	//asset["tilesetVersion"] = "1.2.3";
	//Json::Value properties;
	//tilset["asset"] = asset;
	//tilset["properties"] = properties;
	//Json::Value root;
	////Json::Value transform(Json::arrayValue);
	////transform.append(0.9686356343768792); transform.append(0.24848542777253735); transform.append(0.0); transform.append(0.0);
	////transform.append(-0.15986460744966327); transform.append(0.623177611820219); transform.append(0.765567091384559); transform.append(0.0);
	////transform.append(0.19023226619126932); transform.append(-0.7415555652213445); transform.append(0.6433560667227647); transform.append(0.0);
	////transform.append(1215011.9317263428); transform.append(-4736309.3434217675); transform.append(4081602.0044800863); transform.append(1.0);
	//root["transform"] = createTransformNode(m_localTransform * m_local2world);
	//std::string name;
	//std::string ext;
	//std::string dir;
	//splitFilepath(pagelodfile, dir, name, ext);
	//createNode(pagelodfile, NULL, name, outdir, root, root,8);
	//tilset["root"] = root;
	//Json::StyledWriter sw;
	//std::string tilesetContent = sw.write(tilset);
	//std::ofstream ofs(outdir + "tileset.json");
	//ofs << tilesetContent;
	//ofs.close();

}

void PagedLOD2Tiles::toTileSet(osg::Group* group, std::string outdir)
{
	Json::Value tilset;
	Json::Value asset;
	asset["version"] = "0.0";
	asset["tilesetVersion"] = "1.2.3";
	Json::Value properties;
	tilset["asset"] = asset;
	tilset["properties"] = properties;
	Json::Value root;
	//Json::Value transform(Json::arrayValue);
	//transform.append(0.9686356343768792); transform.append(0.24848542777253735); transform.append(0.0); transform.append(0.0);
	//transform.append(-0.15986460744966327); transform.append(0.623177611820219); transform.append(0.765567091384559); transform.append(0.0);
	//transform.append(0.19023226619126932); transform.append(-0.7415555652213445); transform.append(0.6433560667227647); transform.append(0.0);
	//transform.append(1215011.9317263428); transform.append(-4736309.3434217675); transform.append(4081602.0044800863); transform.append(1.0);
	root["transform"] = createTransformNode(m_localTransform * m_local2world);

	osg::BoundingBox bb;
	osg::ComputeBoundsVisitor cv;
	group->accept(cv);
	bb = cv.getBoundingBox();
	float geometricError = calGeometricError(group->getBound().radius(),0.5);
	float geometricErrorOfFirstLevel = -1;

	Json::Value children(Json::arrayValue);
	for (size_t i = 0; i < group->getNumChildren(); i++)
	{
		std::string childname;
		std::string ext;
		std::string dir;
		Utils::splitFilepath(group->getChild(i)->getName(), dir, childname, ext);
		Json::Value childnode;
		createNode("", group->getChild(i), childname, outdir, childnode, childnode, geometricError * 0.5, geometricErrorOfFirstLevel);
		childnode["refine"] = "replace";
		children.append(childnode);
	}
	root["geometricError"] = geometricErrorOfFirstLevel * 2;
	Json::Value boundingVolume;
	boundingVolume["region"] = createRegionNode(bb);
	root["boundingVolume"] = boundingVolume;
	root["refine"] = "replace";

	root["children"] = children;
	tilset["root"] = root;
	Json::StyledWriter sw;
	std::string tilesetContent = sw.write(tilset);
	std::ofstream ofs(outdir + "tileset.json");
	ofs << tilesetContent;
	ofs.close();

}
