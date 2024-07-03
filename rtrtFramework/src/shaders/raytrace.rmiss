/*********************************************************************
 * file:   raytrace.rmiss
 * author: lawrence.winters (lawrence.winters@digipen.edu)
 * date:   July 2, 2024
 * Copyright © 2024 DigiPen (USA) Corporation.
 *
 * brief: Shader for rays that don't hit light
 *********************************************************************/

#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "shared_structs.h"

layout(location=0) rayPayloadInEXT RayPayload payload;

void main()
{
    // @@ Raycasting: Just set payload.hit = false (DONE)
    payload.hit = false;
}
