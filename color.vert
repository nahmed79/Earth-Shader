varying vec4 myNorm;

void main() 
{
	vec4 n = vec4(gl_Normal.x, gl_Normal.y, gl_Normal.z, 0);
	myNorm = gl_ModelViewMatrixInverseTranspose * n;

	gl_FrontColor = gl_Color;
	gl_Position = ftransform();
}
