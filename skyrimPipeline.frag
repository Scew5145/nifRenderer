varying vec3 View;		//  Eye position
varying vec3 Light;		//  Light direction
varying vec3 Half;	   	// Halfway vector
varying vec3 WorldSpaceView;
varying vec3 WorldSpaceNormal;
varying vec3 WorldSpaceLight;
varying vec3 WorldSpaceHalf;


// Color properties
uniform vec4 emissiveColor;
uniform float emissiveMultiple;
uniform vec4 specularColor;
uniform float specularStrength;
uniform float glossiness;
const float softLightCoefficient = 1.0;

// All of the following textures are named based on their most common suffix for each filetype.
uniform sampler2D tex; // Diffuse: 0
uniform sampler2D tex_n; // Normal map: 1
uniform sampler2D tex_g; // Glow map: 2
//uniform sampler2D tex_p; // Parralax map: 3 - not implemented by skyrim
uniform sampler2D tex_e; // Cube map: 4
uniform sampler2D tex_m; // Enviroment/Cube mask: 5
uniform sampler2D tex_s; // Subsurface map: 6
uniform float texFlags[9];

const vec2 cubeUVMultiple = vec2(0.25, 0.333);


void main()
{
	//Normal map calculations
	float diffuse;
	float softDiff;
	float specular = 0.0;
	// Doing some math with the texture flag to make the mappedNormal value (0.5,0.5,1) if there's no texture
	vec4 mappedNormal = min(texture2D(tex_n, gl_TexCoord[0].xy),texFlags[1]); 
	mappedNormal.z = max(texture2D(tex_n, gl_TexCoord[0].xy).z,1.0-texFlags[1]);
	mappedNormal.xy += 0.5*(1.0-texFlags[1]);
	vec3 N = normalize(((mappedNormal.xyz)*2.0)-1.0);// 0:1 range to -1:1 range mapping
	diffuse = max(dot(Light, N), 0.0);
	specular = pow(max(dot(N, Half),0.0), glossiness);
	specular *= mappedNormal.a;
	if(texFlags[8] == 1.0){
		
		float temp = N.z; // Because someone is a genius, y and z are flipped on model-space normal maps. I hate everything. This took me 3 hours to fix.
		N.z = N.y;
		N.y = temp;
		N = gl_NormalMatrix*N;
		//N.x = -1.0*N.x;
		// In this situation, we'll be doing soft wrapped lighting with a specific texture in glow, so diffuse changes a bit.
		// Basically, it changes from dot(N,H) to (dot(N,H)+W)/(1+W)^2. This makes it so that the light 'wraps' around the object slower.
		diffuse = max(dot(WorldSpaceLight,N),0.0);
		softDiff = (dot(WorldSpaceLight,N)+softLightCoefficient)/((softLightCoefficient+1.0)*(softLightCoefficient+1.0));
		softDiff = max(diffuse,0.0);
		specular = pow(max(dot(N, WorldSpaceHalf),0.0), 50);
		
	}
	
	
	//Cube map stuff
	vec3 reflectDir = normalize(reflect(WorldSpaceView,WorldSpaceNormal));
	// Need to transform this 3-dimension vector into a two-dimensional texture coordinate.
	// This code sucks and I should update it if I have a chance. Probably using radial coordinates or some other magic
	float maxDir;
	vec2 cubeSampler;
	vec4 cubeMapTex;
	vec2 cubeOffset;
	

	if( (abs(reflectDir.x) >= abs(reflectDir.y)) && (abs(reflectDir.x) >= abs(reflectDir.z)) ){
		// x (world coordinates) greatest
		cubeOffset = vec2((max(0.0,sign(reflectDir.x)))*2.0+0.5,1.5);
		cubeSampler.x = cubeUVMultiple.x*(cubeOffset.x + (reflectDir.y)/(reflectDir.x*2.0));
		cubeSampler.y = cubeUVMultiple.y*(cubeOffset.y - sign(reflectDir.x)*(reflectDir.z)/(reflectDir.x*2.0) );
		maxDir = 0.0;
	}else if( (abs(reflectDir.y) >= abs(reflectDir.x)) && (abs(reflectDir.y) >= abs(reflectDir.z)) ){
		// z (world coordinates) greatest
		cubeOffset = vec2((max(0.0,sign(reflectDir.y)))*2.0+1.5,1.5);
		cubeSampler.x = cubeUVMultiple.x*(cubeOffset.x - reflectDir.x/(reflectDir.y*2.0));
		cubeSampler.y = cubeUVMultiple.y*(cubeOffset.y - reflectDir.z/(reflectDir.y*2.0));
		maxDir = 1.0;
	}else if( (abs(reflectDir.z) >= abs(reflectDir.y)) && (abs(reflectDir.z) >= abs(reflectDir.x)) ){
		// y (world coordinates) greatest
		cubeOffset = vec2(1.5,(max(0.0,-sign(reflectDir.z))*2.0+0.5));
		cubeSampler.x = cubeUVMultiple.x*(cubeOffset.x + sign(reflectDir.z)*reflectDir.x/(reflectDir.z*2.0)) ;
		cubeSampler.y = cubeUVMultiple.y*(cubeOffset.y - reflectDir.y/(reflectDir.z*2.0));
		maxDir = 2.0;
	}else{
		maxDir = -1.0;
	}
	cubeMapTex = min(texture2D(tex_e, cubeSampler),texFlags[4]);
	
	// Grab the rest of the textures
	vec4 diffuseTex = max(texture2D(tex,gl_TexCoord[0].xy),1.0-texFlags[0]); // diffuse texture value should be vec4(1.0) if there's no texture
	
	//Glow stuff
	vec4 glowTex = min(texture2D(tex_g,gl_TexCoord[0].xy),texFlags[2]); // glow texture value should be 0.0 if there's no texture
	
	vec4 ambientTotal = gl_LightSource[0].ambient; // TODO: add glow color+intensity here
	vec4 diffuseTotal = vec4(diffuse);
	vec4 specularTotal = vec4(specular) * specularColor * vec4(specularStrength) * max(texture2D(tex_m, gl_TexCoord[0].xy),1.0-texFlags[5]);

	vec4 outsideLight = (ambientTotal + diffuseTotal + specularTotal);
	
	// In the case of a subsurface map existing, the glow map is actually a soft lighting map instead. Do that.
	vec4 glowTotal = emissiveColor*glowTex*emissiveMultiple*(1.0-texFlags[8]) + texFlags[8]*glowTex*softDiff;
	
	// This next part looks scary, but we're just mixing the soft-light texture with the diffuse texture, based on how bright and translucent the spot is.
	//vec4 subsurfaceMixer = mix(diffuseTex, glowTex, min(softDiff*texture2D(tex_s,gl_TexCoord[0].xy),1.0-texFlags[7]));
	gl_FragColor = glowTotal + diffuseTex*gl_Color*outsideLight + outsideLight*max(texture2D(tex_m, gl_TexCoord[0].xy),1.0-texFlags[5])*cubeMapTex;
	//gl_FragColor = vec4(max(dot(N,WorldSpaceLight),0.0));
	//gl_FragColor = 1.0-texture2D(tex_s, gl_TexCoord[0].xy);//vec4(abs(N),1.0);


}
