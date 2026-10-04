#include "GuiSys/CGuiWidgetIdDB.hpp"

namespace {
extern const short kInvalidWidgetId;
}

CGuiWidgetIdDB::CGuiWidgetIdDB()
: mInvalidWidgetName(rstl::string_l("kGSYS_InvalidWidgetID")) {}

void CGuiWidgetIdDB::Reserve(int size) { mNames.reserve(size + mNames.size()); }

short CGuiWidgetIdDB::AddWidget(const rstl::string& name) {
  if (name == rstl::string_l("kGSYS_DummyWidgetID") ||
      name == rstl::string_l("kGSYS_InvalidWidgetID")) {
    return -1;
  }

  short id = FindWidgetID(name);
  if (id == kInvalidWidgetId) {
    mNames.push_back_unsafe(name);
    id = mNames.size() - 1;
  }
  return id;
}

short CGuiWidgetIdDB::FindWidgetID(const rstl::string& name) const {
  for (int i = 0; i < mNames.size(); ++i) {
    if (mNames[i] == name) {
      return i;
    }
  }

  return kInvalidWidgetId;
}

namespace {
const short kInvalidWidgetId = -1;
}
