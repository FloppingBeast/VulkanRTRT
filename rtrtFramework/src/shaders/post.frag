/*********************************************************************
 * file:   post.frag
 * author: lawrence.winters (lawrence.winters@digipen.edu)
 * date:   June 15, 2024
 * Copyright © 2023 DigiPen (USA) Corporation.
 *
 * brief:  Post fragment shader
 *********************************************************************/

#version 450

layout(set=0, binding=0) uniform sampler2D renderedImage;

layout(location = 0) out vec4 fragColor;


void main()
{
    vec2 uv = gl_FragCoord.xy/vec2(1280, 768);
    fragColor = vec4(uv, 0, 1);
}
