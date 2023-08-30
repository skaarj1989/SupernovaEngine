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

using VertexDescriptorToIndexMap = std::map<VertexDescriptor, std::size_t>;
using VertexIndexToDescriptorMap = std::map<std::size_t, VertexDescriptor>;

constexpr auto kInvalidId = -1;
