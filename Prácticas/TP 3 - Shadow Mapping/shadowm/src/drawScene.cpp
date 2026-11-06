#include <glm/ext.hpp>
#include "drawScene.hpp"
#include "Model.hpp"
#include "Shaders.hpp"
#include "Callbacks.hpp"
#include "Window.hpp"

extern Shader shader_texture, shader_phong, shader_smap;
extern Model model_chookity, model_teapot, model_suzanne, model_floor_flat, model_floor_random, model_light, model_crate;
extern glm::vec4 lightPosition;
extern bool flat_floor;

void drawModel(Window &window, const Model &model, const glm::mat4 &m, int pass) {
	
	Shader &shader = [&]()->Shader&{
		if (pass==1) return shader_smap;
		if (model.texture.isOk()) {
			model.texture.bind(1);
			return shader_texture;
		}
		return shader_phong;
	}();
	
	shader.use();
	
	setMatrixes(window, shader, m);
	
	glm::mat4 lightView = glm::lookAt(glm::vec3(lightPosition),
									  glm::vec3( 0.0f, 0.0f,  0.0f),
									  glm::vec3( 0.0f, 1.0f,  0.0f));
	
	constexpr float width = 4.f, near_plane = 0.5f, far_plane = 10.f;
	glm::mat4 lightProjection = glm::ortho(-width, width, -width, width, near_plane, far_plane);
	
	
	// La matriz de proyeccion de la luz esta armada como una proyeccion ortogonal
	// conformada por el tamaño del frustum (sus 4 lados, el plano cerano y el plano lejano).
	
	// Al usar una proyeccion ortogonal, es como si los rayos de luz viniesen desde
	// el infinito en la posicion donde esta la luz, pero esto no simula correctamente
	// una luz con posicion real dentro de la escena, para ese caso seria necesaria
	// una proyeccion perspectiva.
	
	
	// En mi cabeza tiene sentido decir que si el espaciamiento entre los planos
	// znear y zfar es más pequeño, debería tener mayor precisión sobre la
	// nueva zona donde quede el frustum, ya que tengo menos espacio asignado
	// a la misma cantidad de texeles.
	
	// Ademas, se puede reducir el ancho del cuadro del frustum, esto tambien
	// crearia un efecto similar, donde es posible obtener mayor "definicion"
	// de texeles sin necesariamente aumentar la resolucion del shadowmap
	
	
	// Hay dos posibilidades principales que causan fallos al modificar la
	// matriz. La primera es si el plano znear está muy lejos o el zfar demasiado
	// cerca, de tal forma que los valores de profunidad no pueden ser calculados
	// para ciertas partes del shadowmap.
	// La segunda es si el tamaño del ancho del frustum es demasiado pequeño
	// y quedan partes del shadowmap fuera del frustum, en este caso no
	// podríamos encontrar a qué posición del shadowmap corresponde un fragmento.
	
	
	shader.setUniform("lightViewMatrix",lightView);
	shader.setUniform("lightProjectionMatrix",lightProjection);
	
	CameraSettings &cs = window.getCamera();
	shader.setUniform("viewPos", cs.view_pos);
	
	shader.setUniform("depthTexture", 0);
	shader.setUniform("colorTexture", 1);
	
	// setup light and material
	shader.setLight(lightPosition, glm::vec3{1.f,1.f,1.f}, 0.4f);
	shader.setMaterial(model.material);
	
	// send geometry
	shader.setBuffers(model.buffers);
	model.buffers.draw();
}

void drawScene(Window &window, int pass) {
	static glm::vec3 y_axis(0.f,1.f,0.f);
	static const glm::mat4 identity(1.f);
	
	// floor (dos veces porque no es cerrada, por si activan el cull face)
	glFrontFace(GL_CCW);
	drawModel(window, flat_floor?model_floor_flat:model_floor_random, identity, pass);
	glFrontFace(GL_CW);
	drawModel(window, flat_floor?model_floor_flat:model_floor_random, identity, pass);
	glFrontFace(GL_CCW);
	
	// teapots
	static glm::mat4 teapot1_matrix =
		glm::translate(identity,glm::vec3(-1.25f,+0.2f,-1.8f) ) *
		glm::rotate( identity, 1.23f, y_axis ) *
		glm::scale( identity, glm::vec3(.2f) );
	drawModel(window, model_teapot, teapot1_matrix, pass);
	static glm::mat4 teapot2_matrix =
		glm::translate(identity,glm::vec3(+1.8f,+0.2f,-0.25f) ) *
		glm::rotate( identity, -0.53f, y_axis ) *
		glm::scale( identity, glm::vec3(.2f) );
	drawModel(window, model_teapot, teapot2_matrix, pass);
	
	// crates
	static glm::mat4 crate1_matrix =
		glm::translate(identity,glm::vec3(-1.1f,-0.25f,+0.6f) ) *
		glm::rotate( identity, 0.75f, y_axis );
	drawModel(window, model_crate, crate1_matrix, pass);
	static glm::mat4 crate2_matrix =
		glm::translate(identity,glm::vec3(-0.5f,-0.25f,+1.5f) ) *
		glm::rotate( identity, 2.5f, y_axis );
	drawModel(window, model_crate, crate2_matrix, pass);
	
	// suzanne
	static glm::mat4 suzzane_matrix =
		glm::translate( identity, glm::vec3(1.4f,0.f,1.2f) ) *
		glm::rotate( identity, -0.75f, glm::vec3(0.8f,0.4f,-0.13f) ) *
		glm::scale( identity, glm::vec3(.5f) );
	drawModel(window, model_suzanne, suzzane_matrix, pass);
	
	// chookity
	static glm::mat4 chookity_matrix =
		glm::translate( identity, y_axis*0.7f ) *
		glm::rotate( identity, 0.5f, y_axis ) *
		glm::scale( identity, glm::vec3(.4f) );
	drawModel(window, model_chookity, chookity_matrix, pass);
}

