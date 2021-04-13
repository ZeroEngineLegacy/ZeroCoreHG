#include "$rootnamespace$Precompiled.hpp"

//***************************************************************************
ZilchDefineType($safeitemrootname$, $rootnamespace$Library, builder, type)
{
  // This is required for component binding
  ZilchBindDestructor();
  ZilchBindConstructor();
  ZilchBindMethod(Initialize);
  
  // Note: All event connection methods must be bound
  ZilchBindMethod(OnLogicUpdate);
}

//***************************************************************************
$safeitemrootname$::$safeitemrootname$()
{
}

//***************************************************************************
$safeitemrootname$::~$safeitemrootname$()
{
}

//***************************************************************************
void $safeitemrootname$::Initialize(ZeroEngine::CogInitializer* initializer)
{
  //ZeroConnectThisTo(this->GetSpace(), "LogicUpdate", "OnLogicUpdate");
}

//***************************************************************************
void $safeitemrootname$::OnLogicUpdate(ZeroEngine::UpdateEvent* event)
{
}
