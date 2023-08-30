#pragma once

#include "IDPair.hpp"
#include "Connection.hpp"
#include "VertexProp.hpp"
#include "EdgeProp.hpp"

#include <stack>

class ShaderGraph final {
public:
  static constexpr auto kRootNodeId{0};

  ShaderGraph() = default;
  ShaderGraph(const ShaderGraph &);
  ~ShaderGraph() = default;

  ShaderGraph &operator=(const ShaderGraph &);

  void clear();

  //
  // Vertices:
  //

  [[nodiscard]] std::optional<IDPair> getRoot() const;

  [[nodiscard]] VertexDescriptor
  addVertex(std::optional<std::string_view> label = std::nullopt,
            NodeFlags flags = NodeFlags::None);
  void removeVertex(VertexDescriptor);

  [[nodiscard]] VertexProp &getVertexProp(VertexDescriptor);
  [[nodiscard]] const VertexProp &getVertexProp(VertexDescriptor) const;

  [[nodiscard]] std::optional<VertexDescriptor> findVertex(int32_t id) const;

  [[nodiscard]] std::size_t countVertices() const;

  [[nodiscard]] auto vertices() const {
    return boost::make_iterator_range(boost::vertices(m_graph));
  }
  template <typename Func> void vertices(Func func) {
    for (const auto vd : vertices()) {
      func(vd, m_graph[vd]);
    }
  }

  //
  // Edges:
  //

  EdgeDescriptor addEdge(EdgeType, const Connection &);
  void removeEdge(EdgeDescriptor);

  [[nodiscard]] EdgeProp &getEdgeProp(EdgeDescriptor);
  [[nodiscard]] const EdgeProp &getEdgeProp(EdgeDescriptor) const;

  [[nodiscard]] std::optional<EdgeDescriptor> findEdge(int32_t id);
  [[nodiscard]] std::size_t countEdges() const;

  [[nodiscard]] std::size_t countOutEdges(VertexDescriptor) const;
  void removeOutEdges(VertexDescriptor);

  [[nodiscard]] Connection getConnection(EdgeDescriptor) const;

  auto edges() const {
    return boost::make_iterator_range(boost::edges(m_graph));
  }
  template <typename Func> void edges(Func func) {
    for (const auto &&ed : edges()) {
      func(ed, m_graph[ed]);
    }
  }

  // --

  [[nodiscard]] std::vector<Connection> findCycles() const;

  [[nodiscard]] std::stack<VertexDescriptor>
  getExecutionOrder(std::optional<VertexDescriptor> root = std::nullopt) const;

  // ---

  void save(std::ostream &) const;
  void load(std::istream &);

  void exportGraphviz(std::ostream &);

private:
  AdjacencyList<VertexProp, EdgeProp> m_graph;
  std::optional<IDPair> m_root{std::nullopt};

  int32_t m_nextVertexId{0};
  int32_t m_nextEdgeId{0};
};

[[nodiscard]] bool hasRoot(const ShaderGraph &);
[[nodiscard]] bool hasOutEdges(const ShaderGraph &, VertexDescriptor);

void removeVertices(ShaderGraph &, std::span<VertexDescriptor>);
void removeVertices(ShaderGraph &, std::initializer_list<VertexDescriptor>);

// Removes a node and its internal (child) nodes.
void removeNode(ShaderGraph &, VertexDescriptor);
void removeNodes(ShaderGraph &, std::span<VertexDescriptor>);

[[nodiscard]] bool isAcyclic(const ShaderGraph &);

[[nodiscard]] std::set<VertexDescriptor>
findConnectedVertices(const ShaderGraph &,
                      std::optional<VertexDescriptor> root = std::nullopt);

template <class Archive> void save(Archive &archive, const ShaderGraph &g) {
  std::ostringstream oss;
  g.save(oss);
  archive(oss.str());
}
template <class Archive> void load(Archive &archive, ShaderGraph &g) {
  std::string s;
  archive(s);
  std::istringstream iss{s};
  g.load(iss);
}

[[nodiscard]] IDPair clone(ShaderGraph &, VertexDescriptor source);
