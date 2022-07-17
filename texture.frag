uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;

uniform bool showClouds;

uniform vec3 lPos;

uniform float cloudM;

varying vec4 myPos;

varying vec4 myNorm;

uniform bool bumpN;

uniform bool highlight;

uniform bool diffuseLight;

uniform bool textureEarth;

void main()
{
	vec2 ST = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t);
	
	ST = vec2(ST.x + cloudM, ST.y);

	vec4 Earth = texture2D(tex0, gl_TexCoord[0].st);
	vec4 Normal = texture2D(tex1,gl_TexCoord[0].st);
	//vec4 Cloud = texture2D(tex2, gl_TexCoord[0].st);
	vec4 Cloud = texture2D(tex2, ST);
	vec4 Water = texture2D(tex3, gl_TexCoord[0].st);

	vec3 vPos = vec3(myPos.x, myPos.y, myPos.z);

	vec3 norm = vec3(Normal.x, Normal.y, Normal.z);
	norm = normalize(norm);

	vec3 vDir = vec3(0, 0, 1);

	vec3 lDir = normalize(vPos - lPos);

	vec3 normN = vec3(myNorm.x, myNorm.y, myNorm.z);
	normN = normalize(normN);

	float intensity;
	
	intensity = clamp(dot(lDir, normN), 0, 1);

	if(bumpN)
	{
		intensity = clamp(dot(lDir, norm), 0, 1);
	}
	
	vec3 Reflection = lDir - 2.0 * dot(lDir, normN) * normN;
	Reflection = -normalize(Reflection);

	vec4 final = Earth;

	if(!textureEarth)
	{
		final = gl_Color * intensity;
	}

	if(diffuseLight)
	{
		final = 0.1 * Earth + 0.9 * intensity * Earth;
	}

	if(showClouds)
	{
		final = final + Cloud;
	}

	float keTerm = pow(clamp(dot(Reflection, vDir), 0.0, 1.0), 50.0);
	vec4 ksTerm = vec4(0.5, 0.5, 0.5, 0) * keTerm;
	
	//final = vec4(intensity, intensity, intensity, 1);

	if(highlight)
	{
		final = final + ksTerm * Water;
	}

	//final = vec4(normN.x, normN.y, normN.z, 1);

	gl_FragColor = final;
}
