#ifdef GL_ES
precision mediump float;
#endif

// https://thebookofshaders.com
// https://github.com/lettier/3d-game-shaders-for-beginners/blob/master/demonstration/shaders/fragment/chromatic-aberration.frag
//https://lettier.github.io/3d-game-shaders-for-beginners/chromatic-aberration.html
/*
	Shader caShader = LoadShader(0, "resources/shaders/glsl100/chromatic_aberration.fs");

    float resolution[2] = { (float) screenWidth, (float) screenHeight };
    int resolutionLocation = GetShaderLocation(caShader, "u_resolution");
    SetShaderValue(caShader, resolutionLocation, &resolution, SHADER_UNIFORM_VEC2);

    float textureResolution[2] = { (float) 200, (float) 200 };
    int textureResolutionLocation = GetShaderLocation(caShader, "t_resolution");
    SetShaderValue(caShader, textureResolutionLocation, &textureResolution, SHADER_UNIFORM_VEC2);

    float textureLocation = GetShaderLocation(caShader, "tex0");
*/
uniform vec2 u_resolution;  // Canvas size (width,height)
uniform vec2 u_mouse;       // mouse position in screen pixels
uniform float u_time;       // Time in seconds since load

uniform vec3 iResolution;   // viewport resolution (in pixels)
uniform vec4 iMouse;        // mouse pixel coords. xy: current, zw: click
uniform float iTime;        // shader playback time (in seconds)
uniform sampler2D tex0;

void main() {
	// gl_FragCoord // current loc
	gl_FragColor = vec4(1.0,0.0,1.0,1.0);
}