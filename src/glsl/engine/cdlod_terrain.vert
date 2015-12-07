// Copyright (c) 2015, Tamas Csala

#version 330
#extension GL_ARB_bindless_texture : require

#include "engine/bicubic_sampling.glsl"
#include "engine/cube2sphere.glsl"

#export vec4 Terrain_modelPos(vec2 m_pos, vec4 render_data);
#export vec3 Terrain_worldPos(vec3 model_pos);

in uvec2 Terrain_aCurrentGeometryTextureId;
in vec3 Terrain_aCurrentGeometryTexturePosAndSize;
in uvec2 Terrain_aNextGeometryTextureId;
in vec3 Terrain_aNextGeometryTexturePosAndSize;

uniform int Terrain_uMaxLoadLevel;
uniform int Terrain_uTextureDimensionWBorders;
uniform vec3 Terrain_uCamPos;
uniform float Terrain_uSmallestGeometryLodDistance;

uniform int Terrain_uMaxHeight;

const float kMorphEnd = 0.95, kMorphStart = 0.85;

float Terrain_cam_height = length(Terrain_uCamPos) - Terrain_radius();

float Terrain_estimateDistance(vec2 geom_pos) {
  float est_height = clamp(Terrain_cam_height, 0, Terrain_uMaxHeight);
  vec3 est_pos = vec3(geom_pos.x, est_height, geom_pos.y);
  vec3 est_diff = Terrain_uCamPos - Terrain_worldPos(est_pos);
  return length(est_diff);
}

vec2 Terrain_morphVertex(vec2 vertex, float morph) {
  vec2 frac_part = fract(vertex * 0.5) * 2.0;
  return (vertex - frac_part * morph);
}

vec2 Terrain_nodeLocal2Global(vec2 node_coord, vec2 offset, float scale) {
  vec2 pos = offset + scale * node_coord;
  return pos;
}

float Terrain_getHeightInternal(vec2 pos, uvec2 texid, vec3 texPosAndSize) {
  vec2 sample = (pos - texPosAndSize.xy) / texPosAndSize.z;
  sample += 0.5 / Terrain_uTextureDimensionWBorders;
  float normalized_height = textureBicubic(sampler2D(texid), sample).r;
  return normalized_height * Terrain_uMaxHeight;
}

float Terrain_getHeight(vec2 pos, float level, float morph) {
  float height0 =
    Terrain_getHeightInternal(pos, Terrain_aCurrentGeometryTextureId,
                              Terrain_aCurrentGeometryTexturePosAndSize);
  if (morph == 0.0 || level <= 0) {
    return height0;
  }

  float height1 =
    Terrain_getHeightInternal(pos, Terrain_aNextGeometryTextureId,
                              Terrain_aNextGeometryTexturePosAndSize);

  return mix(height0, height1, morph);
}

vec4 Terrain_modelPos(vec2 m_pos, vec4 render_data) {
  vec2 offset = render_data.xy;
  float scale = render_data.z;
  float level = render_data.w;
  vec2 pos = Terrain_nodeLocal2Global(m_pos, offset, scale);
  float dist = Terrain_estimateDistance(pos);
  float morph = 0;

  if (level < Terrain_uMaxLoadLevel) {
    float next_level_size =
        2 * scale * Terrain_uSmallestGeometryLodDistance;
    float max_dist = kMorphEnd * next_level_size;
    float start_dist = kMorphStart * next_level_size;
    morph = smoothstep(start_dist, max_dist, dist);

    vec2 morphed_pos = Terrain_morphVertex(m_pos, morph);
    pos = Terrain_nodeLocal2Global(morphed_pos, offset, scale);
    dist = Terrain_estimateDistance(pos);
  }

  float height = Terrain_getHeight(pos, level, morph);
  return vec4(pos.x, height, pos.y, morph);
}

