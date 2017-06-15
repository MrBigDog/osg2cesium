// OSG2Cesium.cpp : Defines the entry point for the console application.
//
 
#include "stdafx.h"
#include "OSG2GLTF.h"
#include "PAGEDLOD2TILESET.h"
//命令行参数解析
struct TileParams
{
	std::string indir;
	std::string outdir;
	double lat;
	double lon;
	double height;
	double scaleX;
	double scaleY;
	double scaleZ;
	double translateX;
	double translateY;
	double translateZ;
	osg::Matrixd transform()
	{
		return osg::Matrixd::scale(scaleX,scaleY,scaleZ) * osg::Matrixd::translate(translateX, translateY, translateZ);
	}
	TileParams(int argc, char* argv[])
	{
		int n_arg = 1;
		indir = argv[n_arg++];
		outdir = argv[n_arg++];
		lat = atof(argv[n_arg++]);
		lon = atof(argv[n_arg++]);
		height = atof(argv[n_arg++]);
		scaleX = scaleY = scaleZ = 1;
		translateX = translateY = translateZ = 0;
		if (argc >= 9)
		{
			scaleX = atof(argv[n_arg++]);
			scaleY = atof(argv[n_arg++]);
			scaleZ = atof(argv[n_arg++]);
		}

		if (argc >= 12)
		{
			translateX = atof(argv[n_arg++]);
			translateY = atof(argv[n_arg++]);
			translateZ = atof(argv[n_arg++]);
		}

	}
};
int main(int argc, char* argv[])
{
	if (argc < 6)
		return -1;
	TileParams params(argc, argv);
	PagedLOD2Tiles pagedLOD2Tiles;
	pagedLOD2Tiles.setTransform(params.lat, params.lon, params.height,params.transform());
	pagedLOD2Tiles.toTileSet(params.indir, params.outdir);
	return 0;
}

