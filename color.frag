varying vec4 myNorm;

void main()
{
	
	vec3 normN = vec3(myNorm.x, myNorm.y, myNorm.z);
	normN = normalize(normN);

	float intensity;
	
	vec3 lDir = vec3(1.0, 1.0, 1.0);
	lDir = normalize(lDir);
	
	intensity = clamp(dot(lDir, normN), 0, 1);

	gl_FragColor = intensity * gl_Color;
}
