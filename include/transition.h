#pragma once

GLuint gShaderProgramObject_trans;
GLuint gVertexShaderObject_trans;
GLuint gFragmentShaderObject_trans;


GLuint quadVAO_trans;
GLuint quadVBO_trans;

GLuint alpha_uniform;

int initialize_transition(void)
{
	// variables for error checking 
	GLint iShaderCompileStatus = 0;
	GLint iInfoLogLength = 0;
	GLchar* szInfoLog = NULL;
	GLint iProgramLinkStatus = 0;


	// final source ..........................................................
#pragma region VERETX_SHADER
	gVertexShaderObject_trans = glCreateShader(GL_VERTEX_SHADER);
	const GLchar* vertexShaderSourceCode =
		"#version 460 core" \
		"\n" \
		"in vec3 vPosition;" \

		"void main(void)" \
		"{" \
			"gl_Position	= vec4(vPosition,1.0);" \
		"}";

	glShaderSource(gVertexShaderObject_trans, 1, (GLchar**)&vertexShaderSourceCode, NULL);
	glCompileShader(gVertexShaderObject_trans);

	iShaderCompileStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	glGetShaderiv(gVertexShaderObject_trans, GL_COMPILE_STATUS, &iShaderCompileStatus);

	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject_trans, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject_trans, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "\n--------------------------------------------------- \nCompile time log for Vertex Shader : \n %s", szInfoLog);
				fflush(gpFile);
				free(szInfoLog);
				return(-6);
			}
		}
	}
#pragma endregion

#pragma region FRAGMENT_SHADER
	gFragmentShaderObject_trans = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* fragmentShaderSourceCode =
		"#version 460 core" \
		"\n" \

		"out vec4 FragColor;" \
		"uniform float alpha_val;" \

		"void main(void)" \
		"{" \
			"FragColor		= vec4(0.0f,0.0f,0.0f, alpha_val);" \
		"}";

	glShaderSource(gFragmentShaderObject_trans, 1, (GLchar**)&fragmentShaderSourceCode, NULL);
	glCompileShader(gFragmentShaderObject_trans);

	// error checking for fragment shader
	iShaderCompileStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	glGetShaderiv(gFragmentShaderObject_trans, GL_COMPILE_STATUS, &iShaderCompileStatus);

	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject_trans, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObject_trans, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "\n--------------------------------------------------- \nCompile time log for Fragment Shader : \n %s", szInfoLog);
				fflush(gpFile);
				free(szInfoLog);
				return(-7);
			}
		}
	}
	// fragment shader part end
#pragma endregion

#pragma region SHADER_PROGRAM
	gShaderProgramObject_trans = glCreateProgram();

	glAttachShader(gShaderProgramObject_trans, gVertexShaderObject_trans);
	glAttachShader(gShaderProgramObject_trans, gFragmentShaderObject_trans);

	// prelinking, binding to vertex attribute for position and normals
	glBindAttribLocation(gShaderProgramObject_trans, AMC_ATTRIBUTE_POSITION, "vPosition");
	//glBindAttribLocation(gShaderProgramObject_trans, AMC_ATTRIBUTE_NORMAL, "vNormal");
	//glBindAttribLocation(gShaderProgramObject_trans, AMC_ATTRIBUTE_TEXCOORD0, "vTexcoord");

	glLinkProgram(gShaderProgramObject_trans);

	iProgramLinkStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	glGetProgramiv(gShaderProgramObject_trans, GL_LINK_STATUS, &iProgramLinkStatus);

	if (iProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObject_trans, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject_trans, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "\n--------------------------------------------------- \nLink time log for Shader Program : \n %s", szInfoLog);
				fflush(gpFile);
				free(szInfoLog);
				return(-8);
			}
		}
	}


	// postlinking, getting uniform location
	alpha_uniform = glGetUniformLocation(gShaderProgramObject_trans, "alpha_val");
#pragma endregion

#pragma region QUAD

	float quadVertices[] = 
	{
		// positions       
		-1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f
	};

	// setup plane VAO
	glGenVertexArrays(1, &quadVAO_trans);
	glBindVertexArray(quadVAO_trans);
	
	glGenBuffers(1, &quadVBO_trans);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO_trans);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE,0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindVertexArray(0);
	
#pragma endregion


	return(0);
}

void display_transition(float val)
{
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_NOTEQUAL, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(gShaderProgramObject_trans);

	//if(val < 1.01f && val > -0.01f)
		glUniform1f(alpha_uniform, val);
	//else
		//glUniform1f(alpha_uniform, 1.0f);


	glBindVertexArray(quadVAO_trans);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	glUseProgram(0);

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
}

