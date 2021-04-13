#pragma once

#include "Containers/ContainerCommon.hpp"
#include "String/StringBuilder.hpp"
#include "Utility/Typedefs.hpp"

typedef Zero::String String;
typedef const String& StringParam;
typedef Zero::StringBuilder StringBuilder;

enum Types
{
  Point,Ray,Segment,Aabb,Capsule,Cylinder,Ellipsoid,Frustum,Obb,Plane,Sphere,Triangle,Tetrahedra,ConvexMesh,SweptTriangle, Size
};

enum IntersectionType
{
  PointType,CastType,ShapeType,SizeType
};

///Stores the type, variable name, function name and parameter offsets for this object.
struct Parameters
{
  ///Add a new parameter offset for this type.
  void Add(cstr param) { params.push_back(param); }

  ///Add all of the parameters for this variable into the builder.
  void FillOutParamters(StringBuilder& builder, StringParam varName)
  {
    //special case a point...
    if(typeName == "Vec3Param")
    {
      builder.Append(varName);
      return;
    }

    //otherwise, add all of the parameters with , in between
    for(uint i = 0; i < params.size() - 1; ++i)
    {
      builder.Append(varName);
      builder.Append(".");
      builder.Append(params[i]);
      builder.Append(",");
    }
    //add the last parameter, but without a comma after it
    builder.Append(varName);
    builder.Append(".");
    builder.Append(params[params.size() - 1]);
  }

  Zero::Array<String> params;
  String mVarName;
  String typeName;
  String functionName;
  bool mHasMpr;
  uint mCastObject;
};

///Stores the Parameters for all of the different object types.
struct ObjectParameters
{
  ObjectParameters()
  {
    objParams.resize(Size);
  }

  Parameters& operator[](uint i) { return objParams[i]; }

  Zero::Array<Parameters> objParams;
};

void LoadObjectParameters(ObjectParameters& params)
{
  params[Point].typeName = "Vec3Param";
  params[Point].functionName = "Point";
  params[Point].mVarName = "point";
  params[Point].Add("point");
  params[Point].mCastObject = PointType;
  params[Point].mHasMpr = false;

  params[Ray].typeName = "const Ray&";
  params[Ray].functionName = "Ray";
  params[Ray].mVarName = "ray";
  params[Ray].Add("Start");
  params[Ray].Add("Direction");
  params[Ray].mCastObject = CastType;
  params[Ray].mHasMpr = false;

  params[Segment].typeName = "const Segment&";
  params[Segment].functionName = "Segment";
  params[Segment].mVarName = "segment";
  params[Segment].Add("Start");
  params[Segment].Add("End");
  params[Segment].mCastObject = CastType;
  params[Segment].mHasMpr = false;

  params[Aabb].typeName = "const Aabb&";
  params[Aabb].functionName = "Aabb";
  params[Aabb].mVarName = "aabb";
  params[Aabb].Add("mMin");
  params[Aabb].Add("mMax");
  params[Aabb].mCastObject = ShapeType;
  params[Aabb].mHasMpr = true;

  params[Capsule].typeName = "const Capsule&";
  params[Capsule].functionName = "Capsule";
  params[Capsule].mVarName = "capsule";
  params[Capsule].Add("PointA");
  params[Capsule].Add("PointB");
  params[Capsule].Add("Radius");
  params[Capsule].mCastObject = ShapeType;
  params[Capsule].mHasMpr = true;

  params[Cylinder].typeName = "const Cylinder&";
  params[Cylinder].functionName = "Cylinder";
  params[Cylinder].mVarName = "cylinder";
  params[Cylinder].Add("PointA");
  params[Cylinder].Add("PointB");
  params[Cylinder].Add("Radius");
  params[Cylinder].mCastObject = ShapeType;
  params[Cylinder].mHasMpr = true;

  params[Ellipsoid].typeName = "const Ellipsoid&";
  params[Ellipsoid].functionName = "Ellipsoid";
  params[Ellipsoid].mVarName = "ellipsoid";
  params[Ellipsoid].Add("Center");
  params[Ellipsoid].Add("Radii");
  params[Ellipsoid].Add("Basis");
  params[Ellipsoid].mCastObject = ShapeType;
  params[Ellipsoid].mHasMpr = true;

  params[Frustum].typeName = "const Frustum&";
  params[Frustum].functionName = "Frustum";
  params[Frustum].mVarName = "frustum";
  params[Frustum].Add("GetIntersectionData()");
  params[Frustum].mCastObject = ShapeType;
  params[Frustum].mHasMpr = true;

  params[Obb].typeName = "const Obb&";
  params[Obb].functionName = "Obb";
  params[Obb].mVarName = "obb";
  params[Obb].Add("Center");
  params[Obb].Add("HalfExtents");
  params[Obb].Add("Basis");
  params[Obb].mCastObject = ShapeType;
  params[Obb].mHasMpr = true;

  params[Plane].typeName = "const Plane&";
  params[Plane].functionName = "Plane";
  params[Plane].mVarName = "plane";
  params[Plane].Add("GetNormal()");
  params[Plane].Add("GetDistance()");
  params[Plane].mCastObject = ShapeType;
  params[Plane].mHasMpr = false;

  params[Sphere].typeName = "const Sphere&";
  params[Sphere].functionName = "Sphere";
  params[Sphere].mVarName = "sphere";
  params[Sphere].Add("mCenter");
  params[Sphere].Add("mRadius");
  params[Sphere].mCastObject = ShapeType;
  params[Sphere].mHasMpr = true;

  params[Triangle].typeName = "const Triangle&";
  params[Triangle].functionName = "Triangle";
  params[Triangle].mVarName = "triangle";
  params[Triangle].Add("p0");
  params[Triangle].Add("p1");
  params[Triangle].Add("p2");
  params[Triangle].mCastObject = ShapeType;
  params[Triangle].mHasMpr = true;

  params[Tetrahedra].typeName = "const Tetrahedron&";
  params[Tetrahedra].functionName = "Tetrahedron";
  params[Tetrahedra].mVarName = "tetrahedron";
  params[Tetrahedra].Add("p0");
  params[Tetrahedra].Add("p1");
  params[Tetrahedra].Add("p2");
  params[Tetrahedra].Add("p3");
  params[Tetrahedra].mCastObject = ShapeType;
  params[Tetrahedra].mHasMpr = true;

  params[ConvexMesh].typeName = "const ConvexMeshShape&";
  params[ConvexMesh].functionName = "ConvexShape";
  params[ConvexMesh].mVarName = "supportShape";
  params[ConvexMesh].Add("mSupport");
  params[ConvexMesh].mCastObject = ShapeType;
  params[ConvexMesh].mHasMpr = true;

  params[SweptTriangle].typeName = "const SweptTriangle&";
  params[SweptTriangle].functionName = "SweptTriangle";
  params[SweptTriangle].mVarName = "sweptTri";
  params[SweptTriangle].Add("mSupport");
  params[SweptTriangle].mCastObject = ShapeType;
  params[SweptTriangle].mHasMpr = true;
}
