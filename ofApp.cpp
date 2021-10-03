#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	ofSetFrameRate(60);
	ofSetWindowTitle("openframeworks");

	ofBackground(239);
	ofSetLineWidth(0.5);
	ofEnableDepthTest();

	this->fbo.allocate(ofGetWidth(), ofGetHeight());
	this->fbo.begin();
	ofClear(0);

	ofTrueTypeFont font;
	font.loadFont("fonts/Kazesawa-bold.ttf", 280, true, true, true);
	string word = "NFT";
	font.drawString(word, ofGetWidth() * 0.5 - font.stringWidth(word) * 0.5, ofGetHeight() * 0.5 + font.stringHeight(word) - 140);

	this->fbo.end();
	this->fbo.readToPixels(this->pixels);

	ofColor color;
	vector<int> hex_list = { 0x247BA0, 0x70C1B3, 0xB2DBBF, 0xF3FFBD, 0xFF1654 };
	vector<ofColor> base_color_list;
	for (auto hex : hex_list) {

		color.setHex(hex);
		base_color_list.push_back(color);
	}

	for (int i = 0; i < 1000; i++) {

		ofMesh mesh, frame;
		frame.setMode(ofPrimitiveMode::OF_PRIMITIVE_LINES);

		glm::vec3 base;
		while (true) {

			int x = ofRandom(ofGetWidth());
			int y = ofRandom(ofGetHeight());
			if (this->pixels.getColor(x, y) != ofColor(0, 0)) {

				base = glm::vec3(x, y, ofRandom(-30, 30));
				break;
			}
		}
		switch ((int)ofRandom(3)) {
		case 0:
			mesh.addVertex(base + glm::vec3(3, 0, 0));
			mesh.addVertex(base + glm::vec3(-3, 0, 0));
			frame.addVertex(base + glm::vec3(3, 0, 0));
			frame.addVertex(base + glm::vec3(-3, 0, 0));
			break;
		case 1:
			mesh.addVertex(base + glm::vec3(0, 3, 0));
			mesh.addVertex(base + glm::vec3(0, -3, 0));
			frame.addVertex(base + glm::vec3(0, 3, 0));
			frame.addVertex(base + glm::vec3(0, -3, 0));
			break;
		case 2:
			mesh.addVertex(base + glm::vec3(0, 0, 3));
			mesh.addVertex(base + glm::vec3(0, 0, -3));
			frame.addVertex(base + glm::vec3(0, 0, 3));
			frame.addVertex(base + glm::vec3(0, 0, -3));
			break;
		}

		frame.addIndex(0);
		frame.addIndex(1);

		this->face_list.push_back(mesh);
		this->frame_list.push_back(frame);

		auto direction = glm::vec3(ofRandom(-1, 1), ofRandom(-1, 1), ofRandom(-1, 1));
		direction = glm::normalize(direction) * 3;
		this->direction_list.push_back(direction);

		auto noise_seed = glm::vec3(glm::vec3(ofRandom(1000), ofRandom(1000), ofRandom(1000)));
		this->noise_seed_list.push_back(noise_seed);

		this->noise_step_list.push_back(0.f);

		ofColor color;
		color.setHsb(ofRandom(255), 130, 255);
		this->color_list.push_back(color);
	}
}

//--------------------------------------------------------------
void ofApp::update() {

	for (int i = this->face_list.size() - 1; i > -1; i--) {

		while (true) {

			this->noise_step_list[i] += 0.005;

			auto angle_x = ofMap(ofNoise(this->noise_seed_list[i].x, this->noise_step_list[i]), 0, 1, -PI, PI);
			auto angle_y = ofMap(ofNoise(this->noise_seed_list[i].y, this->noise_step_list[i]), 0, 1, -PI, PI);
			auto angle_z = ofMap(ofNoise(this->noise_seed_list[i].z, this->noise_step_list[i]), 0, 1, -PI, PI);

			auto rotation_x = glm::rotate(glm::mat4(), angle_x, glm::vec3(1, 0, 0));
			auto rotation_y = glm::rotate(glm::mat4(), angle_y, glm::vec3(0, 1, 0));
			auto rotation_z = glm::rotate(glm::mat4(), angle_z, glm::vec3(0, 0, 1));

			glm::vec3 rotate_direction = glm::vec4(this->direction_list[i], 0) * rotation_z * rotation_y * rotation_x;

			auto right = this->face_list[i].getVertex(this->face_list[i].getNumVertices() - 2);
			auto left = this->face_list[i].getVertex(this->face_list[i].getNumVertices() - 1);
			auto center = (right + left) / 2;

			auto feature = center + rotate_direction;
			int x = feature.x;
			int y = feature.y;
			if (feature.z > -30 && feature.z < 30) {

				if (this->pixels.getColor(x, y) != ofColor(0, 0)) {

					this->face_list[i].addVertex(right + rotate_direction);
					this->face_list[i].addVertex(left + rotate_direction);

					this->frame_list[i].addVertex(right + rotate_direction);
					this->frame_list[i].addVertex(left + rotate_direction);

					break;
				}
			}
		}

		if (this->face_list[i].getNumVertices() > 30) {

			this->face_list[i].removeVertex(0);
			this->face_list[i].removeVertex(1);
		}
		else {

			int index = this->face_list[i].getNumVertices() - 1;
			this->face_list[i].addIndex(index); this->face_list[i].addIndex(index - 2); this->face_list[i].addIndex(index - 3);
			this->face_list[i].addIndex(index); this->face_list[i].addIndex(index - 1); this->face_list[i].addIndex(index - 3);
		}

		if (this->frame_list[i].getNumVertices() > 30) {

			this->frame_list[i].removeVertex(0);
			this->frame_list[i].removeVertex(1);
		}
		else {

			int index = this->frame_list[i].getNumVertices() - 1;
			this->frame_list[i].addIndex(index); this->frame_list[i].addIndex(index - 2);
			this->frame_list[i].addIndex(index - 1); this->frame_list[i].addIndex(index - 3);

			if (index == 29) {

				this->frame_list[i].addIndex(28);
				this->frame_list[i].addIndex(29);
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	this->cam.begin();
	ofRotateX(180);
	ofRotateZ(45);

	if (ofGetFrameNum() % 480 > 120 && ofGetFrameNum() % 480 < 240) {

		ofRotateY(ofGetFrameNum() * 3);
	}
	else if(ofGetFrameNum() % 480 > 360) {

		ofRotateX(ofGetFrameNum() * 3);
	}

	ofTranslate(ofGetWidth() * -0.5, ofGetHeight() * -0.5);

	for (int i = 0; i < this->face_list.size(); i++) {

		ofSetColor(this->color_list[i]);
		this->face_list[i].draw();

		ofSetColor(139);
		this->frame_list[i].drawWireframe();
	}

	this->cam.end();
}

//--------------------------------------------------------------
int main() {

	ofSetupOpenGL(720, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}