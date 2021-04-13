#pragma once

#include "Engine/EngineContainers.hpp"

namespace Zero
{

struct WikiUpdatePage
{
  WikiUpdatePage() {}
  WikiUpdatePage(StringParam page, StringParam parent)
  {
    mPageToUpdate = page;
    mParentPage = parent;
  }

  void Serialize(Serializer& stream)
  {
    SerializeName(mPageToUpdate);
    SerializeName(mParentPage);
  }

  String mPageToUpdate;
  String mParentPage;
};

struct EventEntry
{
  EventEntry() {}
  EventEntry(StringParam eventType, StringParam eventName)
  {
    mEventType = eventType;
    mEventName = eventName;
  }

  void Serialize(Serializer& stream)
  {
    SerializeName(mEventType);
    SerializeName(mEventName);
  }

  String mEventType;
  String mEventName;
};


}
