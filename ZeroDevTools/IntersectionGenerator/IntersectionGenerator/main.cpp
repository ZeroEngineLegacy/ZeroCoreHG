#include "ObjectParameters.hpp"
#include "ObjectIgnores.hpp"

#include <fstream>

void AddOverlapHeader(StringBuilder& builder, StringParam fileName, bool hpp)
{
  builder.Append("///////////////////////////////////////////////////////////////////////////////\n");
  builder.Append("///\n");
  builder.Append(String::Format("/// \\file %s\n",fileName.c_str()));
  builder.Append("/// NSquared intersection functions for the shape primitives. Wraps the\n");
  builder.Append("/// internal intersection functions for ease of use.\n");
  builder.Append("///\n");
  builder.Append("/// Authors: Joshua Davis, Auto-Generated\n");
  builder.Append("/// Copyright 2010-2012, DigiPen Institute of Technology\n");
  builder.Append("///\n");
  builder.Append("///////////////////////////////////////////////////////////////////////////////\n");
  if(hpp)
    builder.Append("#pragma once\n");
  builder.Append("\n");
}

void AddCollisionHeader(StringBuilder& builder, StringParam fileName, bool hpp)
{
  builder.Append("///////////////////////////////////////////////////////////////////////////////\n");
  builder.Append("///\n");
  builder.Append(String::Format("/// \\file %s\n",fileName.c_str()));
  builder.Append("/// NSquared collision functions for the shape primitives. Wraps the\n");
  builder.Append("/// internal collision functions for ease of use.\n");
  builder.Append("///\n");
  builder.Append("/// Authors: Joshua Davis, Auto-Generated\n");
  builder.Append("/// Copyright 2010-2012, DigiPen Institute of Technology\n");
  builder.Append("///\n");
  builder.Append("///////////////////////////////////////////////////////////////////////////////\n");
  if(hpp)
    builder.Append("#pragma once\n");
  builder.Append("\n");
}

///Stores the info to create the signature of a function
struct FunctionInfo
{
  FunctionInfo(ObjectParameters& objParam, uint i, uint j)
  {
    mOverlapName = "Overlap";
    mCollideName = "Collide";
    mParam1Index = i;
    mParam2Index = j;
    mFlipped = false;

    //determine the correct function name ordering 
    //(Alphabetical, but Ray,Segment and Point are always before the other types...)
    if(i <= Segment && j > Segment)
    {
      mIndex1 = i;
      mIndex2 = j;
    }
    else if(j <= Segment && i > Segment)
    {
      mIndex1 = j;
      mIndex2 = i;
      mFlipped = true;
    }
    else if(objParam[i].functionName < objParam[j].functionName || objParam[i].functionName == objParam[j].functionName)
    {
      mIndex1 = i;
      mIndex2 = j;
    }
    else
    {
      mIndex1 = j;
      mIndex2 = i;
      mFlipped = true;
    }

    //get the variable name and deal with the case of the same type (so append 1,2 onto the var names)
    mVar1Name = objParam[mIndex1].mVarName;
    mVar2Name = objParam[mIndex2].mVarName;
    if(i == j)
    {
      mVar1Name = Zero::BuildString(mVar1Name,"1");
      mVar2Name = Zero::BuildString(mVar2Name,"2");
    }

    //then build the correctly sorted function name
    mFunctionName = BuildString(objParam[mIndex1].functionName,objParam[mIndex2].functionName);
    //also build the flipped string
    mFlippedFunctionName = BuildString(objParam[mIndex2].functionName,objParam[mIndex1].functionName);
  }

  ///Builds the signature for the function
  void BuildSignature(StringBuilder& results, ObjectParameters& objParam, uint i, uint j, bool isHeader)
  {
    results.Append("bool ");
    results.Append(mOverlapName);
    results.Append("(");
    results.Append(objParam[i].typeName);
    results.Append(" ");
    if(i == mIndex1)
      results.Append(mVar1Name);
    else if(i == mIndex2)
      results.Append(mVar2Name);
    results.Append(", ");
    results.Append(objParam[j].typeName);
    results.Append(" ");
    if(j == mIndex2)
      results.Append(mVar2Name);
    else if(j == mIndex1)
      results.Append(mVar1Name);
    results.Append(")");
    if(isHeader)
      results.Append(";");
    results.Append("\n");
  }

  ///Builds the signature for the function
  void BuildCollisionSignature(StringBuilder& results, ObjectParameters& objParam, uint i, uint j, bool isHeader)
  {
    results.Append("bool ");
    results.Append(mCollideName);
    results.Append("(");
    results.Append(objParam[i].typeName);
    results.Append(" ");
    if(i == mIndex1)
      results.Append(mVar1Name);
    else if(i == mIndex2)
      results.Append(mVar2Name);
    results.Append(", ");
    results.Append(objParam[j].typeName);
    results.Append(" ");
    if(j == mIndex2)
      results.Append(mVar2Name);
    else if(j == mIndex1)
      results.Append(mVar1Name);
    results.Append(", Intersection::Manifold* manifold");
    results.Append(")");
    if(isHeader)
      results.Append(";");
    results.Append("\n");
  }

  bool mFlipped;
  uint mIndex1,mIndex2;
  uint mParam1Index,mParam2Index;
  String mVar1Name,mVar2Name;
  String mFunctionName;
  String mFlippedFunctionName;
  String mOverlapName;
  String mCollideName;
};

typedef void (*InternalCall)(ObjectParameters& objParam, FunctionInfo& info, StringBuilder& results);

void GenerateSupportShapeOverlapHeader(StringBuilder& results)
{
  results.Append("bool SupportShapeOverlap(const Intersection::SupportShape& a, const Intersection::SupportShape& b);\n\n");
}

void GenerateSupportShapeOverlapSource(StringBuilder& results)
{
  results.Append("bool SupportShapeOverlap(const Intersection::SupportShape& a, const Intersection::SupportShape& b)\n");
  results.Append("{\n");
  results.Append("  //Test for collision.\n");
  results.Append("  Intersection::Mpr mpr;\n");
  results.Append("  Intersection::Type ret = mpr.Test(&a, &b, NULL);\n");
  results.Append("  if(ret < (Intersection::Type)0)\n");
  results.Append("    return false;\n");
  results.Append("  return true;\n}\n\n");
}

void GenerateSupportShapeCollideHeader(StringBuilder& results)
{
  results.Append("bool SupportShapeCollide(const Intersection::SupportShape& a,\n"
                 "                         const Intersection::SupportShape& b,\n"
                 "                         Intersection::Manifold* manifold);\n\n");
}

void GenerateSupportShapeCollideSource(StringBuilder& results)
{
  results.Append("bool SupportShapeCollide(const Intersection::SupportShape& a,\n"
                 "                         const Intersection::SupportShape& b,\n"
                 "                         Intersection::Manifold* manifold)\n");
  results.Append("{\n");
  results.Append("  //Test for collision.\n");
  results.Append("  Intersection::Gjk gjk;\n");
  results.Append("  Intersection::Type ret = gjk.Test(&a, &b, manifold);\n");
  results.Append("  if(ret < (Intersection::Type)0)\n");
  results.Append("    return false;\n\n");
  results.Append("  if(manifold != NULL)\n");
  results.Append("    FlipSupportShapeManifoldInfo(a,b,manifold);\n");
  results.Append("  return true;\n}\n\n");
}

void GenerateOverlapCallNormal(ObjectParameters& objParam, FunctionInfo& info, StringBuilder& results)
{
  results.Append("{\n  ");
  results.Append("return Intersection::");
  results.Append(info.mFunctionName);
  results.Append("(");

  objParam[info.mIndex1].FillOutParamters(results,info.mVar1Name);
  results.Append(",");
  objParam[info.mIndex2].FillOutParamters(results,info.mVar2Name);

  results.Append(") >= (Intersection::Type)0;\n}\n\n");
}

void ResolveMeshSupport(uint shapeIndex, StringParam supportName, StringParam varName, StringBuilder& results)
{
  if(shapeIndex == ConvexMesh)
  {
    String rightSide = BuildString(supportName," = ",varName,".mSupport;\n");
    results.Append(BuildString("  const Intersection::SupportShape& ",rightSide));
  }
  else
  {
    String rightSide = BuildString(supportName," = Intersection::MakeSupport(&",varName,");\n");
    results.Append(BuildString("  Intersection::SupportShape ",rightSide));
  }
}

void GenerateOverlapCallMpr(ObjectParameters& objParam, FunctionInfo& info, StringBuilder& results)
{
  results.Append("{\n");
  results.Append("  //Test for collision.\n");
  if(info.mParam1Index == info.mIndex1)
  {
    ResolveMeshSupport(info.mIndex1,"a",info.mVar1Name,results);
    ResolveMeshSupport(info.mIndex2,"b",info.mVar2Name,results);
  }
  else
  {
    ResolveMeshSupport(info.mIndex2,"a",info.mVar2Name,results);
    ResolveMeshSupport(info.mIndex1,"b",info.mVar1Name,results);
  }
  results.Append("  return SupportShapeOverlap(a,b);\n");
  results.Append("}\n\n");
}

void GenerateOverlapMprPoint(ObjectParameters& objParam, FunctionInfo& info, StringBuilder& results)
{
  results.Append("{\n");
  ResolveMeshSupport(info.mIndex2,"supportShape",info.mVar2Name,results);
  String params;
  params = String::Format("%s, supportShape", info.mVar1Name.c_str());
  
  results.Append(BuildString("  return Intersection::PointConvexShape(", params, ") >= (Intersection::Type)0;\n"));
  results.Append("}\n\n");
}

void GenerateCollideCallNormal(ObjectParameters& objParam, FunctionInfo& info, StringBuilder& results)
{
  results.Append("{\n  ");
  results.Append("Intersection::Type ret = ");
  results.Append("Intersection::");
  results.Append(info.mFunctionName);
  results.Append("(");

  objParam[info.mIndex1].FillOutParamters(results,info.mVar1Name);
  results.Append(",");
  objParam[info.mIndex2].FillOutParamters(results,info.mVar2Name);

  results.Append(", manifold);\n");

  results.Append("  if(ret < (Intersection::Type)0)\n");
  results.Append("    return false;\n");
  results.Append("  return true;\n}\n\n");
}

void GenerateCollideCallFlipped(ObjectParameters& objParam, FunctionInfo& info, StringBuilder& results)
{
  results.Append("{\n");
  results.Append("  bool ret = ");
  results.Append(info.mCollideName);
  results.Append("(");
  results.Append(info.mVar1Name);
  results.Append(",");
  results.Append(info.mVar2Name);
  results.Append(", manifold);\n");

  results.Append("  if(ret == false)\n");
  results.Append("    return false;\n");

  results.Append("  if(manifold != NULL)\n");
  results.Append("    FlipManifoldInfo(manifold);\n");
  results.Append("  return true;\n}\n\n");
}

void GenerateCollideCallCast(ObjectParameters& objParam, FunctionInfo& info, StringBuilder& results)
{
  results.Append("{\n");
  results.Append("  Intersection::IntersectionPoint point;\n");
  results.Append("  Intersection::Type ret = ");
  results.Append("Intersection::");
  results.Append(info.mFunctionName);
  results.Append("(");

  objParam[info.mIndex1].FillOutParamters(results,info.mVar1Name);
  results.Append(",");
  objParam[info.mIndex2].FillOutParamters(results,info.mVar2Name);

  results.Append(", &point);\n");

  results.Append("  if(ret < (Intersection::Type)0)\n");
  results.Append("    return false;\n");
  results.Append("  if(manifold != NULL)\n");
  results.Append("  {\n");
  results.Append("    manifold->Normal.Set(1,0,0);\n");
  results.Append("    manifold->Points[0] = point;\n");
  results.Append("    if(ret == Intersection::Point)\n");
  results.Append("      manifold->PointCount = 1;\n");
  results.Append("    else\n");
  results.Append("      manifold->PointCount = 2;\n");
  results.Append("  }\n");
  results.Append("  return true;\n}\n\n");
}

void GenerateCollideCallCastFlipped(ObjectParameters& objParam, FunctionInfo& info, StringBuilder& results)
{
  results.Append("{\n");
  results.Append("  Intersection::IntersectionPoint point;\n");
  results.Append("  Intersection::Type ret = ");
  results.Append("Intersection::");
  results.Append(info.mFunctionName);
  results.Append("(");

  objParam[info.mIndex1].FillOutParamters(results,info.mVar1Name);
  results.Append(",");
  objParam[info.mIndex2].FillOutParamters(results,info.mVar2Name);

  results.Append(", &point);\n");

  results.Append("  if(ret < Intersection::None)\n");
  results.Append("    return false;\n");
  results.Append("  if(manifold != NULL)\n");
  results.Append("  {\n");
  results.Append("    manifold->Normal.Set(1,0,0);\n");
  results.Append("    manifold->Points[0] = point;\n");
  results.Append("    if(ret == Intersection::Point)\n");
  results.Append("      manifold->PointCount = 1;\n");
  results.Append("    else\n");
  results.Append("      manifold->PointCount = 2;\n");
  results.Append("  }\n");
  results.Append("  return true;\n}\n\n");
}

void GenerateCollideCallMpr(ObjectParameters& objParam, FunctionInfo& info, StringBuilder& results)
{
  results.Append("{\n");
  results.Append("  //Test for collision.\n");
  if(info.mParam1Index == info.mIndex1)
  {
    ResolveMeshSupport(info.mIndex1,"a",info.mVar1Name,results);
    ResolveMeshSupport(info.mIndex2,"b",info.mVar2Name,results);
  }
  else
  {
    ResolveMeshSupport(info.mIndex2,"a",info.mVar2Name,results);
    ResolveMeshSupport(info.mIndex1,"b",info.mVar1Name,results);
  }
  results.Append("  return SupportShapeCollide(a,b,manifold);\n");
  results.Append("}\n\n");
}

void GenerateOverlapCallError(ObjectParameters& objParam, FunctionInfo& info, StringBuilder& results)
{
  results.Append("{\n");
  results.Append("  ErrorIf(true,\"Not Implemented\");\n");
  results.Append("  return false;\n");
  results.Append("}\n\n");
}

///Generates the header file.
void GenerateOverlapHeader(ObjectParameters& objParam,IgnoreList& ignores, StringBuilder& results)
{
  for(uint i = 0; i < Size; ++i)
  {
    for(uint j = 0; j < Size; ++j)
    {
      if(ignores.Ignores[i][j] == false)
      {
        if(!(i == Point && objParam[j].mHasMpr) &&
           !(j == Point && objParam[i].mHasMpr) && 
           !(objParam[i].mHasMpr && objParam[j].mHasMpr))
          continue;
      }

      FunctionInfo info(objParam,i,j);
      info.BuildSignature(results,objParam,i,j,true);
    }
    results.Append("\n");
  }
}

//Generates the source file.
void GenerateOverlapFunctions(ObjectParameters& objParam,IgnoreList& ignores, StringBuilder& results)
{
  for(uint i = 0; i < Size; ++i)
  {
    for(uint j = 0; j < Size; ++j)
    {
      InternalCall callback = NULL;

      if(ignores.Ignores[i][j] == false)
      {
        bool hasMpr = objParam[i].mHasMpr && objParam[j].mHasMpr;
        if((i == Point && objParam[j].mHasMpr) || 
           (j == Point && objParam[i].mHasMpr))
           callback = GenerateOverlapMprPoint;
        else if(!hasMpr)
          callback = GenerateOverlapCallError;
        else
          callback = GenerateOverlapCallMpr;
      }
      else
        callback = GenerateOverlapCallNormal;

      FunctionInfo info(objParam,i,j);
      info.BuildSignature(results,objParam,i,j,false);
      callback(objParam,info,results);
    }
  }
}

///Generates the header file.
void GenerateCollideHeader(ObjectParameters& objParam,IgnoreList& ignores, StringBuilder& results)
{
  for(uint i = 0; i < Size; ++i)
  {
    for(uint j = 0; j < Size; ++j)
    {
      //if(ignores.Ignores[i][j] == false)
      //  continue;

      if(objParam.objParams[i].mCastObject == PointType || 
        objParam.objParams[j].mCastObject == PointType)
        continue;

      if(objParam.objParams[i].mCastObject == CastType && 
        objParam.objParams[j].mCastObject == CastType)
        continue;

      FunctionInfo info(objParam,i,j);
      info.BuildCollisionSignature(results,objParam,i,j,true);
    }
    results.Append("\n");
  }
}

void FlipManifoldInfoHeader(StringBuilder& results)
{
  results.Append("void FlipManifoldInfo(Intersection::Manifold* manifold);\n");
}

void FlipManifoldInfoSource(StringBuilder& results)
{
  results.Append("void FlipManifoldInfo(Intersection::Manifold* manifold)\n");
  results.Append("{\n");
  results.Append("  manifold->Normal.Negate();\n");
  results.Append("  for(uint i = 0; i < manifold->PointCount; ++i)\n");
  results.Append("    Math::Swap(manifold->Points[i].Points[0],manifold->Points[i].Points[1]);\n");
  results.Append("}\n\n");
}

void FlipSupportShapeManifoldInfoHeader(StringBuilder& results)
{
  results.Append("void FlipSupportShapeManifoldInfo(const Intersection::SupportShape& a,\n"
                 "                                  const Intersection::SupportShape& b,\n"
                 "                                  Intersection::Manifold* manifold);\n");
}

void FlipSupportShapeManifoldInfoSource(StringBuilder& results)
{
  results.Append("void FlipSupportShapeManifoldInfo(const Intersection::SupportShape& a,\n"
                 "                                  const Intersection::SupportShape& b,\n"
                 "                                  Intersection::Manifold* manifold)\n");
  results.Append("{\n");
  results.Append("  Vec3 aCenter,bCenter;\n");
  results.Append("  a.GetCenter(&aCenter);\n");
  results.Append("  b.GetCenter(&bCenter);\n");
  results.Append("  Vec3 aToB = bCenter - aCenter;\n");
  results.Append("  //flip the normal to alway point from a to b\n");
  results.Append("  if(Math::Dot(aToB, manifold->Normal) < Math::real(0.0))\n");
  results.Append("    manifold->Normal *= real(-1.0);\n");
  results.Append("}\n\n");
}

//Generates the source file.
void GenerateCollideFunctions(ObjectParameters& objParam,IgnoreList& ignores, StringBuilder& results)
{
  for(uint i = 0; i < Size; ++i)
  {
    for(uint j = 0; j < Size; ++j)
    {
      InternalCall callback = NULL;

      if(objParam.objParams[i].mCastObject == PointType || 
         objParam.objParams[j].mCastObject == PointType)
        continue;

      if(objParam.objParams[i].mCastObject == CastType && 
        objParam.objParams[j].mCastObject == CastType)
        continue;

      bool castType = (objParam.objParams[i].mCastObject == CastType) || (objParam.objParams[j].mCastObject == CastType);

      FunctionInfo info(objParam,i,j);
      info.BuildCollisionSignature(results,objParam,i,j,false);

      if(ignores.Ignores[i][j] == false)
      {
        bool hasMpr = objParam[i].mHasMpr && objParam[j].mHasMpr;
        if(!hasMpr)
          callback = GenerateOverlapCallError;
        else
          callback = GenerateCollideCallMpr;
      }
      else if(!castType)
      {
        if(!info.mFlipped)
          callback = GenerateCollideCallNormal;
        else
          callback = GenerateCollideCallFlipped;
      }
      else
      {
        if(!info.mFlipped)
          callback = GenerateCollideCallCast;
        else
          callback = GenerateCollideCallCastFlipped;
      }

      callback(objParam,info,results);
    }
  }
}

void BuildOverlaps()
{
  IgnoreList ignoreList;
  LoadObjectIgnores(ignoreList);

  ObjectParameters objParams;
  LoadObjectParameters(objParams);

  StringBuilder headerBuilder;
  StringBuilder sourceBuilder;

  AddOverlapHeader(headerBuilder,"ExtendedIntersection.hpp",true);
  AddOverlapHeader(sourceBuilder,"ExtendedIntersection.cpp",false);
  headerBuilder.Append("#include \"Geometry/ShapeHelpers.hpp\"\n\nnamespace Zero\n{\n\n");
  sourceBuilder.Append("#include \"Precompiled.hpp\"\n\n#include \"Geometry/ExtendedIntersection.hpp\"\n\n#include \"Geometry/Shapes.hpp\"\n#include \"Geometry/Intersection.hpp\"\n#include \"Geometry/Mpr.hpp\"\n\nnamespace Zero\n{\n\n");

  GenerateSupportShapeOverlapHeader(headerBuilder);
  GenerateOverlapHeader(objParams,ignoreList,headerBuilder);

  GenerateSupportShapeOverlapSource(sourceBuilder);
  GenerateOverlapFunctions(objParams,ignoreList,sourceBuilder);

  headerBuilder.Append("\n}//namespace Zero\n");
  sourceBuilder.Append("}//namespace Zero\n");

  String headerStr = headerBuilder.ToString();
  String sourceStr = sourceBuilder.ToString();

  const byte* header = reinterpret_cast<const byte*>(headerStr.c_str());
  const byte* source = reinterpret_cast<const byte*>(sourceStr.c_str());

  std::ofstream stream;
  stream.open("ExtendedIntersection.hpp");
  stream.write(headerStr.c_str(),headerStr.size());
  stream.close();

  stream.open("ExtendedIntersection.cpp");
  stream.write(sourceStr.c_str(),sourceStr.size());
  stream.close();
}

void BuildCollides()
{
  IgnoreList ignoreList;
  LoadObjectIgnores(ignoreList);

  ObjectParameters objParams;
  LoadObjectParameters(objParams);

  StringBuilder headerBuilder;
  StringBuilder sourceBuilder;

  AddOverlapHeader(headerBuilder,"ExtendedCollision.hpp",true);
  AddOverlapHeader(sourceBuilder,"ExtendedCollision.cpp",false);
  headerBuilder.Append("#include \"Geometry/ShapeHelpers.hpp\"\n\n#include \"Geometry/Intersection.hpp\"\n\nnamespace Zero\n{\n\n");
  sourceBuilder.Append("#include \"Precompiled.hpp\"\n\n#include \"Geometry/ExtendedCollision.hpp\"\n\n#include \"Geometry/Shapes.hpp\"\n#include \"Geometry/Gjk.hpp\"\n\nnamespace Zero\n{\n\n");

  FlipManifoldInfoHeader(headerBuilder);
  FlipSupportShapeManifoldInfoHeader(headerBuilder);
  GenerateSupportShapeCollideHeader(headerBuilder);
  GenerateCollideHeader(objParams,ignoreList,headerBuilder);

  FlipManifoldInfoSource(sourceBuilder);
  FlipSupportShapeManifoldInfoSource(sourceBuilder);
  GenerateSupportShapeCollideSource(sourceBuilder);
  GenerateCollideFunctions(objParams,ignoreList,sourceBuilder);

  headerBuilder.Append("\n}//namespace Zero\n");
  sourceBuilder.Append("}//namespace Zero\n");

  String headerStr = headerBuilder.ToString();
  String sourceStr = sourceBuilder.ToString();

  const byte* header = reinterpret_cast<const byte*>(headerStr.c_str());
  const byte* source = reinterpret_cast<const byte*>(sourceStr.c_str());

  std::ofstream stream;
  stream.open("ExtendedCollision.hpp");
  stream.write(headerStr.c_str(),headerStr.size());
  stream.close();

  stream.open("ExtendedCollision.cpp");
  stream.write(sourceStr.c_str(),sourceStr.size());
  stream.close();
}

int main()
{
  BuildOverlaps();
  BuildCollides();

  return 0;
}
