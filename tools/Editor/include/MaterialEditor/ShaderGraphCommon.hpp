#pragma once

#include "boost/graph/adjacency_list.hpp"

template <typename VertexProperty = boost::no_property,
          typename EdgeProperty = boost::no_property>
using AdjacencyList = boost::adjacency_list<boost::listS,     // OutEdgeList
                                            boost::listS,     // VertexList
                                            boost::directedS, // Directed
                                            VertexProperty, EdgeProperty,
                                            boost::no_property, boost::vecS>;

using VertexDescriptor = AdjacencyList<>::vertex_descriptor;
using EdgeDescriptor = AdjacencyList<>::edge_descriptor;

using VertexID = int32_t;
using EdgeID = int32_t;

using VertexDescriptorToIndexMap = std::map<VertexDescriptor, VertexID>;
using VertexIndexToDescriptorMap = std::map<VertexID, VertexDescriptor>;

constexpr auto kInvalidId = -1;

struct IDPair {
  VertexDescriptor vd{nullptr};
  VertexID id{kInvalidId};

  operator VertexID() const { return id; }
  operator VertexDescriptor() const { return vd; }

  [[nodiscard]] bool isValid() const {
    return vd != nullptr && id != kInvalidId;
  }

  template <class Archive> void serialize(Archive &archive) { archive(id); }
};

using ConnectedIDs = std::pair<VertexID, VertexID>;
using ConnectedVDs = std::pair<VertexDescriptor, VertexDescriptor>;

struct Connection {
  IDPair source;
  IDPair target;

  operator ConnectedIDs() const { return {source.id, target.id}; }
  operator ConnectedVDs() const { return {source.vd, target.vd}; }

  [[nodiscard]] bool isValid() const {
    return source.isValid() && target.isValid();
  }

  template <class Archive> void serialize(Archive &archive) {
    archive(source, target);
  }
};

namespace std {

template <> struct hash<EdgeDescriptor> {
  size_t operator()(const EdgeDescriptor &ed) const noexcept {
    return std::hash<VertexDescriptor>{}(ed.m_source) ^
           (std::hash<VertexDescriptor>{}(ed.m_target) << 1);
  }
};

} // namespace std
