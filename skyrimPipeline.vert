
varying vec3 View;     //  Eye position
varying vec3 Light;    //  Light direction
varying vec3 Half;
varying vec3 WorldSpaceView;
varying vec3 WorldSpaceNormal;
varying vec3 WorldSpaceLight;
varying vec3 WorldSpaceHalf;

attribute vec3 Tangent;
attribute vec3 Bitangent;
uniform float texFlags[9];


void main()
{
	//
	//  Lighting values needed by fragment shader
	//
	//  Vertex location in modelview coordinates
	
	vec3 T = gl_NormalMatrix*Tangent;
	vec3 B = gl_NormalMatrix*Bitangent;
	vec3 N = gl_NormalMatrix*gl_Normal;
	vec3 P = vec3(gl_ModelViewMatrix * gl_Vertex);
	WorldSpaceView = normalize(-P);
	WorldSpaceNormal = normalize(N);
	//  Light direction
	vec3 lightDir = vec3(gl_LightSource[0].position.xyz - P);
	WorldSpaceLight = normalize(lightDir);
	vec3 temp;
	temp.x = dot(lightDir, T);
	temp.y = dot(lightDir, B);
	temp.z = dot(lightDir, N);
	Light = normalize(temp);
	
	//  Eye position
	temp.x = dot(P, T);
	temp.y = dot(P, B);
	temp.z = dot(P, N);
	View  = normalize(temp);
	
	// Halfway vector
	P = normalize(P);
	vec3 halfVector = normalize(P + lightDir);
	WorldSpaceHalf = halfVector;
	temp.x = dot(halfVector, T);
	temp.y = dot(halfVector, B);
	temp.z = dot(halfVector, N);
	Half = temp;
	
	gl_FrontColor = gl_Color;
	//  Texture coordinate for fragment shader
	gl_TexCoord[0] = gl_MultiTexCoord0;
	//  Set vertex position
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
