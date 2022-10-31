#version 330

uniform sampler2D texmap;
uniform sampler2D texmap1;
uniform sampler2D texmap2;
uniform sampler2D texmap3;
uniform sampler2D texmap4;
uniform sampler2D texmap5;
uniform sampler2D texmap6;
uniform sampler2D texmap7;

uniform int texMode;
uniform int fogActivation;
uniform int directionalLight;


uniform bool spotlight_mode;
uniform bool pointlight_mode;
uniform bool directional_mode;

uniform vec4 coneDir;
uniform float spotCosCutOff;


out vec4 FragColor;
out vec4 colorOut;

struct Materials {
    vec3 direction;
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};
uniform Materials mat;

in vec4 pos;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
	vec2 tex_coord;
	//Directions for Point Lights
	vec3 pl_dir[6];
	// Direction for Directional Lights
	vec3 dl_dir;
	//Directions for Spot Lights
	vec3 sl_dir[2];


} DataIn;


vec4 applyFog( in vec3 rgb, in float distance) {

	float fogAmount = exp( -distance*0.05 );
	clamp(fogAmount, 0, 1.0);
	vec3 fogColor = vec3(0.5, 0.6, 0.7);
	vec3 final_color = mix(fogColor, rgb, fogAmount);
	return vec4(final_color, 1.0f);
}

void main() {
	vec4 texel, texel1, texel2, texel3, texel4, texel5, texel6, texel7; 

	vec4 spec = vec4(0.0);
	float intensity = 0.0f;
	float intSpec = 0.0f;

	float att = 0.0;
	float spotExp = 80.0;

	vec3 n = normalize(DataIn.normal);
	
	vec3 e = normalize(DataIn.eye); 
	vec3 sd = normalize(vec3(-coneDir));

	//directionalLight
	//vec3 l = normalize(DataIn.lightDir);

	if(spotlight_mode == true)  {  //Scene iluminated by a spotlight
		for(int i = 0; i < 2; i++){
			vec3 l = normalize(DataIn.sl_dir[i]);
			float spotCos = dot(l, sd);
			if(spotCos > spotCosCutOff)  {	//inside cone
				//att = pow(spotCos, spotExp);
				intensity = max(dot(n,l), 0.0);
				if (intensity > 0.0) {
					vec3 h = normalize(l + e);
					intSpec = max(dot(h,n), 0.0);
					spec = mat.specular * pow(intSpec, mat.shininess);
				}
			}
		}
	}

	if(pointlight_mode == true){//Scene iluminated by a pointlight
		for(int i = 0; i < 6; i++){
			//Temporário
			vec3 l = normalize(DataIn.lightDir);
			//vec3 l = normalize(DataIn.pl_dir[i]);
			intensity = max(dot(n,l), 0.0);
				if (intensity > 0.0) {
					vec3 h = normalize(l + e);
					intSpec = max(dot(h,n), 0.0);
					spec = mat.specular * pow(intSpec, mat.shininess);
				}
		}
	} 

	if(directional_mode == true) { //Scene iluminated by a directionalLight
		//Temporário
		vec3 l = normalize(DataIn.lightDir);
		//vec3 l = normalize(DataIn.dl_dir);
		intensity = max(dot(n,l), 0.0);
			if (intensity > 0.0) {
				vec3 h = normalize(l + e);
				intSpec = max(dot(h,n), 0.0);
				spec = mat.specular * pow(intSpec, mat.shininess);
			}
	}
	


	if(texMode == 0) { // modulate diffuse color with texel color //0 + 1
	//road+grass
		texel = texture(texmap, DataIn.tex_coord);  //texel from grass        		
		texel1 = texture(texmap1, DataIn.tex_coord);  //texel from road  

		colorOut = max(intensity*mix(texel, texel1, texel1.a) + spec, 0.07* mix(texel, texel1, texel1.a));
    }
    else if (texMode == 2) { // diffuse color is replaced by texel color, with specular area or ambient (0.1*texel)
    
        texel2 = texture(texmap2, DataIn.tex_coord); //texel from cheerio 
        colorOut = max(intensity*texel2 + spec, 0.07*texel2);
        //if(texel.a < 0.5)
            //discard;
          
    }
    else if (texMode == 3) {
        texel3 = texture(texmap3, DataIn.tex_coord);//texel from orange
        //TRANSPARENCY
        //if(texel3.a < 0.5)
            //discard;
        colorOut = max(intensity*texel3 + spec, 0.07*texel3);
        
          
    }
	else if (texMode == 4) {
        texel4 = texture(texmap4, DataIn.tex_coord);//texel from tree
        //TRANSPARENCY
		if(texel4.a < 0.5)
			discard;
        colorOut = max(intensity*texel4 + spec, 0.07*texel4); 
    }
	else if (texMode == 6) {
        texel6 = texture(texmap6, DataIn.tex_coord);//texel from butter
        colorOut = max(intensity*texel6 + spec, 0.07*texel6); 
    }
	else if (texMode == 7) {
        texel7 = texture(texmap7, DataIn.tex_coord);//texel from butter
        colorOut = max(intensity*texel7 + spec, 0.07*texel7); 
    }
    else {
        colorOut = max(intensity * mat.diffuse + spec, mat.ambient);
    }
 

	if (fogActivation == 1) {
		float dist = length(pos);
		colorOut = applyFog(vec3(colorOut), dist);
	}

}