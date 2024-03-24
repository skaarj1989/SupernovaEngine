// -- ARCHIVES:

#include "cereal/archives/binary.hpp"

// -- TYPES:
// Everything used inside `save/load/serialize(Archive &)`

#include "cereal/types/polymorphic.hpp"

#include "math/Serialization.hpp"
#include "cereal/types/string.hpp"  // NodeBase
#include "cereal/types/vector.hpp"  // CompoundNode
#include "cereal/types/variant.hpp" // ValueNode

// -- NODES:

#include "MaterialEditor/Nodes/Empty.hpp"

#include "MaterialEditor/ValueVariant.hpp"
#include "MaterialEditor/PropertyVariant.hpp"
#include "MaterialEditor/TextureParam.hpp"
#include "MaterialEditor/Attribute.hpp"
#include "MaterialEditor/BuiltInConstants.hpp"
#include "MaterialEditor/FrameBlockMember.hpp"
#include "MaterialEditor/CameraBlockMember.hpp"

#include "MaterialEditor/SplitVector.hpp"
#include "MaterialEditor/SplitMatrix.hpp"

#include "MaterialEditor/Nodes/Embedded.hpp"

#include "MaterialEditor/Nodes/Append.hpp"
#include "MaterialEditor/Nodes/VectorSplitter.hpp"
#include "MaterialEditor/Nodes/MatrixSplitter.hpp"
#include "MaterialEditor/Nodes/Swizzle.hpp"
#include "MaterialEditor/Nodes/Arithmetic.hpp"
#include "MaterialEditor/Nodes/MatrixTransform.hpp"
#include "MaterialEditor/Nodes/TextureSampling.hpp"
#include "MaterialEditor/Nodes/Scripted.hpp"
#include "MaterialEditor/Nodes/Custom.hpp"

#include "MaterialEditor/Nodes/VertexMaster.hpp"
#include "MaterialEditor/Nodes/SurfaceMaster.hpp"
#include "MaterialEditor/Nodes/PostProcessMaster.hpp"

template <class Archive> void serialize(Archive &, TextureParam &) {
  // Can not serialize right here.
  // A texture path must be relative to its own parent (graph) file.
}

CEREAL_REGISTER_TYPE(EmptyNode);

template class EmbeddedNode<ValueVariant>;
template class EmbeddedNode<PropertyVariant>;
template class EmbeddedNode<TextureParam>;
template class EmbeddedNode<Attribute>;
template class EmbeddedNode<BuiltInConstant>;
template class EmbeddedNode<BuiltInSampler>;
template class EmbeddedNode<FrameBlockMember>;
template class EmbeddedNode<CameraBlockMember>;

template class EmbeddedNode<SplitVector>;
template class EmbeddedNode<SplitMatrix>;

CEREAL_REGISTER_TYPE(EmbeddedNode<ValueVariant>);
CEREAL_REGISTER_TYPE(EmbeddedNode<PropertyVariant>);
CEREAL_REGISTER_TYPE(EmbeddedNode<TextureParam>);
CEREAL_REGISTER_TYPE(EmbeddedNode<Attribute>);
CEREAL_REGISTER_TYPE(EmbeddedNode<BuiltInConstant>);
CEREAL_REGISTER_TYPE(EmbeddedNode<BuiltInSampler>);
CEREAL_REGISTER_TYPE(EmbeddedNode<FrameBlockMember>);
CEREAL_REGISTER_TYPE(EmbeddedNode<CameraBlockMember>);

CEREAL_REGISTER_TYPE(EmbeddedNode<SplitVector>);
CEREAL_REGISTER_TYPE(EmbeddedNode<SplitMatrix>);

CEREAL_REGISTER_TYPE(CompoundNode);
CEREAL_REGISTER_TYPE(AppendNode);
CEREAL_REGISTER_TYPE(VectorSplitterNode);
CEREAL_REGISTER_TYPE(MatrixSplitterNode);
CEREAL_REGISTER_TYPE(SwizzleNode);
CEREAL_REGISTER_TYPE(ArithmeticNode);
CEREAL_REGISTER_TYPE(MatrixTransformNode);
CEREAL_REGISTER_TYPE(TextureSamplingNode);
CEREAL_REGISTER_TYPE(ScriptedNode);
CEREAL_REGISTER_TYPE(CustomNode);

CEREAL_REGISTER_TYPE(VertexMasterNode);
CEREAL_REGISTER_TYPE(SurfaceMasterNode);
CEREAL_REGISTER_TYPE(PostProcessMasterNode);

CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, EmptyNode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, EmbeddedNode<ValueVariant>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, EmbeddedNode<PropertyVariant>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, EmbeddedNode<TextureParam>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, EmbeddedNode<Attribute>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, EmbeddedNode<BuiltInConstant>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, EmbeddedNode<BuiltInSampler>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, EmbeddedNode<FrameBlockMember>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, EmbeddedNode<CameraBlockMember>)

CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, EmbeddedNode<SplitVector>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, EmbeddedNode<SplitMatrix>)

CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, CompoundNode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, AppendNode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, VectorSplitterNode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, MatrixSplitterNode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, SwizzleNode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, ArithmeticNode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, MatrixTransformNode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, TextureSamplingNode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, ScriptedNode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, CustomNode)

CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, VertexMasterNode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, SurfaceMasterNode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(NodeBase, PostProcessMasterNode)

CEREAL_REGISTER_DYNAMIC_INIT(NodeSerialization)
