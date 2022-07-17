varying vec4 myPos;
varying vec4 myNorm;

void main() 
{
	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0;

	vec4 v = vec4(gl_Vertex);
	myPos = gl_ModelViewProjectionMatrix * v;

	vec4 n = vec4(gl_Normal.x, gl_Normal.y, gl_Normal.z, 0);
	myNorm = gl_ModelViewMatrixInverseTranspose * n;

	gl_Position = ftransform();
}
