// Copyright (c) 2015, Tamas Csala

#ifndef ENGINE_CDLOD_GRID_MESH_H_
#define ENGINE_CDLOD_GRID_MESH_H_

#include <map>
#include <limits>
#include "../../oglwrap_all.h"

namespace engine {

// A two-dimensional vector of GLshort values
struct svec2 {
  GLshort x, y;
  svec2() :x(0), y(0) {}
  svec2(GLshort a, GLshort b) : x(a), y(b) {}
  svec2 operator+(const svec2 rhs) { return svec2(x + rhs.x, y + rhs.y); }
  friend svec2 operator*(GLshort lhs, const svec2 rhs) {
    return svec2(lhs * rhs.x, lhs * rhs.y);
  }
};

// Renders a regular grid mesh, that is of (dimension+1) x (dimension+1) in size
// so a GridMesh(16) will go from (-8, -8) to (8, 8). It is designed to render
// a lots of this at the same time, with instanced rendering.
//
// For performance reasons, GridMesh's maximum size is 255*255 (so that it can
// use unsigned shorts instead of ints or floats), but for CDLOD, you need
// pow2 sizes, so there 128*128 is the max
class GridMesh {
  gl::VertexArray vao_;
  gl::IndexBuffer aIndices_;
  gl::ArrayBuffer aPositions_, aRenderData_, aTextureIds_, aTextureInfo_;
  int index_count_, dimension_;
  std::vector<glm::vec4> render_data_; // xy: offset, z: scale, w: level
  std::vector<uint64_t> texture_ids_;
  std::vector<glm::vec3> texture_info_;

  GLushort indexOf(int x, int y);
  GLushort kPrimitiveRestart = std::numeric_limits<GLushort>::max();

 public:
  GridMesh(GLubyte dimension);
  void setupPositions(gl::VertexAttrib attrib);
  void setupRenderData(gl::VertexAttrib attrib);
  void setupTextureIds(gl::VertexAttrib attrib);
  void setupTextureInfo(gl::VertexAttrib attrib);

  void addToRenderList(const glm::vec4& render_data, uint64_t texture,
                       const glm::vec3& texture_info);
  void clearRenderList();

  // render with vertex attrib divisor
  void render();

  int dimension() const {return dimension_;}
  size_t node_count() const { return render_data_.size(); }
};

} // namespace engine

#endif
