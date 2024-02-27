#include "MaterialEditor/ShaderGraph.hpp"

#include "TypeTraits.hpp"
#include "AlwaysFalse.hpp"

#include "CycleDetector.hpp"
#include "TraverseGraph.hpp"
#include "boost/graph/graphviz.hpp"

#include "cereal/archives/binary.hpp"
#include "math/Serialization.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/optional.hpp"
#include "cereal/types/variant.hpp"

#include "tracy/Tracy.hpp"

namespace {

constexpr char kMagicId[] = "sneSG";
constexpr auto kMagicIdSize = sizeof(kMagicId);

[[nodiscard]] auto getRoot(auto &g) {
  assert(boost::num_vertices(g) > 0);
  return IDPair{*boost::vertices(g).first, ShaderGraph::kRootNodeId};
}

void deepCopy(const ShaderGraph &from, ShaderGraph &to) {
  std::stringstream ss;
  from.save(ss);
  to.load(ss);
}

[[nodiscard]] auto buildVertexDescriptorToIndexMap(const ShaderGraph &g) {
  VertexDescriptorToIndexMap map;
  for (const auto vd : g.vertices()) {
    map[vd] = g.getVertexProp(vd).id;
  }
  return map;
}

void _removeVertices(ShaderGraph &g, auto &container) {
  for (const auto vd : container) {
    g.removeVertex(vd);
  }
}

} // namespace

//
// ShaderGraph class:
//

ShaderGraph::ShaderGraph(const ShaderGraph &other) {
  // Don't use copy assignment constructor/operator for the m_graph!
  // a variant in VertexProp may contain a child vertex.
  // VertexDescriptor is a pointer, unique for a graph (can't be copied).
  deepCopy(other, *this);
}
ShaderGraph &ShaderGraph::operator=(const ShaderGraph &rhs) {
  if (this != &rhs) {
    deepCopy(rhs, *this);
  }
  return *this;
}

void ShaderGraph::clear() {
  m_graph = {};
  m_root = std::nullopt;

  m_nextVertexId = 0;
  m_nextEdgeId = 0;
}

std::optional<IDPair> ShaderGraph::getRoot() const { return m_root; }

VertexDescriptor ShaderGraph::addVertex(std::optional<std::string_view> label,
                                        NodeFlags flags) {
  const auto id = m_nextVertexId++;
  const auto vd = boost::add_vertex(
    VertexProp{
      .id = id,
      .flags = flags,
      .label = label.value_or("").data(),
    },
    m_graph);
  if (!hasRoot(*this)) [[unlikely]] {
    m_root = {vd, id};
  }
  return vd;
}
void ShaderGraph::removeVertex(VertexDescriptor vd) {
  assert(vd);
  boost::clear_vertex(vd, m_graph);
  boost::remove_vertex(vd, m_graph);
}

VertexProp &ShaderGraph::getVertexProp(VertexDescriptor vd) {
  assert(vd != nullptr);
  return m_graph[vd];
}
const VertexProp &ShaderGraph::getVertexProp(VertexDescriptor vd) const {
  assert(vd != nullptr);
  return m_graph[vd];
}

std::optional<VertexDescriptor> ShaderGraph::findVertex(int32_t id) const {
  for (const auto vd : vertices()) {
    if (m_graph[vd].id == id) return vd;
  }
  return std::nullopt;
}

std::size_t ShaderGraph::countVertices() const {
  return boost::num_vertices(m_graph);
}

EdgeDescriptor ShaderGraph::addEdge(EdgeType type,
                                    const Connection &connection) {
  assert(connection.isValid());

  const auto [ed, _] =
    boost::add_edge(connection.from, connection.to,
                    EdgeProp{.id = m_nextEdgeId++, .type = type}, m_graph);

  return ed;
}
void ShaderGraph::removeEdge(EdgeDescriptor ed) {
  assert(ed != EdgeDescriptor{});
  boost::remove_edge(ed, m_graph);
}

EdgeProp &ShaderGraph::getEdgeProp(EdgeDescriptor ed) { return m_graph[ed]; }
const EdgeProp &ShaderGraph::getEdgeProp(EdgeDescriptor ed) const {
  return m_graph[ed];
}

std::optional<EdgeDescriptor> ShaderGraph::findEdge(int32_t id) {
  for (const auto &&ed : edges()) {
    if (m_graph[ed].id == id) return ed;
  }
  return std::nullopt;
}
std::size_t ShaderGraph::countEdges() const {
  return boost::num_edges(m_graph);
}

std::size_t ShaderGraph::countOutEdges(VertexDescriptor vd) const {
  assert(vd != nullptr);
  return boost::out_degree(vd, m_graph);
}
void ShaderGraph::removeOutEdges(VertexDescriptor vd) {
  assert(vd != nullptr);
  boost::clear_out_edges(vd, m_graph);
}

Connection ShaderGraph::getConnection(EdgeDescriptor ed) const {
  assert(ed != EdgeDescriptor{});
  return {
    .from = boost::source(ed, m_graph),
    .to = boost::target(ed, m_graph),
  };
}

std::vector<Connection> ShaderGraph::findCycles() const {
  // Do not use 'countVertices' for colorMap size.
  // Otherwise it will trigger: 'out of range vector iterator' in
  // boost::depth_first_search
  std::vector<boost::default_color_type> colorMap(m_nextVertexId);
  auto descriptorToIndex = buildVertexDescriptorToIndexMap(*this);
  auto ipmap = boost::make_assoc_property_map(descriptorToIndex);
  auto cpmap = boost::make_iterator_property_map(colorMap.begin(), ipmap);

  std::vector<Connection> cycles;
  boost::depth_first_search(m_graph,
                            CycleDetector{[this, &cycles](const auto &ed) {
                              cycles.emplace_back(getConnection(ed));
                            }},
                            cpmap);

  return cycles;
}

std::stack<VertexDescriptor>
ShaderGraph::getExecutionOrder(std::optional<VertexDescriptor> root) const {
  assert(isAcyclic(*this));

  ZoneScopedN("ShaderGraph::GetExecutionOrder");
  if (!root) root = getRoot()->vd;

  std::stack<VertexDescriptor> stack;
  if (root) {
    traverse(m_graph, *root, [&stack](const auto vd) { stack.push(vd); });
  }
  return stack;
}

void ShaderGraph::save(std::ostream &os) const {
  ZoneScopedN("ShaderGraph::Save");

  const auto descriptorToIndex = buildVertexDescriptorToIndexMap(*this);
  {
    UserDataAdapter<const VertexDescriptorToIndexMap,
                    cereal::BinaryOutputArchive>
      archive{descriptorToIndex, os};

    archive.saveBinary(kMagicId, kMagicIdSize);
    archive(m_nextVertexId, m_nextEdgeId);

    // -- Vertices:

    const auto numVertices = countVertices();
    archive(numVertices);

    for (const auto vd : vertices()) {
      archive(m_graph[vd]);
    }
    for (const auto vd : vertices()) {
      archive(m_graph[vd].variant);
    }

    // -- Edges:

    const auto numEdges = countEdges();
    archive(numEdges);

    for (const auto &&ed : edges()) {
      archive(m_graph[ed], getConnection(ed));
    }
  }
}
void ShaderGraph::load(std::istream &is) {
  ZoneScopedN("ShaderGraph::Load");

  clear();

  VertexIndexToDescriptorMap indexToDescriptor;
  UserDataAdapter<const VertexIndexToDescriptorMap, cereal::BinaryInputArchive>
    archive{indexToDescriptor, is};

  std::remove_const_t<decltype(kMagicId)> magicId{};
  archive.loadBinary(magicId, kMagicIdSize);
  if (strncmp(magicId, kMagicId, kMagicIdSize) != 0)
    throw std::runtime_error{"Invalid ShaderGraph signature."};

  archive(m_nextVertexId, m_nextEdgeId);

  // -- Vertices:

  std::size_t numVertices;
  archive(numVertices);
  if (numVertices <= 0) {
    throw std::runtime_error{"Can't load a graph without vertices."};
  }

  for (decltype(numVertices) i{0}; i < numVertices; ++i) {
    const auto vd = boost::add_vertex(m_graph);
    auto &prop = m_graph[vd];
    archive(prop);

    indexToDescriptor[prop.id] = vd;
  }
  for (const auto vd : vertices()) {
    archive(m_graph[vd].variant);
  }

  if (numVertices > 0) m_root = ::getRoot(m_graph);

  // -- Edges:

  std::size_t numEdges;
  archive(numEdges);

  for (decltype(numEdges) i{0}; i < numEdges; ++i) {
    EdgeProp ep{};
    Connection connection{};
    archive(ep, connection);
    auto [_, inserted] =
      boost::add_edge(connection.from, connection.to, ep, m_graph);
    assert(inserted);
  }
}

void ShaderGraph::exportGraphviz(std::ostream &os) {
  struct GraphWriter {
    void operator()(std::ostream &os) const {
      os << "rankdir=RL\n"
            "node[shape=record]\n";
    }
  };

#define USE_CUSTOM_VERTEX_WRITER 1

#if USE_CUSTOM_VERTEX_WRITER
  struct VertexWriter {
    AdjacencyList<VertexProp, EdgeProp> &graph;

    void operator()(std::ostream &os, VertexDescriptor vd) const {
      os << "[label=" << boost::escape_dot_string(graph[vd].toString()) << "]";
    }
  };
#endif

  boost::write_graphviz(
    os, m_graph,
#if USE_CUSTOM_VERTEX_WRITER
    VertexWriter{m_graph},
#else
    boost::make_label_writer(boost::get(&VertexProp::label, m_graph)),
#endif
    boost::default_writer{}, GraphWriter{},
    boost::make_assoc_property_map(buildVertexDescriptorToIndexMap(*this)));
}

//
//
//

bool hasRoot(const ShaderGraph &g) { return g.getRoot().has_value(); }
bool hasOutEdges(const ShaderGraph &g, VertexDescriptor vd) {
  return g.countOutEdges(vd) > 0;
}

int32_t getVertexId(const ShaderGraph &g, VertexDescriptor vd) {
  return g.getVertexProp(vd).id;
}

void removeVertices(ShaderGraph &g, std::span<VertexDescriptor> v) {
  _removeVertices(g, v);
}
void removeVertices(ShaderGraph &g, std::initializer_list<VertexDescriptor> v) {
  _removeVertices(g, v);
}

void removeNode(ShaderGraph &g, VertexDescriptor vd) {
  assert(vd);

  auto &vertexProp = g.getVertexProp(vd);
  std::visit(
    [&g](auto &arg) {
      using T = std::decay_t<decltype(arg)>;

      if constexpr (std::is_enum_v<T> ||
                    is_any_v<T, std::monostate, ValueVariant, PropertyValue,
                             TextureParam>) {
        // No children nodes ...
      } else if constexpr (std::is_same_v<T, ContainerNode>) {
        arg.remove(g);
      } else if constexpr (std::is_same_v<T, CompoundNodeVariant>) {
        remove(g, arg);
      } else if constexpr (std::is_same_v<T, MasterNodeVariant>) {
        // Do not remove a MasterNode!
        assert(false);
      } else {
        static_assert(always_false_v<T>, "non-exhaustive visitor!");
      }
    },
    vertexProp.variant);

  g.removeVertex(vd);
}
void removeNodes(ShaderGraph &g, std::span<VertexDescriptor> v) {
  for (const auto vd : v)
    removeNode(g, vd);
}

bool isAcyclic(const ShaderGraph &g) { return g.findCycles().empty(); }

std::set<VertexDescriptor>
findConnectedVertices(const ShaderGraph &g,
                      std::optional<VertexDescriptor> root) {
  ZoneScopedN("ShaderGraph::FindConnectedVertices");

  if (!root) root = g.getRoot()->vd;

  std::set<VertexDescriptor> result;
  auto s = g.getExecutionOrder(root);
  while (!s.empty()) {
    const auto c = s.top();
    s.pop();

    result.insert(c);
  }
  return result;
}

IDPair clone(ShaderGraph &g, VertexDescriptor sourceVd) {
  const auto &sourceProp = g.getVertexProp(sourceVd);

  const auto newVd = g.addVertex(sourceProp.label, NodeFlags::Output);
  auto &targetProp = g.getVertexProp(newVd);

  std::visit(
    [&g, newVd, &targetVariant = targetProp.variant](const auto &arg) {
      using T = std::decay_t<decltype(arg)>;

      if constexpr (is_any_v<T, std::monostate, ValueVariant, Attribute,
                             PropertyValue, FrameBlockMember, CameraBlockMember,
                             BuiltInConstant, BuiltInSampler, SplitVector,
                             SplitMatrix, TextureParam>) {
        targetVariant = arg;
      } else if constexpr (std::is_same_v<T, ContainerNode>) {
        targetVariant = arg.clone(g, newVd);
      } else if constexpr (std::is_same_v<T, CompoundNodeVariant>) {
        std::visit(
          [&g, newVd, &targetVariant](const auto &compoundNode) {
            targetVariant = compoundNode.clone(g, newVd);
          },
          arg);
      } else if constexpr (std::is_same_v<T, MasterNodeVariant>) {
        // Can't clone a master node (one master node per graph).
        assert(false);
      } else {
        static_assert(always_false_v<T>, "non-exhaustive visitor!");
      }
    },
    sourceProp.variant);

  return IDPair{newVd, targetProp.id};
}
