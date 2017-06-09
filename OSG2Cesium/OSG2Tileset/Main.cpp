// OSG2Cesium.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "OSG2GLTF.h"
#include "PAGEDLOD2TILESET.h"
std::vector<std::string> findSubdirs(std::string dir)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	std::string sPath;
	std::vector<std::string> MyVect;//"C:\\Documents and Settings\\yugesh\\Desktop\\*"
	sPath.assign(dir);
	hFind = FindFirstFileA(sPath.data(), &FindFileData);
	do
	{
		if (FindFileData.dwFileAttributes == 16)
		{
			if (std::string(FindFileData.cFileName) != "." && std::string(FindFileData.cFileName) != "..")
			{
				MyVect.push_back(FindFileData.cFileName);
			}

		}
	} while (FindNextFileA(hFind, &FindFileData));
	FindClose(hFind);
	//for (int i = 0; i<MyVect.size(); i++)
	//	cout << MyVect.at(i).data() << endl;
	return MyVect;
}
osg::Node* loadModels(std::string file)
{
	return osgDB::readNodeFile(file);
}
osg::Node* loadModels(std::vector<std::string> files)
{
	if (files.size() == 1)
	{
		osg::Node* nd = osgDB::readNodeFile(files[0]);
		return nd;
	}
	
	osg::Group* scene = new osg::Group;
	//osg::ComputeBoundsVisitor cbv;
	//scene->accept(cbv);
	//scene->setDatabasePath("");
	//scene->setFileName(0, "");
	osg::BoundingBox bb;
	bb.init();
	for	(int i=0;i<files.size();i++)
	{
		osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(files[i]);
		if (!node || !node.valid())
			continue;
		node->setName(files[i]);
		scene->addChild(node.get());
		/*osg::ComputeBoundsVisitor cbv;
		node->accept(cbv);
		bb.expandBy(cbv.getBoundingBox());
		scene->setFileName(i, files[i]);*/
	}
	//scene->setInitialBound(bb);
	//scene->setCenter(bb.center());

	return scene;
}

void ConvertTileDir(std::string indir,std::string outdir, double lat, double lon,double height)
{
	CreateDirectoryA(outdir.data(),NULL);
	std::vector<std::string> files;
	std::vector<std::string> dirs = findSubdirs(indir + "*");
	for (size_t i = 0; i < dirs.size(); i++)
	{
		if (dirs[i] == "." || dirs[i] == "..")
			continue;
		files.push_back(indir + dirs[i] + "\\" + dirs[i] + ".osgb");
		//break;
	}
	osg::ref_ptr<osg::Group> scene = dynamic_cast<osg::Group*>(loadModels(files));

	PagedLOD2Tiles pagedLOD2Tiles;
	pagedLOD2Tiles.setTransform(lat,lon,height);
	pagedLOD2Tiles.toTileSet(scene, outdir);
}

struct TileParams
{
	std::string indir;
	std::string outdir;
	double lat;
	double lon;
	double height;
	TileParams(int argc, char* argv[])
	{
		indir = argv[1];
		outdir = argv[2];
		lat = atof(argv[3]);
		lon = atof(argv[4]);
		height = atof(argv[5]);
	}
};
int main(int argc, char* argv[])
{
	if (argc < 6)
		return -1;
	TileParams params(argc, argv);
	PagedLOD2Tiles pagedLOD2Tiles;
	ConvertTileDir(params.indir, params.outdir, params.lat, params.lon, params.height);
	return 0;
}

