#pragma once

class $safeitemrootname$ : public ZeroEngine::ZilchComponent
{
public:
  ZilchDeclareDerivedType($safeitemrootname$, ZeroEngine::ZilchComponent);
  
  $safeitemrootname$();
  ~$safeitemrootname$();
  
  void Initialize(ZeroEngine::CogInitializer* initializer);
  
  void OnLogicUpdate(ZeroEngine::UpdateEvent* event);
};
