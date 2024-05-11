#include "ImGuiModal.hpp"
#include "IconsFontAwesome6.h"

const char *toString(ModalButton button) {
  switch (button) {
    using enum ModalButton;
  case Ok:
    return ICON_FA_CHECK " Ok";
  case Yes:
    return ICON_FA_CHECK " Yes";
  case No:
    return ICON_FA_XMARK " No";
  case Cancel:
    return ICON_FA_BAN " Cancel";
  }
  assert(false);
  return "";
}
