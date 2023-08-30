#pragma once

// The one built in cereal does not compile inside {save/load}_minimal
// @see cereal/archives/adapters.hpp
template <class UserData, class Archive>
class UserDataAdapter : public Archive {
public:
  template <class... Args>
  UserDataAdapter(UserData &ud, Args &&...args)
      : Archive{std::forward<Args>(args)...}, userdata{ud} {}

  UserData &userdata;

private:
  void rtti() {}
};

template <class UserData, class Archive>
decltype(auto) extractAdapter(Archive &archive) {
  return static_cast<const UserDataAdapter<UserData, const Archive> &>(archive);
}
template <class UserData, class Archive>
UserData &getUserData(Archive &archive) {
  return extractAdapter<UserData>(archive).userdata;
}
