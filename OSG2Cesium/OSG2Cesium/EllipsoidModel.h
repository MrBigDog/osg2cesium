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
 
#ifndef _EllipsoidModel
#define _EllipsoidModel 1
#include <Windows.h>
#include <osg/Group>
#include <osg/Matrixd>

namespace osg
{


//构建地理坐标到世界坐标转换矩阵的类
class CesiumEllipsoidModel
{
    public:
		double WGS_84_RADIUS_EQUATOR;
		double WGS_84_RADIUS_POLAR;
        /** WGS_84 is a common representation of the earth's spheroid */
		CesiumEllipsoidModel()
		{
			WGS_84_RADIUS_EQUATOR = 6378137.0;
			//WGS_84_RADIUS_POLAR = 6378137.0;
			WGS_84_RADIUS_POLAR = 6356752.3142;
			_radiusEquator = WGS_84_RADIUS_EQUATOR;
			_radiusPolar = WGS_84_RADIUS_POLAR;
			computeCoefficients();
		}

        //CesiumEllipsoidModel(const CesiumEllipsoidModel& et,const CopyOp& copyop=CopyOp::SHALLOW_COPY):
        //    Object(et,copyop),
        //    _radiusEquator(et._radiusEquator),
        //    _radiusPolar(et._radiusPolar) { computeCoefficients(); }

        //META_Object(osg,CesiumEllipsoidModel);

        void setRadiusEquator(double radius) { _radiusEquator = radius; computeCoefficients(); }
        double getRadiusEquator() const { return _radiusEquator; }

        void setRadiusPolar(double radius) { _radiusPolar = radius; computeCoefficients(); }
        double getRadiusPolar() const { return _radiusPolar; }

         void convertLatLongHeightToXYZ(double latitude, double longitude, double height,
                                              double& X, double& Y, double& Z) const;

         void convertXYZToLatLongHeight(double X, double Y, double Z,
                                              double& latitude, double& longitude, double& height) const;

         void computeLocalToWorldTransformFromLatLongHeight(double latitude, double longitude, double height, osg::Matrixd& localToWorld) const;

         void computeLocalToWorldTransformFromXYZ(double X, double Y, double Z, osg::Matrixd& localToWorld) const;

         void computeCoordinateFrame(double latitude, double longitude, osg::Matrixd& localToWorld) const;

         osg::Vec3d computeLocalUpVector(double X, double Y, double Z) const;

        // Convenience method for determining if CesiumEllipsoidModel is a stock WGS84 ellipsoid
         bool isWGS84() const {return(_radiusEquator == WGS_84_RADIUS_EQUATOR && _radiusPolar == WGS_84_RADIUS_POLAR);}

        // Compares two CesiumEllipsoidModel by comparing critical internal values.
        // Ignores _eccentricitySquared since it's just a cached value derived from
        // the _radiusEquator and _radiusPolar members.
        friend bool operator == ( const CesiumEllipsoidModel & e1, const CesiumEllipsoidModel& e2) {return(e1._radiusEquator == e2._radiusEquator && e1._radiusPolar == e2._radiusPolar);}


    protected:

        void computeCoefficients()
        {
            double flattening = (_radiusEquator-_radiusPolar)/_radiusEquator;
            _eccentricitySquared = 2*flattening - flattening*flattening;
        }

        double _radiusEquator;
        double _radiusPolar;
        double _eccentricitySquared;

};

/** CoordinateFrame encapsulates the orientation of east, north and up.*/
typedef Matrixd CoordinateFrame;



}
#endif
