#include "MetaRender/AmbientLightScale.hpp"

rstl::pair< int, float > MakeAmbientLightScale(int editorId, float scale) {
  return rstl::pair< int, float >(editorId, scale);
}
