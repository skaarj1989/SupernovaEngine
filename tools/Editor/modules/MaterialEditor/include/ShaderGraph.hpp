#pragma once

#include "Nodes/NodeBase.hpp"
#include "EdgeProperty.hpp"
#include "entt/signal/emitter.hpp"
#include "boost/iterator/transform_iterator.hpp"
#include <memory>
#include <stack>
#include <unordered_set>

enum class EventPhase { Default = 0, Load, Restore };

struct NodeAddedEvent {
  NodeBase *node{nullptr};
  EventPhase phase{EventPhase::Default};
};
struct BeforeNodeRemoveEvent {
  NodeBase *node{nullptr};
  EventPhase phase{EventPhase::Default};
};
struct EdgeAddedEvent {
  NodeBase *source{nullptr};
  NodeBase *target{nullptr};
  EventPhase phase{EventPhase::Default};
};
struct BeforeEdgeRemoveEvent {
  NodeBase *source{nullptr};
  NodeBase *target{nullptr};
  EventPhase phase{EventPhase::Default};
};

class ShaderGraph : private entt::emitter<ShaderGraph> {
  friend class entt::emitter<ShaderGraph>;

  using Graph = AdjacencyList<std::unique_ptr<NodeBase>, EdgeProperty>;

  using VertexIterator = typename Graph::vertex_iterator;
  using NodeGetter = std::function<NodeBase *(const VertexDescriptor)>;
  using NodeIterator = boost::transform_iterator<NodeGetter, VertexIterator>;
  using ConstNodeGetter =
    std::function<const NodeBase *(const VertexDescriptor)>;
  using ConstNodeIterator =
    boost::transform_iterator<ConstNodeGetter, VertexIterator>;

  using EdgeIterator = typename Graph::edge_iterator;
  using LinkInfo = std::pair<const EdgeProperty, const Connection>;
  using LinkGetter = std::function<LinkInfo(const EdgeDescriptor &)>;
  using LinkIterator = boost::transform_iterator<LinkGetter, EdgeIterator>;

public:
  explicit ShaderGraph(const rhi::ShaderType);
  ShaderGraph(const ShaderGraph &) = delete;
  ShaderGraph(ShaderGraph &&) noexcept = default;
  ~ShaderGraph() = default;

  ShaderGraph &operator=(const ShaderGraph &) = delete;
  ShaderGraph &operator=(ShaderGraph &&) noexcept = default;

  using entt::emitter<ShaderGraph>::emitter;

  using entt::emitter<ShaderGraph>::on;
  using entt::emitter<ShaderGraph>::erase;

  static constexpr auto kRootNodeId = 0;

  ShaderGraph &clear();
  [[nodiscard]] bool empty() const;

  [[nodiscard]] rhi::ShaderType getShaderType() const;

  [[nodiscard]] IDPair getRoot() const;

  // -- Vertex:

  template <class NodeT, typename... Args>
    requires std::is_base_of_v<NodeBase, NodeT>
  auto add(std::optional<VertexID> hint = std::nullopt, Args &&...args) {
    const auto vertex = _createVertex(hint);
    (*m_graph)[vertex.vd] =
      std::make_unique<NodeT>(*this, vertex, std::forward<Args>(args)...);
    auto *node = (*m_graph)[vertex.vd].get();
    node->joinChildren();
    publish(NodeAddedEvent{.node = node, .phase = EventPhase::Default});
    return node;
  }
  ShaderGraph &remove(const VertexDescriptor);
  IDPair clone(const VertexDescriptor source,
               const std::optional<VertexID> hint = std::nullopt);

  [[nodiscard]] VertexDescriptor findVertex(const VertexID) const;

  [[nodiscard]] std::size_t countVertices() const;

  const ShaderGraph &updateVertexDescriptor(IDPair &) const;
  const ShaderGraph &updateVertexDescriptor(Connection &) const;

  NodeBase *findNode(const VertexID);
  const NodeBase *findNode(const VertexID) const;
  [[nodiscard]] NodeBase *getNode(const VertexDescriptor);
  [[nodiscard]] const NodeBase *getNode(const VertexDescriptor) const;

  [[nodiscard]] boost::iterator_range<VertexIterator> vertices() const;

  boost::iterator_range<NodeIterator> nodes();
  boost::iterator_range<ConstNodeIterator> nodes() const;

  const ShaderGraph &collectChildren(const VertexDescriptor,
                                     std::vector<VertexDescriptor> &) const;

  // -- Edge:

  std::optional<EdgeDescriptor>
  link(const EdgeType, const ConnectedVDs,
       const std::optional<EdgeID> hint = std::nullopt);
  ShaderGraph &remove(const EdgeDescriptor &);
  ShaderGraph &removeOutEdges(VertexDescriptor);

  [[nodiscard]] std::optional<EdgeDescriptor> findEdge(const EdgeID) const;

  [[nodiscard]] EdgeProperty &getProperty(const EdgeDescriptor &);
  [[nodiscard]] const EdgeProperty &getProperty(const EdgeDescriptor &) const;

  Connection getConnection(const EdgeDescriptor &) const;

  [[nodiscard]] std::size_t countEdges() const;
  [[nodiscard]] std::size_t countOutEdges(const VertexDescriptor) const;

  [[nodiscard]] boost::iterator_range<EdgeIterator> edges() const;
  boost::iterator_range<LinkIterator> links() const;

  // ---

  [[nodiscard]] bool isLinkedToRoot(const VertexDescriptor) const;

  [[nodiscard]] std::stack<VertexDescriptor>
  getExecutionOrder(VertexDescriptor root = nullptr) const;

  const ShaderGraph &exportGraphviz(std::ostream &) const;

  // ---

  const ShaderGraph &save(std::ostream &) const;
  ShaderGraph &load(std::istream &);

  [[nodiscard]] std::string makeSnapshot(const VertexDescriptor) const;
  ShaderGraph &loadSnapshot(const std::string &);

private:
  IDPair _createVertex(const std::optional<VertexID> hint = std::nullopt);

  [[nodiscard]] bool _isAcyclic() const;

private:
  std::unique_ptr<Graph> m_graph;
  IDPair m_root;

  VertexID m_nextVertexId{0};
  EdgeID m_nextEdgeId{0};

  rhi::ShaderType m_shaderType{};
};

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

//
// Helper:
//

using LinkedNodes = std::pair<const NodeBase *, const NodeBase *>;

[[nodiscard]] LinkedNodes extractNodes(const ShaderGraph &, const ConnectedIDs);

bool externalLink(const std::pair<const EdgeProperty, const Connection> &);
bool nonInternalNode(const NodeBase *);

Connection transform(ShaderGraph &, const ConnectedIDs);

void collectExternalEdges(const ShaderGraph &, const VertexID,
                          std::unordered_set<EdgeDescriptor> &);
void collectExternalEdges(const ShaderGraph &, const VertexDescriptor,
                          std::unordered_set<EdgeDescriptor> &);
