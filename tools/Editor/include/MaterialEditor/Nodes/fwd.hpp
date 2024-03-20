#pragma once

class NodeBase;

class EmptyNode;

enum class Attribute;
enum class BuiltInConstant;
enum class BuiltInSampler;
enum class FrameBlockMember;
enum class CameraBlockMember;
struct TextureParam;

enum class SplitVector;
enum class SplitMatrix;

template <typename T> class EmbeddedNode;

class AppendNode;
class VectorSplitterNode;
class MatrixSplitterNode;
class SwizzleNode;
class ArithmeticNode;
class MatrixTransformNode;
class TextureSamplingNode;
class ScriptedNode;
class CustomNode;

class VertexMasterNode;
class SurfaceMasterNode;
class PostProcessMasterNode;
