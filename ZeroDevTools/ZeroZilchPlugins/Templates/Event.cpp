#include "$rootnamespace$Precompiled.hpp"

//***************************************************************************
ZilchDefineType($safeitemrootname$, $rootnamespace$Library, builder, type)
{
  // This is required for event binding
  ZilchBindDestructor();
  ZilchBindConstructor();
}

//***************************************************************************
$safeitemrootname$::$safeitemrootname$()
{
}

//***************************************************************************
$safeitemrootname$::~$safeitemrootname$()
{
}
