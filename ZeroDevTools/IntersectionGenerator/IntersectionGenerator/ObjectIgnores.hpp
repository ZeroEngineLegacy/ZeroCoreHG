///A 2D block array that signifies whether or not to ignore a pairing.
struct IgnoreList
{
  IgnoreList()
  {
    for(uint i = 0; i < Size; ++i)
      for(uint j = 0; j < Size; ++j)
        Ignores[i][j] = true;
  }

  void Add(uint objA, uint objB)
  {
    Ignores[objA][objB] = false;
    Ignores[objB][objA] = false;
  }

  void AddAll(uint id)
  {
    for(uint i = 0; i < Size; ++i)
    {
      Ignores[i][id] = false;
      Ignores[id][i] = false;
    }
  }

  bool Ignores[Size][Size];
};

void LoadObjectIgnores(IgnoreList& ignoreList)
{
  ignoreList.Add(Ray,Ray);
  ignoreList.Add(Ray,Segment);
  ignoreList.Add(Ray,Frustum);
  ignoreList.Add(Segment,Ray);
  ignoreList.Add(Segment,Segment);
  ignoreList.Add(Segment,Ellipsoid);
  ignoreList.Add(Segment,Frustum);
  ignoreList.Add(Aabb,Capsule);
  ignoreList.Add(Aabb,Cylinder);
  ignoreList.Add(Aabb,Ellipsoid);
  ignoreList.Add(Aabb,Frustum);
  ignoreList.Add(Capsule,Aabb);
  ignoreList.Add(Capsule,Cylinder);
  ignoreList.Add(Capsule,Ellipsoid);
  ignoreList.Add(Capsule,Frustum);
  ignoreList.Add(Capsule,Obb);
  ignoreList.Add(Capsule,Plane);
  ignoreList.Add(Capsule,Triangle);
  ignoreList.Add(Cylinder,Aabb);
  ignoreList.Add(Cylinder,Capsule);
  ignoreList.Add(Cylinder,Cylinder);
  ignoreList.Add(Cylinder,Ellipsoid);
  ignoreList.Add(Cylinder,Frustum);
  ignoreList.Add(Cylinder,Obb);
  ignoreList.Add(Cylinder,Plane);
  ignoreList.Add(Cylinder,Sphere);
  ignoreList.Add(Cylinder,Triangle);
  ignoreList.Add(Ellipsoid,Segment);
  ignoreList.Add(Ellipsoid,Aabb);
  ignoreList.Add(Ellipsoid,Capsule);
  ignoreList.Add(Ellipsoid,Cylinder);
  ignoreList.Add(Ellipsoid,Ellipsoid);
  ignoreList.Add(Ellipsoid,Frustum);
  ignoreList.Add(Ellipsoid,Obb);
  ignoreList.Add(Ellipsoid,Plane);
  ignoreList.Add(Ellipsoid,Sphere);
  ignoreList.Add(Ellipsoid,Triangle);
  ignoreList.Add(Frustum,Ray);
  ignoreList.Add(Frustum,Segment);
  ignoreList.Add(Frustum,Aabb);
  ignoreList.Add(Frustum,Capsule);
  ignoreList.Add(Frustum,Cylinder);
  ignoreList.Add(Frustum,Ellipsoid);
  ignoreList.Add(Frustum,Frustum);
  ignoreList.Add(Frustum,Obb);
  ignoreList.Add(Frustum,Plane);
  ignoreList.Add(Frustum,Sphere);
  ignoreList.Add(Frustum,Triangle);
  ignoreList.Add(Obb,Frustum);
  ignoreList.Add(Obb,Capsule);
  ignoreList.Add(Obb,Cylinder);
  ignoreList.Add(Obb,Ellipsoid);
  ignoreList.Add(Plane,Capsule);
  ignoreList.Add(Plane,Cylinder);
  ignoreList.Add(Plane,Ellipsoid);
  ignoreList.Add(Plane,Frustum);
  ignoreList.Add(Plane,Triangle);
  ignoreList.Add(Sphere,Cylinder);
  ignoreList.Add(Sphere,Ellipsoid);
  ignoreList.Add(Sphere,Frustum);
  ignoreList.Add(Plane,Capsule);
  ignoreList.Add(Plane,Cylinder);
  ignoreList.Add(Plane,Ellipsoid);
  ignoreList.Add(Plane,Plane);
  ignoreList.Add(Plane,Triangle);

  ignoreList.Add(Segment,Tetrahedra);
  ignoreList.Add(Aabb,Tetrahedra);
  ignoreList.Add(Capsule,Tetrahedra);
  ignoreList.Add(Cylinder,Tetrahedra);
  ignoreList.Add(Ellipsoid,Tetrahedra);
  ignoreList.Add(Frustum,Tetrahedra);
  ignoreList.Add(Obb,Tetrahedra);
  ignoreList.Add(Plane,Tetrahedra);
  ignoreList.Add(Sphere,Tetrahedra);
  ignoreList.Add(Triangle,Tetrahedra);
  ignoreList.Add(Tetrahedra,Tetrahedra);
  ignoreList.Add(ConvexMesh,Tetrahedra);

  ignoreList.Add(Triangle,Frustum);

  ignoreList.Add(Tetrahedra,Segment);
  ignoreList.Add(Tetrahedra,Aabb);
  ignoreList.Add(Tetrahedra,Capsule);
  ignoreList.Add(Tetrahedra,Cylinder);
  ignoreList.Add(Tetrahedra,Ellipsoid);
  ignoreList.Add(Tetrahedra,Frustum);
  ignoreList.Add(Tetrahedra,Obb);
  ignoreList.Add(Tetrahedra,Plane);
  ignoreList.Add(Tetrahedra,Sphere);
  ignoreList.Add(Tetrahedra,Triangle);
  ignoreList.Add(Tetrahedra,Tetrahedra);
  ignoreList.Add(Tetrahedra,ConvexMesh);

  ignoreList.Add(Ray,ConvexMesh);
  ignoreList.Add(Segment,ConvexMesh);
  ignoreList.Add(Aabb,ConvexMesh);
  ignoreList.Add(Capsule,ConvexMesh);
  ignoreList.Add(Cylinder,ConvexMesh);
  ignoreList.Add(Ellipsoid,ConvexMesh);
  ignoreList.Add(Frustum,ConvexMesh);
  ignoreList.Add(Obb,ConvexMesh);
  ignoreList.Add(Plane,ConvexMesh);
  ignoreList.Add(Sphere,ConvexMesh);
  ignoreList.Add(Triangle,ConvexMesh);
  ignoreList.Add(ConvexMesh,ConvexMesh);

  ignoreList.Add(ConvexMesh,Ray);
  ignoreList.Add(ConvexMesh,Segment);
  ignoreList.Add(ConvexMesh,Aabb);
  ignoreList.Add(ConvexMesh,Capsule);
  ignoreList.Add(ConvexMesh,Cylinder);
  ignoreList.Add(ConvexMesh,Ellipsoid);
  ignoreList.Add(ConvexMesh,Frustum);
  ignoreList.Add(ConvexMesh,Obb);
  ignoreList.Add(ConvexMesh,Plane);
  ignoreList.Add(ConvexMesh,Sphere);
  ignoreList.Add(ConvexMesh,Triangle);

  ignoreList.Add(SweptTriangle,Point);
  ignoreList.Add(SweptTriangle,Ray);
  ignoreList.Add(SweptTriangle,Segment);
  ignoreList.Add(SweptTriangle,Aabb);
  ignoreList.Add(SweptTriangle,Capsule);
  ignoreList.Add(SweptTriangle,Cylinder);
  ignoreList.Add(SweptTriangle,Ellipsoid);
  ignoreList.Add(SweptTriangle,Frustum);
  ignoreList.Add(SweptTriangle,Obb);
  ignoreList.Add(SweptTriangle,Plane);
  ignoreList.Add(SweptTriangle,Sphere);
  ignoreList.Add(SweptTriangle,Triangle);
  ignoreList.Add(SweptTriangle,Tetrahedra);
  ignoreList.Add(SweptTriangle,ConvexMesh);
  ignoreList.Add(SweptTriangle,SweptTriangle);

}
