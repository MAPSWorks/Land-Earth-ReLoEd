// Copyright (c) 2015, Tamas Csala

#ifndef ENGINE_COLLISION_SPHERIZED_AABB_H_
#define ENGINE_COLLISION_SPHERIZED_AABB_H_

#include "../misc.h"
#include "./bounding_box.h"
#include "./cube2sphere.h"
#include "../settings.h"

namespace engine {

struct Interval {
  double min;
  double max;
};

static inline bool HasIntersection(Interval a, Interval b) {
  return a.min - kEpsilon < b.max && b.min - kEpsilon < a.max;
}

class SpherizedAABB {
  BoundingBox aabb_;

  glm::dvec3 normals_[4];
  Interval extents_[4];
  Interval radial_extent_;

 public:
  SpherizedAABB() = default;

  SpherizedAABB(const BoundingBox& bbox, CubeFace face, double kFaceSize)
      : aabb_ {Cube2Sphere(bbox, face, kFaceSize)} {
    using namespace glm;

    const glm::dvec3& mins = bbox.mins();
    const glm::dvec3& maxes = bbox.maxes();
    double radius = Settings::kSphereRadius;
    radial_extent_ = {radius + mins.y, radius + maxes.y};

    /*
      The input coordinate system is right handed:
          (O)----(x - longite)
          /|
         / |
 (y - rad) |
           |
      (z - latitude)

      But we can use any convenient coordinate system,
      as long as it is right handed too:

          (y)
           |
           |
           |
          (O)-----(x)
          /
         /
       (z)

      The verices:

           (E)-----(A)
           /|      /|
          / |     / |
        (F)-----(B) |
         | (H)---|-(D)
         | /     | /
         |/      |/
        (G)-----(C)
    */

    enum {
      A, B, C, D, E, F, G, H
    };

    glm::dvec3 m_vertices[8];
    m_vertices[A] = {maxes.x, maxes.y, mins.z};
    m_vertices[B] = {maxes.x, maxes.y, maxes.z};
    m_vertices[C] = {maxes.x, mins.y,  maxes.z};
    m_vertices[D] = {maxes.x, mins.y,  mins.z};
    m_vertices[E] = {mins.x,  maxes.y, mins.z};
    m_vertices[F] = {mins.x,  maxes.y, maxes.z};
    m_vertices[G] = {mins.x,  mins.y,  maxes.z};
    m_vertices[H] = {mins.x,  mins.y,  mins.z};

    glm::dvec3 vertices[8];
    for (int i = 0; i < 8; ++i) {
      vertices[i] = Cube2Sphere(m_vertices[i], face, kFaceSize);;
    }

    enum {
      Front = 0, Right = 1, Back = 2, Left = 3
    };

    // normals are towards the inside of the AABB
    normals_[Front] = GetNormal(vertices, G, C, B, C);
    normals_[Right] = GetNormal(vertices, C, D, A, D);
    normals_[Back]  = GetNormal(vertices, D, H, E, H);
    normals_[Left]  = GetNormal(vertices, H, G, F, G);

    extents_[Front] =
      getExtent(normals_[Front], m_vertices[B], m_vertices[A], face, kFaceSize);
    extents_[Right] =
      getExtent(normals_[Right], m_vertices[A], m_vertices[E], face, kFaceSize);
    extents_[Back]  =
      getExtent(normals_[Back],  m_vertices[H], m_vertices[G], face, kFaceSize);
    extents_[Left]  =
      getExtent(normals_[Left],  m_vertices[F], m_vertices[B], face, kFaceSize);
  }

  static glm::dvec3 GetNormal(glm::dvec3 vertices[], int a, int b, int c, int d) {
    glm::dvec3 ba = vertices[a]-vertices[b];
    glm::dvec3 dc = vertices[c]-vertices[d];

    // If one of these are null vectors, we can't use this plane for separation,
    // so let the normal be null vector, and the (0, 0) intervals will intersect.
    if (length(ba) < kEpsilon || length(dc) < kEpsilon) {
      return glm::dvec3();
    }

    return normalize(cross(ba, dc));
  }


  static Interval getExtent(const glm::dvec3& normal,
                            const glm::dvec3& m_space_min,
                            const glm::dvec3& m_space_max,
                            CubeFace face, double kFaceSize) {
    Interval interval;
    glm::dvec3 diff = m_space_max - m_space_min;
    for (int i = 0; i <= 4; ++i) {
      glm::dvec3 current = Cube2Sphere(m_space_min + i/4.0*diff, face, kFaceSize);
      double current_projection = dot(current, normal);
      if (i == 0) {
        interval.min = current_projection;
        interval.max = current_projection;
      } else {
        interval.min = std::min(interval.min, current_projection);
        interval.max = std::max(interval.max, current_projection);
      }
    }

    return interval;
  }

  bool collidesWithSphere(const Sphere& sphere) const {
    for (size_t i = 0; i < 4; ++i) {
      double interval_center = dot(sphere.center(), normals_[i]);
      Interval projection_extent = {interval_center - sphere.radius(),
                                    interval_center + sphere.radius()};

      if (!HasIntersection(extents_[i], projection_extent)) {
        return false;
      }
    }

    double radial_interval_center = length(sphere.center());
    Interval radial_extent = {radial_interval_center - sphere.radius(),
                              radial_interval_center + sphere.radius()};
    if (!HasIntersection(radial_extent_, radial_extent)) {
      return false;
    }

    return aabb_.collidesWithSphere(sphere);
  }

  bool collidesWithFrustum(const Frustum& frustum) const {
    return aabb_.collidesWithFrustum(frustum);
  }
};


}


#endif
