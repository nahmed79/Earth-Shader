varying vec4 myPos;
varying vec4 myNorm;
uniform vec3 lPos;

void main()
{
	//gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);

	//vec3 lPos = vec3(5, 3, -10);
	vec3 vPos = vec3(myPos.x, myPos.y, myPos.z);

	vec3 normN = vec3(myNorm.x, myNorm.y, myNorm.z);
	normN = normalize(normN);

	vec3 vDir = vec3(0, 0, -1);

	vec3 lDir = normalize(vPos - lPos);

	vec3 Reflection = lDir - 2.0 * dot(lDir, normN) * normN;
	Reflection = normalize(Reflection);

	float intensity = clamp(dot(lDir, normN), 0, 1);

	float keTerm = pow(clamp(dot(Reflection, normalize(vDir)), 0.0, 1.0), 50.0);
	vec4 ksTerm = vec4(0.5, 0.5, 0.5, 0) * keTerm;

	//normN = normN * intensity;

	gl_FragColor = gl_Color * intensity;
	gl_FragColor = gl_Color * intensity + ksTerm;
	//gl_FragColor = vec4(intensity, intensity, intensity, 1.0);
	//gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
