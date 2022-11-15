/*
 * @Author: Raul Cisneros
 * @Date:   11/1/2021
 */
#include "ofApp.h"


bool Plane::intersect(const Ray& ray, glm::vec3& point, glm::vec3& normalAtIntersect) { 
	float dist; 
	bool insidePlane = false; 
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal, dist); 
	if (hit) { 
		Ray r = ray; point = r.evalPoint(dist); 
		normalAtIntersect = this->normal; 
		glm::vec2 xrange = glm::vec2(position.x - width / 2, position.x + width / 2); 
		glm::vec2 zrange = glm::vec2(position.z - height / 2, position.z + height / 2); 
		if (point.x < xrange[1] && point.x > xrange[0] && point.z < zrange[1] && point.z > zrange[0]) { 
			insidePlane = true; 
		} 
	}
	return insidePlane; 
}
/*
bool Plane::intersect(const Ray& ray, glm::vec3& point, glm::vec3& normalAtIntersect) {
	float dist;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal, dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
	}
	return (hit);
}
*/
// Convert (u, v) to (x, y, z) 
// We assume u,v is in [0, 1]
glm::vec3 ViewPlane::toWorld(float u, float v) {
	float w = width();
	float h = height();
	return (glm::vec3((u * w) + min.x, (v * h) + min.y, position.z));

}

// Get a ray from the current camera position to the (u, v) position on
// the ViewPlane
Ray RenderCam::getRay(float u, float v) {
	glm::vec3 pointOnPlane = view.toWorld(u, v);
	return(Ray(position, glm::normalize(pointOnPlane - position)));
}
void ofApp::rayTrace() {
	
	newColor = 0;
	for (int j = 0; j < imageHeight; j++) {
		for (int i = 0; i < imageWidth; i++) {

			//converts i and j to u and v
			u = (i + 0.5) / imageWidth; 
			v = (j + 0.5) / imageHeight;

			//gets ray
			ray = renderCam.getRay(u, v);
			
			hit = false;

			distance = std::numeric_limits<float>::infinity();

			closestObj = NULL;
			
			
			for (int k = 0; k < scene.size(); k++) {
			
				//checks to see if the scene intersects
				if (scene[k]->intersect(ray, intersectPoint, normal)) {
					
					//calculates current distance
					currentdis = sqrt(pow(intersectPoint.x - ray.p.x, 2) + pow(intersectPoint.y - ray.p.y, 2)) + pow(intersectPoint.z - ray.p.z, 2);
					
					// determine if we hit the object and save closest obj
					if (distance > currentdis) {
						distance = currentdis;
						closestObj = scene[k];
					}
					hit = true;
				}
				if (hit == true) {
					// if there is a hit draw the objects in scene
					color = closestObj->diffuseColor;
				
					closestObj->intersect(ray, intersectPoint,normal);

					color = getColor(intersectPoint);

					for (int m = 0; m < lights.size(); m++) {
						powerValue = this->power;
						newColor = LambertPhong(ray, intersectPoint, normal, color, ofColor::white, powerValue);
						image.setColor(i, imageHeight - 1 - j, newColor);
					}
				}
				else {
					//draws background
					color = ofGetBackgroundColor();
					image.setColor(i, imageHeight - 1 - j, color);
				}
			}
		}
	}
	//saves the created image
	image.save("render.png");
	
}

//--------------------------------------------------------------
void ofApp::setup() {
	
	gui.setup();

	gui.add(intensity.setup("Intensity", 100, 1, 200));
	gui.add(power.setup("Power", 40, 0, 200));

	
	ofSetBackgroundColor(ofColor::black);

	
	//adds the spheres and planes to the scene 
	scene.push_back(plane);
	scene.push_back(sphere);
	scene.push_back(sphere2);
	scene.push_back(sphere3);

	addLight(light);
//	addLight(light2);

	// initially set theCam pointer to point to the easyCam
	theCam = &mainCam;           
	mainCam.setDistance(50);
	mainCam.setNearClip(.1);

	//sets up sidecam 
	sideCam.setPosition(glm::vec3(100, 0, 0));
	sideCam.lookAt(glm::vec3(0, 0, 0));
	sideCam.setNearClip(.1);
	
	//sets up the preview cam
	previewCam.setPosition(renderCam.position);
	previewCam.lookAt(renderCam.aim);

	//allocates space for an image
	image.allocate(imageWidth, imageHeight, ofImageType::OF_IMAGE_COLOR);
	image.save("render.png");
	
	texture.allocate(texture.getWidth(), texture.getHeight(), ofImageType::OF_IMAGE_COLOR);
	texture.load("test.jpg");

	applyText(texture);
}

void ofApp::addLight(Light* light) {
	lights.push_back(light);
}

//--------------------------------------------------------------
void ofApp::update() {

}
void ofApp::applyText(ofImage texture) {
	this->texture = texture;
	textureSet = true;
}
ofColor ofApp::getColor(glm::vec3 vec){

	if (textureSet && normal == glm::vec3(0, 1, 0)) {
		
		min = glm::vec3(plane->position.x - plane->width / 2, plane->position.y, plane->position.z - plane->height / 2);

		UText = ((vec.x - min.x) / plane->width) * 10;
		VText = ((vec.z - min.z) / plane->height) * 10;

		IPixel = IJConversion(UText,texture.getWidth());
		JPixel = IJConversion(VText, texture.getHeight());

		return texture.getColor(fmod(IPixel, texture.getWidth()), fmod(JPixel, texture.getHeight()));
	}
	else {
		return closestObj->diffuseColor;
	//	return plane->diffuseColor;
	}
}
float ofApp::IJConversion(float uv, float size)
{
	return uv * size - 0.5;
}
//--------------------------------------------------------------
void ofApp::draw() {
	
	if (guiHide == true) {
		ofDisableDepthTest();
		gui.draw();
	}
	else {
		ofEnableDepthTest();
	}
	
	// calls begin on camera instance theCam points to
	theCam->begin();

	mainCam.draw();
	
	//fills spheres
	ofFill();
	
	// draws scene
	for (int i = 0; i < scene.size(); i++) {
		scene[i]->draw();

	}

	//does fill cameras
	ofNoFill();

	renderCam.view.draw();
	renderCam.draw();


	//does fill cameras
	ofNoFill();
	
	theCam->end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch (key) {
	case 'r':
		//calls raytrace method
		cout << "Rendering..." << endl;
		rayTrace();
		cout << "done" << endl;
		break;
	case OF_KEY_UP:
		// swaps camera pointer theCam to easyCam
		theCam = &mainCam;
		break;
	case OF_KEY_DOWN:
		// swaps camera pointer theCam to previewcam
		theCam = &previewCam;
		break;
	case ' ':
		// swaps camera pointer theCam to sidecam
		theCam = &sideCam;
		break;
	case 'g':
		guiHide = !guiHide;
		break;
	default:
		break;
	}
}
ofColor ofApp::lambert(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse) {
	for (int i = 0; i < lights.size(); i++) {
		
		ILLUMINATION = lights[i]->intensity;
		RADIUS = glm::length(lights[i]->position - p);
		MAX = std::max(0.0f, glm::dot(glm::normalize(norm), glm::normalize(lights[i]->position - p)));
		
		col = diffuse * (ILLUMINATION / (RADIUS * RADIUS)) * MAX;
	}
	return col;
}
ofColor ofApp::phong(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, const ofColor specular, float power) {
	
	for (int i = 0; i < lights.size(); i++) {
		V = glm::normalize(renderCam.position - p);
		l = glm::normalize(lights[i]->position - p);
		ILLUMINATION = lights[i]->intensity;
		RADIUS = glm::length(lights[i]->position - p);
		MAX = std::max(0.0f, glm::dot(glm::normalize(norm), glm::normalize((l + V) / glm::length(l + V))));
		
		col2 = specular * (ILLUMINATION / (RADIUS * RADIUS)) * std::pow(MAX, power); 
	}
	return col2;
}
bool ofApp::inShadow(Ray ray,glm::vec3& p, glm::vec3& norm)
{	
	for (int i = 0; i < scene.size(); i++)
	{
		if (scene[i]->intersect(ray, p, norm) == true){
			return true;
		}
	
	}
	return false;
}
ofColor ofApp::LambertPhong(Ray ray, const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, const ofColor specular, float power) {
	//sets ambient shading
	finalColor = 0.25 * (diffuse);

	for (int i = 0; i < lights.size(); i++) {

		//calculates point near surface that where shade ray begins 
		shadowRay = p + 0.0001 * glm::normalize(norm);
		shadeRay = Ray(shadowRay, glm::normalize(lights[i]->position - p));

		if (inShadow(shadeRay, blockedIntersect, blockedNormal) == false) {

			lights[i]->intensity = this->intensity;

			//lambert calculations
			ILLUMINATION = lights[i]->intensity;
			RADIUS = glm::length(lights[i]->position - p);
			MAX = std::max(0.0f, glm::dot(glm::normalize(norm), glm::normalize(lights[i]->position - p)));

			finalColor += diffuse * (ILLUMINATION / (RADIUS * RADIUS)) * MAX;

			//phong calculations
			MAX = std::max(0.0f, glm::dot(glm::normalize(norm), glm::normalize(glm::normalize(renderCam.position - p) + glm::normalize(lights[i]->position - p))));

			finalColor += specular * (ILLUMINATION / (RADIUS * RADIUS)) * std::pow(MAX, power);

		}

	}
	return finalColor;

}
//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}


