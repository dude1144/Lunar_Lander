#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxAssimpModelLoader.h"
#include "Octree.h"
#include "ParticleEmitter.h"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		void loadVbo();
		void drawAxis(glm::vec3 location);
		void initLightingAndMaterials();
		void restart();

		//cameras
		ofEasyCam cam;
		ofCamera trackCam;
		ofCamera frontCam;
		ofCamera bottomCam;
		ofCamera* theCam = &cam;

		ofxAssimpModelLoader mars, lander;
		ofLight light;
		Octree tree;

		//lander Particle System stuff
		glm::vec3 startingPosition = glm::vec3(0, 20, 0);
		ParticleSystem landerSystem;

		GravityForce* gravity;

		float dist = 0;
		int score = 0;

		//gui stuff
		ofxPanel gui;
		ofxFloatSlider gravitySlider;
		ofxFloatSlider magnitude;
		ofxFloatSlider restitution;
		
		//shader stuff
		ParticleEmitter exhaust;
		ofSoundPlayer exhaustPlayer;

		ofTexture  particleTex;
		ofVbo vbo;
		ofShader shader;

		//landing stuff
		glm::vec3 landingPosition = glm::vec3(15, -1, 17);
		float landingRadius = 6;

#if _DEBUG
		int numLevels = 8;
#else
		int numLevels = 13;
#endif

		//booleans to track state of game
		bool bSpacePressed = false;
		bool bWPressed = false;
		bool bAPressed = false;
		bool bSPressed = false;
		bool bDPressed = false;

		bool bStarted = false;
		bool bEnded = false;
		bool bDragging = false;
		bool bRoverLoaded = false;
		bool bTerrainSelected = false;
		bool bDrawTree = false;
		bool bShowGui = false;
		
		string message = "";
};
