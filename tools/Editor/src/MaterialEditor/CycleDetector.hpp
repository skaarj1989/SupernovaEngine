#pragma once

#include "boost/graph/depth_first_search.hpp"

template <typename Func>
class CycleDetector final : public boost::default_dfs_visitor {
public:
  explicit CycleDetector(Func f) : m_callback{std::move(f)} {}

  void back_edge(const EdgeDescriptor &ed, auto &) { m_callback(ed); }

private:
  Func m_callback;
};
