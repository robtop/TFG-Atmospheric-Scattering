#version 430

//#define N_STEPS 20.0f

// CONTANTS
uniform float H_R;
uniform float H_M;
uniform float WORLD_RADIUS;
uniform vec3 C_EARTH;
uniform float ATM_TOP_HEIGHT;
uniform float ATM_RADIUS;
uniform float ATM_RADIUS_2;
uniform float M_PI;
uniform float _3_16PI;
uniform float _3_8PI;
uniform float G;
uniform float G2;
uniform float P0;

// PSEUDO-CONTANTS
uniform vec3 lightDir;
uniform float lightSun;

uniform vec3 betaER;
uniform vec3 betaEM;
uniform vec3 betaSR;
uniform vec3 betaSM;

// VARIABLES
uniform sampler2D texture_diffuse;
uniform sampler2D densityRayleigh;
uniform sampler2D densityMie;
uniform mat4 projection_matrix;

uniform vec3 cam;

in vec4 vs_fs_color;
in vec3 obj;

layout (location = 0) out vec4 color;

bool intersection(vec3 p1, vec3 p2, inout vec3 t1, inout vec3 t2, vec3 cEarth, float atmRadius_2) {
	vec3 rayD = normalize(p2 - p1);
	vec3 oc = p1 - cEarth;

	float b = 2.0f * dot(rayD, oc);
	float c = dot(oc, oc) - atmRadius_2;
	float disc = b*b - 4.0f*c;

	t1 = p1;
	t2 = p2;

	if(disc < 0.0f) return false;
	
	float d0 = (-b - sqrt(disc)) / 2.0f;
	float d1 = (-b + sqrt(disc)) / 2.0f;

	if (d0 > d1) {
		float aux = d0;
		d0 = d1;
		d1 = aux;
	}
	
	if( d1 < 0.0f ) return false;

	t1 = max(d0, 0.0f) * rayD + p1;
	t2 = (d1 > distance(p1, p2)) ? p2 : d1 * rayD + p1;

	return true;
}

void main(void)
{
	vec4 transfVec = vec4(256.0f * 256.0f * 256.0f, 256.0f * 256.0f, 256.0f, 1.0f) * 255.0f;
	float N_STEPS = 20.0f;

	float _3_16PI = 3.0f / (16.0f * M_PI);
	float _3_8PI = 3.0f / (8.0f * M_PI);
	vec3 cEarth = C_EARTH;

	// Calculo view
	mat4 projTrans = transpose(projection_matrix);
	vec3 view = normalize(projTrans[2].xyz);
	vec3 normLightDir = normalize(lightDir);
	
	vec3 computedCam;
	vec3 obj2;

	bool q = intersection(cam, obj, computedCam, obj2, cEarth, ATM_RADIUS_2);
	vec3 delta_P = (obj2 - computedCam) / N_STEPS;

	float diferential_s = length(delta_P);
	
	vec2 density_PC = vec2(0.0f, 0.0f);
	vec3 rayLeigh_In = vec3(0.0f, 0.0f, 0.0f);
	vec3 mie_In = vec3(0.0f, 0.0f, 0.0f);

	if (q) for (float s = 0.5f; s < N_STEPS; s += 1.0f){

		vec3 point = computedCam + delta_P * s;

		float h = max(length(point - cEarth) - WORLD_RADIUS, 0.0f);

		vec3 normalEarth = normalize(point - cEarth);

		vec2 partDenRM = P0 * exp(-h / vec2(H_R, H_M));

		float cosPhi = dot(normalEarth, -normLightDir);
			
		vec2 densityCoord = vec2((h / ATM_TOP_HEIGHT), (cosPhi + 1.0f) / 2.0f);
		vec4 vDAP_Ray = texture(densityRayleigh, densityCoord);
		vec4 vDAP_Mie = texture(densityMie, densityCoord);
		float fDAP_Ray = dot(transfVec, vDAP_Ray.abgr);
		float fDAP_Mie = dot(transfVec, vDAP_Mie.abgr);
			
		vec2 density_AP = vec2(fDAP_Ray, fDAP_Mie);
			

		density_PC += partDenRM * diferential_s;

		vec2 density_APC = density_AP + density_PC;

		vec3 extinction_RM = exp(-( density_APC.x * betaER + density_APC.y * betaEM ));

		vec3 difLR = partDenRM.x * betaSR * extinction_RM * diferential_s;
		vec3 difLM = partDenRM.y * betaSM * extinction_RM * diferential_s;

		// Calcular visibilidad de P
		float visi = cosPhi < -0.24f ? 0.0f : 1.0f;

		rayLeigh_In += difLR * visi;
		mie_In += difLM * visi;
	}

	float cosTheta = dot(normalize(-view), normLightDir);

	float cos2ThetaP1 = 1.0f + (cosTheta * cosTheta);

	// Apply Phase Functions 
	// RAY
	float phase_rayLeigh = _3_16PI * cos2ThetaP1;
	// MIE
	float phase_mieScattering = _3_8PI * (((1.0f - G2) * cos2ThetaP1) / ((2.0f + G2) * pow(1.0f + G2 - 2.0f * G*cosTheta, 1.5f)));

	//ApplyPhaseFunctions(rayLeigh, mieScattering, cosTheta);

	//vec3 inScattering = (rayLeigh + mieScattering) * lightSun;
	vec3 inScattering = (rayLeigh_In * phase_rayLeigh + mie_In * phase_mieScattering) * lightSun;
	vec3 extintion = exp(-(density_PC.x * betaER + density_PC.y * betaEM));

	//color = vs_fs_color;
	vec3 L0_Ext = texture(texture_diffuse, vs_fs_color.st).rgb * extintion;
	color = vec4(1.0f - exp(-1.0f * (L0_Ext + inScattering) ), 1.0f);
	//color = q? vec4(exp(-diferential_s/10000), exp(-diferential_s/1000), exp(-diferential_s/10), 1) : vec4(0,0,1,1);
	
	//color = vec4(L0_Ext, 1);
	//color = vec4(texture(densityRayleigh, vs_fs_color.st).rgb, 1);
	//color = texture(texture_diffuse, vs_fs_color.xy);
	//color = vec4(inScattering, 1);
	//color = vec4(phase_mieScattering, phase_mieScattering, phase_mieScattering, 1);
	//color = vec4(inScattering, 1);
	//color = vec4(extintion, 1);
	//color = q? vec4(1,1,1,1) : vec4(0,0,0,1) ;
	//color = vec4(-f,f,exp(f),1);
	//color = diferential_s<0? vec4(1,1,1,1) : vec4(0,0,0,1) ;
	//*/


}
