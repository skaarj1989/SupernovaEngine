#include "PrintSceneStats.hpp"
#include <ostream>
#include <format>

namespace {

std::ostream &operator<<(std::ostream &os, const aiVector3D &v) {
  os << std::format("vec3(x={},y={},z={})", v.x, v.y, v.z);
  return os;
}
std::ostream &operator<<(std::ostream &os, const aiQuaternion &q) {
  os << std::format("quat(x={},y={},z={},w={})", q.x, q.y, q.z, q.w);
  return os;
}
std::ostream &operator<<(std::ostream &os, const aiMatrix4x4 &m) {
  aiVector3D s;
  aiQuaternion r;
  aiVector3D t;
  m.Decompose(s, r, t);

  os << "T=" << t << std::endl
     << "R=" << r << std::endl
     << "S=" << s << std::endl;
  return os;
}

} // namespace

std::ostream &operator<<(std::ostream &os, const aiScene &scene) {
  os << "Root transform:\n"
     << scene.mRootNode->mTransformation << std::endl

     << "Num meshes: " << scene.mNumMeshes << std::endl
     << "Num materials: " << scene.mNumMaterials << std::endl
     << "Num animations: " << scene.mNumAnimations << std::endl;
  return os;
}
