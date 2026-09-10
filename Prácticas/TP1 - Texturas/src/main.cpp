#include <algorithm>
#include <stdexcept>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "Model.hpp"
#include "Window.hpp"
#include "Callbacks.hpp"
#include "Debug.hpp"
#include "Shaders.hpp"

#define VERSION 20260831

Window main_window; // ventana principal: muestra el modelo en 3d sobre el que se pinta
Window aux_window; // ventana auxiliar que muestra la textura
void drawMain(); // dibuja el modelo "normalmente" para la ventana principal
void drawBack(); // dibuja el modelo con un shader alternativo para convertir coords de la ventana a coords de textura
void drawAux(); // dibuja la textura en la ventana auxiliar
void drawImGui(Window &window); // settings sub-window

float radius = .5; // radio del "pincel" con el que pintamos en la textura
glm::vec4 color = { 0.f, 0.f, 0.f, 1.f }; // color actual con el que se pinta en la textura

//---------------
double xprev; //globales para las posiciones previas
double yprev;
//---------------

Texture texture; // textura (compartida por ambas ventanas)
Image image; // imagen (para la textura, Image está en RAM, Texture la envía a GPU)

Model model_chookity; // el objeto a pintar, para renderizar en la ventan principal
Model model_aux; // un quad para cubrir la ventana auxiliar y mostrar la textura

Shader shader_main; // shader para el objeto principal (drawMain)
Shader shader_aux; // shader para la ventana auxiliar (drawTexture)

// callbacks del mouse y auxiliares para los callbacks
enum class MouseAction { None, ManipulateView, Draw };
MouseAction mouse_action = MouseAction::None; // qué hacer en el callback del motion si el botón del mouse está apretado
void mainMouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
void mainMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void auxMouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
void auxMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

int main() {
	
	// main window (3D view)
	
	main_window = Window(800, 600, "Main View");
	glfwSetCursorPosCallback(main_window, mainMouseMoveCallback);
	glfwSetMouseButtonCallback(main_window, mainMouseButtonCallback);
	main_window.getCamera().model_angle = 2.5;
	
	glClearColor(1.f,1.f,1.f,1.f);
	shader_main = Shader("shaders/main");
	
	image = Image("models/chookity.png",true);
	texture = Texture(image);
	
	model_chookity = Model::loadSingle("models/chookity", Model::fNoTextures);
	
	// aux window (texture image)
	Window aux_window(512,512, "Texture", Window::fDefaults, main_window);
	glfwSetCursorPosCallback(aux_window, auxMouseMoveCallback);
	glfwSetMouseButtonCallback(aux_window, auxMouseButtonCallback);
	
	model_aux = Model::loadSingle("models/texquad", Model::fNoTextures);
	shader_aux = Shader("shaders/quad");
	
	// main loop
	do {
		glfwMakeContextCurrent(main_window);
		drawMain();
		drawImGui(main_window);
		glFinish();
		glfwSwapBuffers(main_window);
		
		glfwMakeContextCurrent(aux_window);
		drawAux();
		drawImGui(aux_window);
		glFinish();
		glfwSwapBuffers(aux_window);
		
		glfwPollEvents();
		
	} while( (not glfwWindowShouldClose(main_window)) and (not glfwWindowShouldClose(aux_window)) );
	main_window.disableImGui();
	aux_window.disableImGui();
}



// ===== pasos del renderizado =====

void drawMain() {
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	texture.bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	shader_main.use();
	setMatrixes(main_window, shader_main);
	shader_main.setLight(glm::vec4{-1.f,1.f,4.f,1.f}, glm::vec3{1.f,1.f,1.f}, 0.35f);
	shader_main.setMaterial(model_chookity.material);
	shader_main.setBuffers(model_chookity.buffers);
	model_chookity.buffers.draw();
}

void drawAux() {
	glDisable(GL_DEPTH_TEST);
	texture.bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	shader_aux.use();
	shader_aux.setMatrixes(glm::mat4{1.f}, glm::mat4{1.f}, glm::mat4{1.f});
	shader_aux.setBuffers(model_aux.buffers);
	model_aux.buffers.draw();
}

void drawBack() {
	glfwMakeContextCurrent(main_window);
	glDisable(GL_MULTISAMPLE);

	/// @ToDo: Parte 2: renderizar el modelo en 3d con un nuevo shader de forma 
	///                 que queden las coordenadas de textura de cada fragmento
	///                 en el back-buffer de color
	
	glEnable(GL_MULTISAMPLE);
	glFlush();
	glFinish();
}

void drawImGui(Window &window) {
	if (!glfwGetWindowAttrib(window, GLFW_FOCUSED)) return;
	// settings sub-window
	window.ImGuiDialog("Settings",[&](){
		ImGui::SliderFloat("Radius",&radius,.5,50);
		ImGui::ColorEdit4("Color",&(color[0]),0);
		
		static std::vector<std::pair<const char *, ImVec4>> pallete = { // colores predefindos
			{"white" , {1.f,1.f,1.f,1.f}},
			{"pink"  , {0.749f,0.49f,0.498f,1.f}},
			{"yellow", {0.965f,0.729f,0.106f,1.f}},
			{"black" , {0.f,0.f,0.f,1.f}} };
		
		ImGui::Text("Pallete:");
		for (auto &p : pallete) {
			ImGui::SameLine();
			if (ImGui::ColorButton(p.first, p.second))
				color[0] = p.second.x, color[1] = p.second.y, color[2] = p.second.z;
		}
		
		if (ImGui::Button("Reload Image")) {
			image = Image("models/chookity.png",true);
			texture.update(image);
		}
	});
}



// ===== callbacks de la ventana auxiliar (textura) =====

void auxMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (ImGui::GetIO().WantCaptureMouse) return;
	if (action==GLFW_PRESS) {
		mouse_action = MouseAction::Draw;
		
		/// @ToDo: Parte 1: pintar un punto de radio "radius" en la imagen
		///                 "image" que se usa como textura
		
		/// por ahora ignoramos el radio, necesito transformar la posicion de la
		/// ventana de forma que cuando intente pintar siempre este dentro de
		/// la textura
		
		double posx; double posy;
		glfwGetCursorPos(window, &posx, &posy); // recupero la pos del mouse en la ventana
		
		// recupero altura y ancho total de la textura
		float imageWidth = image.GetWidth();
		float imageHeight = image.GetHeight();
		
		/// voy a necesitar el tamanio total de la ventana, se me ocurre dividir
		/// la pos actual del mouse por el tamanio total, multiplicar eso por
		/// el tamanio de la textura ya sea en X o Y y ahi le pego un round
		
		BufferSize bs = getBufferSize(window);
		
		posx = round(posx/bs.width*imageWidth);
		posy = round((1 - posy/bs.height)*imageHeight);
		///me toco invertir el scaling de la y porque el SetRGB parece tomar
		///y positiva para arriba arancando desde abajo
		
		xprev = posx; yprev = posy; //guardo las coords como previas
		
		/// he vuelto, ahora tengo que hacer lo del radio.
		/// la formulita para hacer un circulo es (x-xoffset)^2+(y-yoffset)^2=r^2
		/// los offsets para este caso son los posx y posy que ya calculamos
		/// le ponemos <= para que rellene el circulo: (x-posx)^2+(y-posy)^2<=r^2
		
		/// ¿cómo dibujo esos pixeles? ¿un doble for que recorra toda la
		/// pantalla y dibuje únicamente los pixeles que satisfagan la ecuación?
		
		/// no suena mal, pero podría ser mejor, puedo evitar recorrer toda la
		/// pantalla, solo tengo que revisar el cuadradito comprendido por
		/// posx, posy +- el radio
		
		for (int i=(posy-radius);i<(posy+radius);i++){
			for(int j=(posx-radius);j<(posx+radius);j++){
				if ((pow((j-posx),2)+pow((i-posy),2))<=pow(radius,2)){
					//aca le agrego un checkeo para no pintar si se sale de la imagen
					if (i<0 || j<0 || i>(imageHeight-1) || j>(imageWidth-1)) {continue;}
					//aca obtengo el color de la imagen para mezclar los colores transparentes
					glm::vec3 oldColor = image.GetRGB(i, j);
					float alpha = color[3];
					glm::vec3 brushColor = glm::vec3(color);
					//formulita para el color final, nuestro pincel tiene el peso
					//del alpha y el color original tiene el resto del peso, el
					//alpha resultante siempre queda en 1
					glm::vec3 result = brushColor * alpha + oldColor * (1-alpha);
					
					image.SetRGB(i, j, result); // pinto
				}
			}
		}
		
		texture.update(image); //actualizo textura
		
	} else {
		mouse_action = MouseAction::None;
	}
}

void auxMouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
	if (mouse_action!=MouseAction::Draw) return;
	
	/// @ToDo: Parte 1: pintar un segmento de ancho "2*radius" en la imagen
	///                 "image" que se usa como textura
	
	//std::cout << "Arrastre - X: " << xpos << ", Y: " << ypos << std::endl;
	
/// misma logica de reescalado para pintar donde esta el mouse
	
	float imageWidth = image.GetWidth();
	float imageHeight = image.GetHeight();
	BufferSize bs = getBufferSize(window);
	xpos = round(xpos/bs.width*imageWidth);
	ypos = round((1 - ypos/bs.height)*imageHeight);
	
/// acá toca implementar el DDA de línea (Digital Differential Analyzer)
/// este algoritmo sirve para rasterizar una linea recta entre dos puntos.
/// o sea, agarrás de input las coordenadas de dos puntos y en base a eso
/// pintás ciertos pixeles que formen una especie de "línea recta" entre ellos.

/// para esto vamos a usar la pendiente de la recta, que la calculamos restando
/// los puntitos y dividiendo las restas (la ecuacion punto-punto).

/// el algoritmo contempla dos situaciones, una donde la X es dominante y la
/// otra donde la Y es dominante, esto va a dictar cuál de las dos coordenadas
/// va a tener la pendiente como paso

/// entonces, si se mueve mucho en X (o sea que |dx| es más grande que |dy|)
/// vamos a ir sumando de a 1 en X y a Y le sumamos la pendiente de la recta,
/// a esa suma la vamos a ir redondeando para pintar (porque son pixeles, o
/// sea que son valores enteros, y además el redondeo es parte de lo que nos
/// garantiza la contiguidad de la linea).
/// caso inverso si se mueve mucho en Y (|dy| más grande, sumamos 1 en y, sumamos
/// pendiente a x)
	
/// tenemos además que contemplar los casos donde el usuario quiera pintar
/// para el lado inverso al de crecimiento de los ejes, para esto revisamos
/// despues de determinar si se mueve mucho en X o en Y si ese movimiento es
/// positivo o negativo. si es negativo, damos vuelta los puntos y recalculamos
/// las diferencias acordemente, esto hace que no tengamos que modificar la
/// condición del loop, ya que la coordenada calculada siempre va a ser mas
/// pequeña que la actual
	
/// de ahí queda lo mas importante, vamos dibujando los puntos interpolados hasta
/// que alcancen la posicion actual del mouse


/// para implementar esto necesito variables globales donde me guardo la pos previa
/// del mouse en la textura porque sino no tengo forma de calcular los dx y dy.
/// ademas, necesito guardarmela tambien apenas se hace el primer clic
/// por lo que voy a modificar el eventito de arriba
	
/// AVANCE
/// ahora que agregué soporte para el radio en el single-click, vamos a meterlo
/// en el algoritmo de DDA, debería ser bastante copiar y pegar
	
/// AVANCE FINAL
/// implemente soporte para el alpha channel, lo único que hace es detectar
/// el alpha del pincel y modificar el color real que se va a pintar teniendo
/// en cuenta el color antiguo, le pone el peso del alpha al color del pincel
/// y el resto del peso al color original de la imagen
	
/// un detalle no menor, para evidenciar el alpha usando el DDA necesitamos
/// valores extremadamente pequeños, esto es porque el círculo grandote del
/// pincel se está dibujando una cantidad ridícula de veces y se solapa con
/// los anteriores, supongo que esto no es necesario fixearlo ya
	
	double xcurrent = xpos; double ycurrent = ypos; //necesito estas para guardarlas como previas al final
	double dx = xpos-xprev; double dy = ypos-yprev;
	double x; double y;
	
	if (abs(dx)>abs(dy)){
		
		if(dx<0){
			double temp;
			temp = xpos; xpos = xprev; xprev = temp;
			temp = ypos; ypos = yprev; yprev = temp;
			dx = xpos-xprev; dy = ypos-yprev;
		}
		
		x = round(xprev); y = yprev;
		
		while(x<xpos){
			for (int i=(y-radius*2);i<(y+radius*2);i++){
				for(int j=(x-radius*2);j<(x+radius*2);j++){
					if ((pow((j-x),2)+pow((i-y),2))<=pow(radius*2,2)){
						if (i<0 || j<0 || i>(imageHeight-1) || j>(imageWidth-1)) {continue;}
						glm::vec3 oldColor = image.GetRGB(i, j);
						float alpha = color[3];
						glm::vec3 brushColor = glm::vec3(color);
						//formulita para el color final, nuestro pincel tiene el peso
						//del alpha y el color original tiene el resto del peso, el
						//alpha resultante siempre queda en 1
						glm::vec3 result = brushColor * alpha + oldColor * (1-alpha);
						image.SetRGB(i, j, result); // pinto
					}
				}
			}
			x++; y+=dy/dx;
		}
		
	} else {
		
		if(dy<0){
			double temp;
			temp = xpos; xpos = xprev; xprev = temp;
			temp = ypos; ypos = yprev; yprev = temp;
			dx = xpos-xprev; dy = ypos-yprev;
		}
		
		x = xprev; y = round(yprev);
		
		while(y<ypos){
			for (int i=(y-radius*2);i<(y+radius*2);i++){
				for(int j=(x-radius*2);j<(x+radius*2);j++){
					if ((pow((j-x),2)+pow((i-y),2))<=pow(radius*2,2)){
						if (i<0 || j<0 || i>(imageHeight-1) || j>(imageWidth-1)) {continue;}
						glm::vec3 oldColor = image.GetRGB(i, j);
						float alpha = color[3];
						glm::vec3 brushColor = glm::vec3(color);
						//formulita para el color final, nuestro pincel tiene el peso
						//del alpha y el color original tiene el resto del peso, el
						//alpha resultante siempre queda en 1
						glm::vec3 result = brushColor * alpha + oldColor * (1-alpha);
						image.SetRGB(i, j, result); // pinto
					}
				}
			}
			y++; x+=dx/dy;
		}
		
	}
	
	texture.update(image); //actualizo textura
	xprev = xcurrent; yprev = ycurrent; //guardo las coords como previas
	

	
	
	
}


// ===== callbacks de la ventana principal (vista 3D) =====

void mainMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (ImGui::GetIO().WantCaptureMouse) return;
	if (action==GLFW_PRESS) {
		if (mods!=0 or button==GLFW_MOUSE_BUTTON_RIGHT) {
			mouse_action = MouseAction::ManipulateView;
			common_callbacks::mouseButtonCallback(window, GLFW_MOUSE_BUTTON_LEFT, action, mods);
			return;
		}
		
		mouse_action = MouseAction::Draw;
		
		/// @ToDo: Parte 2: pintar un punto de radio "radius" en la imagen
		///                 "image" que se usa como textura
		
	} else {
		if (mouse_action==MouseAction::ManipulateView)
			common_callbacks::mouseButtonCallback(window, GLFW_MOUSE_BUTTON_LEFT, action, mods);
		mouse_action = MouseAction::None;
	}
}

void mainMouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
	if (mouse_action!=MouseAction::Draw) {
		if (mouse_action==MouseAction::ManipulateView);
			common_callbacks::mouseMoveCallback(window,xpos,ypos);
		return; 
	}
	
	/// @ToDo: Parte 2: pintar un segmento de ancho "2*radius" en la imagen
	///                 "image" que se usa como textura
	
}
