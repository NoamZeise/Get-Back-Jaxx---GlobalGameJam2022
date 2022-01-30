#version 450

layout(push_constant) uniform fragconstants
{
    layout(offset = 128) vec4 colour;
    vec4 texOffset;
    uint texID;
    uint useLighting;
} pc;


layout(set = 2, binding = 0) uniform sampler texSamp;
layout(set = 2, binding = 1) uniform texture2D textures[200];

const uint MAX_LIGHTS = 50;
layout(set = 3, binding = 0) readonly buffer PerFrameBuffer {
    vec2 lights[MAX_LIGHTS];
} lighting;

layout(set = 4, binding = 0) uniform UniformBufferObject
{
    float linear;
    float quadratic;
} ubo;

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inVertPos;

layout(location = 0) out vec4 outColour;

void main()
{
    vec2 coord = inTexCoord;
    coord.x *= pc.texOffset.z;
    coord.y *= pc.texOffset.w;
    coord.x += pc.texOffset.x;
    coord.y += pc.texOffset.y;

    vec4 col = texture(sampler2D(textures[pc.texID], texSamp), coord) * pc.colour;

    if(pc.useLighting > 5)
    {
        float attenuation = 0;
        for(int i = 0; i < MAX_LIGHTS; i++)
        {
            if(lighting.lights[i] != vec2(0, 0))
            {
                float distance = length(distance(lighting.lights[i], gl_FragCoord.xy));
                 attenuation += 1.0 / (1.0f + ubo.linear * distance + 
    		                    ubo.quadratic * (distance * distance));  
            }
        }
        //if(attenuation < 0.2)
        //    attenuation = 0.2;

        col *= attenuation;
    }

    if(col.w == 0)
        discard;
    outColour = col;
}
