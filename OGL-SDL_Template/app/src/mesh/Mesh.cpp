
#include "Mesh.h"


//Mesh::Mesh() : _vertexCount(0), _texCoordCount(0), _facesCount(0) {}

void Mesh::initOGLData() {

	ShaderInfo shaderFiles[] = {
		{ GL_VERTEX_SHADER, "../OGL-SDL_Template/app/shaders/scattering.vs.glsl" },
		{ GL_FRAGMENT_SHADER, "../OGL-SDL_Template/app/shaders/scattering.fs.glsl" },
		//{ GL_FRAGMENT_SHADER, "../OGL-SDL_Template/app/shaders/gouraud.frag" },
		{ GL_NONE, NULL }
	};

	_shader.load(shaderFiles);
	_shader.use();

	// Pedimos un buffer para el element buffer object
	_ebo = new GLuint[_eboDataCount];
	_tso = new GLuint[_eboDataCount];
	glGenBuffers(_eboDataCount, _ebo);
	glGenTextures(_eboDataCount, _tso);

	for (GLuint i = 0; i < _eboDataCount; i++) {
		PerDraw *_eboData = &_elementBufferObjectData[i];
		_eboData->indicesSize = _eboData->indicesCount * sizeof(GLuint);
		
		// Le hacemos hueco diciendole el tipo de buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo[i]);
		// Lo rellenamos con los indices de los cubos
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _eboData->indicesSize, _eboData->indices, GL_STATIC_DRAW);

		_texture = IMG_Load(_eboData->mat->textureDiffuse.c_str());

		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, _tso[i]);

		int mode = GL_RGB;
		if (_texture->format->BytesPerPixel == 4) mode = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, mode, _texture->w, _texture->h, 0, mode, GL_UNSIGNED_BYTE, _texture->pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	// Pedimos un array de vertices
	glGenVertexArrays(1, _vao);
	// Le hacemos hueco
	glBindVertexArray(_vao[0]);

	// Pedimos un buffer para el vertex buffer object
	glGenBuffers(1, _vbo);
	// Le hacemos hueco
	glBindBuffer(GL_ARRAY_BUFFER, _vbo[0]);
	_vboDataSize = _vboDataCount * sizeof(PerVertex);
	// Le decimos que el hueco tiene que ser de tama�o "tama�o de cube positions"+"tama�o de cube colors"
	glBufferData(GL_ARRAY_BUFFER, _vboDataSize, _vertexBufferObjectData, GL_STATIC_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, _vboDataSize, _vertexBufferObjectData);
	//glBufferSubData(GL_ARRAY_BUFFER, _sizeOfVertex, _sizeOfTexCoords, _texCoord);
	//GLint vertexInfoAttrib = glGetAttribLocation(, "vertexInfo");

	_shader.use();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PerVertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PerVertex), (const GLvoid*)sizeof(Vertex));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, sizeof(PerVertex), (const GLvoid*)(sizeof(Vertex)+sizeof(Normal)));
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 0, (const GLvoid*)(_sizeOfVertex));


	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	Log::trace("InitOGL Data check...");
	CheckErr();
	Log::trace("CHECKED!");


	_uniformProjectionId = glGetUniformLocation(_shader.id(), "projection_matrix");
	//_texture_id = 0;

	// You should probably use CSurface::OnLoad ... ;)
	//-- and make sure the Surface pointer is good!
	//stringstream ss;
	//ss << "../OGL-SDL_Template/app/resources/ObjTex/QuantumArid_Diffuse_" << _name[10] << ".jpg";
	//_texture = IMG_Load(ss.str().c_str());
	/*
	glActiveTexture(GL_TEXTURE0 + 0);
	glGenTextures(1, &_texture_id);
	glBindTexture(GL_TEXTURE_2D, _texture_id);
	glBindSampler(0, GL_LINEAR);

	int mode = GL_RGB;
	if (_texture->format->BytesPerPixel == 4) mode = GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, mode, _texture->w, _texture->h, 0, mode, GL_UNSIGNED_BYTE, _texture->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/
}

void Mesh::cleanup() {
	/*glDeleteVertexArrays(1, _vao);
	glDeleteBuffers(1, _ebo);
	glDeleteBuffers(1, _vbo);
	glDeleteTextures(1, &_texture_id);*/
}

void Mesh::draw(vmath::mat4 projection_matrix, vmath::vec4 cameraPos) {
	_shader.use();

	// Activamos el vertex array Object
	glBindVertexArray(_vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo[0]);
	// Activamos el buffer de indices

	glUniformMatrix4fv(_uniformProjectionId, 1, GL_FALSE, projection_matrix);


	for (GLuint i = 0; i < _eboDataCount; i++) {
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, _tso[i]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo[i]);
		glDrawElements(GL_TRIANGLES, _elementBufferObjectData[i].indicesCount, GL_UNSIGNED_INT, 0);
	}
}




