#version 410 core

layout (location = 0) in vec3 vertexPosition_modelSpace;
layout (location = 1) in vec3 vertexNormal_modelSpace;
layout (location = 2) in vec3 vertexColor;

uniform vec4 lightDirection; // rename to lightDirection

uniform mat4 model;
uniform mat3 normalMat;
uniform mat4 mvp;
uniform mat4 lightBiasMVP;
uniform mat4 view;
//uniform mat4 lightViewMat;

out vec4 ShadowCoord;

out vec4 fragmentColor;
//out vec3 vertexPosition_worldSpace;
out vec3 normal_worldSpace;
out vec3 eyeDirection_worldSpace;
out vec3 lightDirection_worldSpace;


void main()
{
        gl_Position = mvp * vec4(vertexPosition_modelSpace, 1.0);
        ShadowCoord = lightBiasMVP * vec4(vertexPosition_modelSpace, 1.0);

        lightDirection_worldSpace = (-lightDirection).xyz;

        //interpolating normals
        normal_worldSpace = (inverse(transpose(model)) * vec4(vertexNormal_modelSpace, 1)).xyz;

        mat4 viewInv = inverse(view);
        vec3 cameraPosition_worldSpace = viewInv[3].xyz/viewInv[3].w;

        //vector that goes from vertex to camera in world space
        eyeDirection_worldSpace = cameraPosition_worldSpace -  (model * vec4(vertexPosition_modelSpace, 1.0)).xyz;

        fragmentColor = vec4(vertexColor, 1.0);

}