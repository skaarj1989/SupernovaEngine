#pragma once

#include <span>

#define MAKE_SPAN(Var, Member)                                                 \
  std::span { (Var).m##Member, (Var).mNum##Member }
