#include "ShaderGraph.hpp"
#include "TraverseGraph.hpp"
#include "boost/graph/depth_first_search.hpp"
#include "boost/range/adaptor/transformed.hpp"
#include "boost/range/adaptor/reversed.hpp"
#include "boost/graph/graphviz.hpp"

#include "cereal/types/polymorphic.hpp"
#include "cereal/archives/binary.hpp"

#include "tracy/Tracy.hpp"

#include <ranges>
#include <format>

CEREAL_FORCE_DYNAMIC_INIT(NodeSerialization)

namespace {

constexpr char kMagicId[] = "sneSG";
constexpr auto kMagicIdSize = sizeof(kMagicId);

[[nodiscard]] auto buildVertexDescriptorToIndexMap(const ShaderGraph &g) {
  VertexDescriptorToIndexMap map;
  for (const auto *node : g.nodes()) {
    map[node->vertex.vd] = node->vertex.id;
  }
  return map;
}

[[nodiscard]] std::unordered_set<VertexDescriptor>
findConnectedVertices(const ShaderGraph &g,
                      const VertexDescriptor root = nullptr) {
  ZoneScopedN("ShaderGraph::findConnectedVertices");

  std::unordered_set<VertexDescriptor> result;
  auto s = g.getExecutionOrder(root);
  while (!s.empty()) {
    const auto c = s.top();
    s.pop();

    result.insert(c);
  }
  return result;
}

} // namespace

//
// ShaderGraph class:
//

ShaderGraph::ShaderGraph(const rhi::ShaderType shaderType)
    : m_graph{std::make_unique<Graph>()}, m_shaderType{shaderType} {}

ShaderGraph &ShaderGraph::clear() {
  m_graph = std::make_unique<Graph>();
  m_root = {};

  m_nextVertexId = 0;
  m_nextEdgeId = 0;

  return *this;
}
bool ShaderGraph::empty() const { return countVertices() == 0; }

rhi::ShaderType ShaderGraph::getShaderType() const { return m_shaderType; }

IDPair ShaderGraph::getRoot() const { return m_root; }

ShaderGraph &ShaderGraph::remove(const VertexDescriptor in) {
  assert(in);
  ZoneScopedN("ShaderGraph::remove(node)");

  std::vector<VertexDescriptor> vertexList;
  collectChildren(in, vertexList);

  for (const auto vd : vertexList | std::views::reverse) {
    publish(BeforeNodeRemoveEvent{
      .node = (*m_graph)[vd].get(),
      .phase = EventPhase::Default,
    });
    boost::clear_vertex(vd, *m_graph);
    boost::remove_vertex(vd, *m_graph);
  }
  return *this;
}
IDPair ShaderGraph::clone(const VertexDescriptor source,
                          const std::optional<VertexID> hint) {
  ZoneScopedN("ShaderGraph::clone(node)");

  const auto *sourceNode = getNode(source);
  const auto vertex = _createVertex(hint);
  (*m_graph)[vertex.vd] = sourceNode->clone(vertex);
  auto *node = (*m_graph)[vertex.vd].get();
  node->graph = this;
  node->vertex = vertex;
  node->label = sourceNode->label;
  node->flags = sourceNode->flags;
  node->joinChildren();
  publish(NodeAddedEvent{.node = node, .phase = EventPhase::Default});
  return vertex;
}

VertexDescriptor ShaderGraph::findVertex(const VertexID id) const {
  for (const auto *node : nodes()) {
    if (node && node->vertex.id == id) return node->vertex.vd;
  }
  return nullptr;
}

std::size_t ShaderGraph::countVertices() const {
  return boost::num_vertices(*m_graph);
}

const ShaderGraph &ShaderGraph::updateVertexDescriptor(IDPair &vertex) const {
  assert(vertex.id != kInvalidId);
  if (vertex.vd == nullptr) vertex.vd = findVertex(vertex.id);
  return *this;
}
const ShaderGraph &ShaderGraph::updateVertexDescriptor(Connection &c) const {
  return updateVertexDescriptor(c.source).updateVertexDescriptor(c.target);
}

NodeBase *ShaderGraph::findNode(const VertexID id) {
  return const_cast<NodeBase *>(
    const_cast<const ShaderGraph *>(this)->findNode(id));
}
const NodeBase *ShaderGraph::findNode(const VertexID id) const {
  for (auto *node : nodes()) {
    if (node && node->vertex.id == id) return node;
  }
  return nullptr;
}

NodeBase *ShaderGraph::getNode(const VertexDescriptor vd) {
  return const_cast<NodeBase *>(
    const_cast<const ShaderGraph *>(this)->getNode(vd));
}
const NodeBase *ShaderGraph::getNode(const VertexDescriptor vd) const {
  return (*m_graph)[vd].get();
}

boost::iterator_range<ShaderGraph::VertexIterator>
ShaderGraph::vertices() const {
  return boost::make_iterator_range(boost::vertices(*m_graph));
}
boost::iterator_range<ShaderGraph::NodeIterator> ShaderGraph::nodes() {
  return boost::make_iterator_range(
    boost::vertices(*m_graph) |
    boost::adaptors::transformed(
      [this](const auto vd) { return (*m_graph)[vd].get(); }));
}
boost::iterator_range<ShaderGraph::ConstNodeIterator>
ShaderGraph::nodes() const {
  return boost::make_iterator_range(
    boost::vertices(*m_graph) |
    boost::adaptors::transformed(
      [this](const auto vd) { return (*m_graph)[vd].get(); }));
}

const ShaderGraph &
ShaderGraph::collectChildren(const VertexDescriptor vd,
                             std::vector<VertexDescriptor> &out) const {
  out.push_back(vd);
  getNode(vd)->collectChildren(out);
  return *this;
}

std::optional<EdgeDescriptor>
ShaderGraph::link(const EdgeType type, const ConnectedVDs c,
                  const std::optional<EdgeID> hint) {
  ZoneScopedN("ShaderGraph::link");

  const auto id = hint ? *hint : m_nextEdgeId++;
  const auto [ed, inserted] =
    boost::add_edge(c.first, c.second, EdgeProperty{id, type}, *m_graph);
  assert(inserted);
  if (!_isAcyclic()) {
    boost::remove_edge(ed, *m_graph);
    return std::nullopt;
  }
  if (type == EdgeType::External) {
    publish(EdgeAddedEvent{
      .source = (*m_graph)[ed.m_source].get(),
      .target = (*m_graph)[ed.m_target].get(),
      .phase = EventPhase::Default,
    });
  }
  return ed;
}
ShaderGraph &ShaderGraph::remove(const EdgeDescriptor &ed) {
  assert(ed != EdgeDescriptor{});
  if (getProperty(ed).type == EdgeType::External) {
    publish(BeforeEdgeRemoveEvent{
      .source = (*m_graph)[ed.m_source].get(),
      .target = (*m_graph)[ed.m_target].get(),
      .phase = EventPhase::Default,
    });
  }
  boost::remove_edge(ed, *m_graph);
  return *this;
}
ShaderGraph &ShaderGraph::removeOutEdges(const VertexDescriptor vd) {
  assert(vd != nullptr);
  boost::clear_out_edges(vd, *m_graph);
  return *this;
}

std::optional<EdgeDescriptor> ShaderGraph::findEdge(const EdgeID id) const {
  for (const auto &ed : edges()) {
    if (getProperty(ed).id == id) return ed;
  }
  return std::nullopt;
}

EdgeProperty &ShaderGraph::getProperty(const EdgeDescriptor &ed) {
  return const_cast<EdgeProperty &>(
    const_cast<const ShaderGraph *>(this)->getProperty(ed));
}
const EdgeProperty &ShaderGraph::getProperty(const EdgeDescriptor &ed) const {
  return (*m_graph)[ed];
}

Connection ShaderGraph::getConnection(const EdgeDescriptor &ed) const {
  assert(ed != EdgeDescriptor{});
  return {
    .source = getNode(ed.m_source)->vertex,
    .target = getNode(ed.m_target)->vertex,
  };
}

std::size_t ShaderGraph::countEdges() const {
  return boost::num_edges(*m_graph);
}
std::size_t ShaderGraph::countOutEdges(const VertexDescriptor vd) const {
  assert(vd != nullptr);
  return boost::out_degree(vd, *m_graph);
}

boost::iterator_range<ShaderGraph::EdgeIterator> ShaderGraph::edges() const {
  return boost::make_iterator_range(boost::edges(*m_graph));
}
boost::iterator_range<ShaderGraph::LinkIterator> ShaderGraph::links() const {
  return boost::make_iterator_range(
    boost::edges(*m_graph) |
    boost::adaptors::transformed([this](const auto &ed) {
      return LinkInfo{getProperty(ed), getConnection(ed)};
    }));
}

const ShaderGraph &ShaderGraph::exportGraphviz(std::ostream &os) const {
  struct GraphWriter {
    void operator()(std::ostream &os) const {
      os << "rankdir=RL\n"
            "node[shape=record]\n";
    }
  };

  struct VertexWriter {
    Graph &graph;

    void operator()(std::ostream &os, VertexDescriptor vd) const {
      const auto &node = *graph[vd];
      auto label = std::format(R"([{}]: label="{}"|type={})", node.vertex.id,
                               node.label, node.toString());
      boost::replace_all(label, "<", "\\<");
      boost::replace_all(label, ">", "\\>");
      os << "[label=" << boost::escape_dot_string(label) << "]";
    }
  };
  struct EdgeWriter {
    Graph &graph;

    void operator()(std::ostream &os, const EdgeDescriptor &ed) const {
      const auto &prop = graph[ed];
      const auto label = std::format(
        "{}{}", prop.id, prop.type == EdgeType::External ? 'e' : 'i');
      os << "[label=" << boost::escape_dot_string(label) << "]";
    }
  };

  boost::write_graphviz(
    os, *m_graph, VertexWriter{*m_graph}, EdgeWriter{*m_graph}, GraphWriter{},
    boost::make_assoc_property_map(buildVertexDescriptorToIndexMap(*this)));

  return *this;
}

const ShaderGraph &ShaderGraph::save(std::ostream &os) const {
  ZoneScopedN("ShaderGraph::save");
  {
    cereal::BinaryOutputArchive archive{os};
    archive.saveBinary(kMagicId, kMagicIdSize);

    archive(m_nextVertexId);

    // -- Vertices:

    const auto numVertices = countVertices();
    archive(numVertices);

    for (const auto vd : vertices()) {
      auto &node = (*m_graph)[vd];
      archive(node);
    }

    // -- Edges:
    // Only external edges are saved (internal ones are recreated in `load`).
    // Edge IDs don't matter and can be regenerated during loading.

    auto filteredLinks = links() | std::views::filter(externalLink);
    const std::size_t numLinks = std::ranges::distance(filteredLinks);
    archive(numLinks);

    for (const auto &[_, connection] : filteredLinks) {
      archive(connection);
    }
  }
  return *this;
}

ShaderGraph &ShaderGraph::load(std::istream &is) {
  ZoneScopedN("ShaderGraph::load");
  clear();

  cereal::BinaryInputArchive archive{is};

  std::remove_const_t<decltype(kMagicId)> magicId{};
  archive.loadBinary(magicId, kMagicIdSize);
  if (strncmp(magicId, kMagicId, kMagicIdSize) != 0)
    throw std::runtime_error{"Invalid ShaderGraph signature."};

  archive(m_nextVertexId);

  // -- Vertices:

  std::size_t numVertices;
  archive(numVertices);
  if (numVertices <= 0) {
    throw std::runtime_error{"Can't load a graph without vertices."};
  }

  for (decltype(numVertices) i{0}; i < numVertices; ++i) {
    const auto vd = boost::add_vertex(*m_graph);
    auto &node = (*m_graph)[vd];
    archive(node);
    node->graph = this;
    node->vertex.vd = vd;
    publish(NodeAddedEvent{.node = node.get(), .phase = EventPhase::Load});
  }
  for (auto *node : nodes() | boost::adaptors::reversed |
                      std::views::filter(nonInternalNode)) {
    node->joinChildren();
  }
  m_root = IDPair{findVertex(kRootNodeId), kRootNodeId};

  // -- Edges (only external):

  std::size_t numEdges;
  archive(numEdges);

  for (decltype(numEdges) i{0}; i < numEdges; ++i) {
    Connection c{};
    archive(c);
    updateVertexDescriptor(c);
    auto [ed, inserted] = boost::add_edge(
      c.source, c.target, {.type = EdgeType::External}, *m_graph);
    assert(inserted);
    publish(EdgeAddedEvent{
      .source = (*m_graph)[ed.m_source].get(),
      .target = (*m_graph)[ed.m_target].get(),
      .phase = EventPhase::Load,
    });
  }

  for (const auto &[i, ed] : edges() | std::views::enumerate) {
    getProperty(ed).id = static_cast<EdgeID>(i);
  }
  m_nextEdgeId = static_cast<EdgeID>(countEdges());

  return *this;
}

std::string ShaderGraph::makeSnapshot(const VertexDescriptor vd) const {
  ZoneScopedN("ShaderGraph::makeSnapshot");

  std::vector<VertexDescriptor> hierarchy;
  collectChildren(vd, hierarchy);
  std::ostringstream oss;
  {
    cereal::BinaryOutputArchive archive{oss};

    archive(hierarchy.size());
    for (const auto vd_ : hierarchy) {
      auto &node = (*m_graph)[vd_];
      archive(node);
    }
  }
  return oss.str();
}
ShaderGraph &ShaderGraph::loadSnapshot(const std::string &snapshot) {
  ZoneScopedN("ShaderGraph::loadSnapshot");

  std::istringstream iss{snapshot};
  cereal::BinaryInputArchive archive{iss};

  std::size_t numVertices;
  archive(numVertices);

  std::vector<VertexDescriptor> loaded(numVertices);
  for (decltype(numVertices) i{0}; i < numVertices; ++i) {
    const auto vd = boost::add_vertex(*m_graph);
    auto &node = (*m_graph)[vd];
    archive(node);
    node->graph = this;
    node->vertex.vd = vd;
    publish(NodeAddedEvent{.node = node.get(), .phase = EventPhase::Restore});

    loaded[i] = vd;
  }
  for (const auto vd : loaded | std::views::reverse) {
    getNode(vd)->joinChildren();
  }

  return *this;
}

bool ShaderGraph::isLinkedToRoot(const VertexDescriptor vd) const {
  return findConnectedVertices(*this).contains(vd);
}

std::stack<VertexDescriptor>
ShaderGraph::getExecutionOrder(VertexDescriptor root) const {
  ZoneScopedN("ShaderGraph::getExecutionOrder");

  if (!root) root = getRoot();

  std::stack<VertexDescriptor> stack;
  if (root != nullptr) {
    traverse(*m_graph, root, [&stack](const auto vd) { stack.push(vd); });
  }
  return stack;
}

//
// (private):
//

IDPair ShaderGraph::_createVertex(const std::optional<VertexID> hint) {
  ZoneScopedN("ShaderGraph::createVertex");

  const auto id = hint ? *hint : m_nextVertexId;
  assert(!findVertex(id));
  const auto vertex = IDPair{boost::add_vertex(*m_graph), id};
  if (!m_root.isValid()) [[unlikely]] {
    m_root = vertex;
  }
  m_nextVertexId = std::max(m_nextVertexId, hint.value_or(m_nextVertexId)) + 1;
  return vertex;
}

bool ShaderGraph::_isAcyclic() const {
  ZoneScopedN("ShaderGraph::_isAcyclic");

  std::vector<boost::default_color_type> colorMap(m_nextVertexId);
  auto descriptorToIndex = buildVertexDescriptorToIndexMap(*this);
  auto ipmap = boost::make_assoc_property_map(descriptorToIndex);
  auto cpmap = boost::make_iterator_property_map(colorMap.begin(), ipmap);

  class CycleDetector final : public boost::default_dfs_visitor {
  public:
    void back_edge(const EdgeDescriptor &, const Graph &) {
      throw std::runtime_error{"Cycle detected."};
    }
  };
  try {
    boost::depth_first_search(*m_graph, CycleDetector{}, cpmap);
  } catch (...) {
    return false;
  }
  return true;
}

//
// Helper:
//

bool externalLink(const std::pair<const EdgeProperty, const Connection> &p) {
  return p.first.type == EdgeType::External;
}
bool nonInternalNode(const NodeBase *node) {
  return !bool(node->flags & NodeBase::Flags::Internal);
}

Connection transform(ShaderGraph &g, const ConnectedIDs c) {
  return {
    .source = {g.findVertex(c.first), c.first},
    .target = {g.findVertex(c.second), c.second},
  };
}

LinkedNodes extractNodes(const ShaderGraph &g, const Connection &c) {
  return std::pair{g.getNode(c.source.vd), g.getNode(c.target.vd)};
}
LinkedNodes extractNodes(const ShaderGraph &g, const ConnectedIDs ids) {
  return std::pair{g.findNode(ids.first), g.findNode(ids.second)};
}

void collectExternalEdges(const ShaderGraph &g, const VertexID id,
                          std::unordered_set<EdgeDescriptor> &out) {
  return collectExternalEdges(g, g.findVertex(id), out);
}
void collectExternalEdges(const ShaderGraph &g, const VertexDescriptor vd,
                          std::unordered_set<EdgeDescriptor> &out) {
  std::vector<VertexDescriptor> hierarchy;
  g.collectChildren(vd, hierarchy);

  const auto inHierarchy = [&g, &hierarchy](const auto &ed) {
    return g.getProperty(ed).type == EdgeType::External &&
           std::ranges::any_of(hierarchy, [&ed](const auto vd) {
             return ed.m_source == vd || ed.m_target == vd;
           });
  };
  std::ranges::copy(g.edges() | std::views::filter(inHierarchy),
                    std::inserter(out, out.begin()));
}
