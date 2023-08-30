add_library(Common INTERFACE
  "include/AlwaysFalse.hpp"
  "include/TypeTraits.hpp"
  "include/MapOptional.hpp"
  "include/MergeVector.hpp"
  "include/VisitorHelper.hpp"
  "include/VariantCast.hpp"
  "include/VariantIndex.hpp"
  "include/VariantFromIndex.hpp"
  "include/ScopedEnumFlags.hpp"
)
target_include_directories(Common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(Common PROPERTIES FOLDER "Framework")
