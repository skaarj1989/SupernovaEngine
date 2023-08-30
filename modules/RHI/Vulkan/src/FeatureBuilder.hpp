#pragma once

#include "glad/vulkan.h"
#include <concepts>
#include <memory>
#include <unordered_map>

namespace rhi {

template <typename T>
concept VkFeatureInfo = requires {
  { T::sType } -> std::same_as<VkStructureType &>;
  { T::pNext } -> std::same_as<void *&>;
};

class FeatureBuilder final {
public:
  explicit FeatureBuilder(VkPhysicalDevice physicalDevice)
      : m_physicalDevice{physicalDevice} {
    assert(m_physicalDevice != VK_NULL_HANDLE);
  }

  template <VkFeatureInfo T>
  [[nodiscard]] T &requestExtensionFeatures(VkStructureType sType) {
    if (auto it = m_extensionFeatures.find(sType);
        it != m_extensionFeatures.end()) {
      return *static_cast<T *>(it->second.get());
    }

    T extension{.sType = sType};
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR,
      .pNext = &extension,
    };
    vkGetPhysicalDeviceFeatures2(m_physicalDevice, &physicalDeviceFeatures);

    auto [it, _] = m_extensionFeatures.insert(
      {sType, std::make_shared<T>(std::move(extension))});
    auto ext = static_cast<T *>(it->second.get());
    if (m_lastRequestedExtensionFeature)
      ext->pNext = m_lastRequestedExtensionFeature;

    m_lastRequestedExtensionFeature = ext;
    return *ext;
  }

  [[nodiscard]] const void *getRoot() const {
    return m_lastRequestedExtensionFeature;
  }

private:
  VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};

  // The only reason for using shared_ptr is that it implements type-erasure
  // whilst unique_ptr does not.
  std::unordered_map<VkStructureType, std::shared_ptr<void>>
    m_extensionFeatures;
  void *m_lastRequestedExtensionFeature{nullptr};
};

} // namespace rhi
