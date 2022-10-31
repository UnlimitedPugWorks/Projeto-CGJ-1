#version 330

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

//Temporário
uniform vec4 l_pos;

// 6 Point Lights
uniform vec4 pl_pos[6];

// 1 Directional Light
uniform vec4 dl_dir;

// 2 Spot Lights
uniform vec4 sl_pos[2];

in vec4 position;
in vec4 normal;    //por causa do gerador de geometria
in vec4 texCoord;

out vec4 pos;

out Data {
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


} DataOut;


void main () {

	pos = m_viewModel * position;

	DataOut.normal = normalize(m_normal * normal.xyz);
	
	//Point Lights
	for(int i = 0; i < 6 ; i++){
		DataOut.pl_dir[i] = vec3(pl_pos[i] - pos);
	}

	//Directional Lights
	DataOut.dl_dir = vec3(-dl_dir);
	
	//Spot Lights
	for(int i = 0; i < 2; i++){
		DataOut.sl_dir[i] = vec3(sl_pos[i] - pos);
	}

	//Temporário
	DataOut.lightDir = vec3(l_pos - pos);

	DataOut.eye = vec3(-pos);
	DataOut.tex_coord = texCoord.st;
	
	gl_Position = m_pvm * position;	
}