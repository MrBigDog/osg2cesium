#include "EllipsoidModel.h"
using namespace osg;
//Ellipsoid.prototype.geodeticSurfaceNormalCartographic = function(cartographic, result) {
//	//>>includeStart('debug', pragmas.debug);
//	if (!defined(cartographic)) {
//		throw new DeveloperError('cartographic is required.');
//	}
//	//>>includeEnd('debug');
//
//	var longitude = cartographic.longitude;
//	var latitude = cartographic.latitude;
//	var cosLatitude = Math.cos(latitude);
//
//	var x = cosLatitude * Math.cos(longitude);
//	var y = cosLatitude * Math.sin(longitude);
//	var z = Math.sin(latitude);
//
//	if (!defined(result)) {
//		result = new Cartesian3();
//	}
//	result.x = x;
//	result.y = y;
//	result.z = z;
//	return Cartesian3.normalize(result, result);
//};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// implement  methods.
//
 void CesiumEllipsoidModel::convertLatLongHeightToXYZ(double latitude, double longitude, double height,
	double& X, double& Y, double& Z) const
{
	// for details on maths see http://www.colorado.edu/geography/gcraft/notes/datum/gif/llhxyz.gif
	double sin_latitude = sin(latitude);
	double cos_latitude = cos(latitude);
	double N = _radiusEquator / sqrt(1.0 - _eccentricitySquared*sin_latitude*sin_latitude);
	X = (N + height)*cos_latitude*cos(longitude);
	Y = (N + height)*cos_latitude*sin(longitude);
	Z = (N*(1 - _eccentricitySquared) + height)*sin_latitude;
}


 void CesiumEllipsoidModel::convertXYZToLatLongHeight(double X, double Y, double Z,
	double& latitude, double& longitude, double& height) const
{
	// http://www.colorado.edu/geography/gcraft/notes/datum/gif/xyzllh.gif
	double p = sqrt(X*X + Y*Y);
	double theta = atan2(Z*_radiusEquator, (p*_radiusPolar));
	double eDashSquared = (_radiusEquator*_radiusEquator - _radiusPolar*_radiusPolar) /
		(_radiusPolar*_radiusPolar);

	double sin_theta = sin(theta);
	double cos_theta = cos(theta);

	latitude = atan((Z + eDashSquared*_radiusPolar*sin_theta*sin_theta*sin_theta) /
		(p - _eccentricitySquared*_radiusEquator*cos_theta*cos_theta*cos_theta));
	longitude = atan2(Y, X);

	double sin_latitude = sin(latitude);
	double N = _radiusEquator / sqrt(1.0 - _eccentricitySquared*sin_latitude*sin_latitude);

	height = p / cos(latitude) - N;
}

 void CesiumEllipsoidModel::computeLocalToWorldTransformFromLatLongHeight(double latitude, double longitude, double height, osg::Matrixd& localToWorld) const
{
	double X, Y, Z;
	convertLatLongHeightToXYZ(latitude, longitude, height, X, Y, Z);

	localToWorld.makeTranslate(X, Y, Z);
	computeCoordinateFrame(latitude, longitude, localToWorld);
}

 void CesiumEllipsoidModel::computeLocalToWorldTransformFromXYZ(double X, double Y, double Z, osg::Matrixd& localToWorld) const
{
	double  latitude, longitude, height;
	convertXYZToLatLongHeight(X, Y, Z, latitude, longitude, height);

	localToWorld.makeTranslate(X, Y, Z);
	computeCoordinateFrame(latitude, longitude, localToWorld);
}

 void CesiumEllipsoidModel::computeCoordinateFrame(double latitude, double longitude, osg::Matrixd& localToWorld) const
{
	// Compute up vector
	osg::Vec3d    up(cos(longitude)*cos(latitude), sin(longitude)*cos(latitude), sin(latitude));

	// Compute east vector
	osg::Vec3d    east(-sin(longitude), cos(longitude), 0);

	// Compute north vector = outer product up x east
	osg::Vec3d    north = up ^ east;

	// set matrix
	localToWorld(0, 0) = east[0];
	localToWorld(0, 1) = east[1];
	localToWorld(0, 2) = east[2];

	localToWorld(1, 0) = north[0];
	localToWorld(1, 1) = north[1];
	localToWorld(1, 2) = north[2];

	localToWorld(2, 0) = up[0];
	localToWorld(2, 1) = up[1];
	localToWorld(2, 2) = up[2];
}

 osg::Vec3d CesiumEllipsoidModel::computeLocalUpVector(double X, double Y, double Z) const
{
	// Note latitude is angle between normal to ellipsoid surface and XY-plane
	double  latitude;
	double  longitude;
	double  altitude;
	convertXYZToLatLongHeight(X, Y, Z, latitude, longitude, altitude);

	// Compute up vector
	return osg::Vec3d(cos(longitude) * cos(latitude),
		sin(longitude) * cos(latitude),
		sin(latitude));
}