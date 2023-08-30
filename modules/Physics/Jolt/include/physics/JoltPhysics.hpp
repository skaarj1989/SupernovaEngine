#pragma once

class DebugRenderer;

class JoltPhysics {
public:
  JoltPhysics() = delete;
  ~JoltPhysics() = delete;

  static void setup();
  static void cleanup();

  static DebugRenderer *debugRenderer;
};
