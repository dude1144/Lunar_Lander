#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	//setup camera
	cam.setDistance(50);
	cam.setNearClip(.1);
	cam.setFov(65.5);

	trackCam.setGlobalPosition(glm::vec3(20, 20, 20));
	trackCam.lookAt(startingPosition);
	trackCam.setNearClip(.1);

	frontCam.setPosition(startingPosition);
	frontCam.lookAt(frontCam.getPosition() + glm::vec3(0, 0, -1));
	frontCam.setNearClip(.1);

	bottomCam.setPosition(startingPosition);
	bottomCam.lookAt(glm::vec3(0, 0, 0));
	bottomCam.setNearClip(.01);

	
	ofSetVerticalSync(true);
	ofEnableSmoothing();
	initLightingAndMaterials();
	ofBackground(ofColor::black);
	ofDisableArbTex();

	//load shader stuff
	if (!ofLoadImage(particleTex, "images/dot.png")) {
		cout << "Particle Texture File: images/dot.png not found" << endl;
		ofExit();
	}

	#ifdef TARGET_OPENGLES
		shader.load("shaders_gles/shader");
	#else
		shader.load("shaders/shader");
	#endif

	//setup GUI components
	gui.setup("sliders");
	gui.add(gravitySlider.setup("Gravity", 2.5, .1, 10));
	gui.add(magnitude.setup("Magnitude", 5, 1, 200));
	gui.add(restitution.setup("Restitution", .5, .1, 1));

	//setup models and octree
	mars.loadModel("geo/moon-houdini.obj");
	mars.setScaleNormalization(false);
	mars.setRotation(1, 180, 0, 0, 1);
	tree.create(mars.getMesh(0), numLevels);

	lander.loadModel("geo/lander.obj");
	lander.setScaleNormalization(false);
	lander.setScale(1, 1, 1);
	lander.setRotation(1, 180, 0, 0, 1);
	bRoverLoaded = true;

	//setup particle system
	Particle p;
	p.lifespan = -1;
	p.damping = .9999;
	p.position = startingPosition;

	gravity = new GravityForce(glm::vec3(0, -1 * gravitySlider, 0));

	landerSystem.add(p);
	landerSystem.addForce(gravity);

	//setup particle Emitter
	exhaust.velocity = glm::vec3(0, -15, 0);
	exhaust.rate = 30;
	exhaust.sys->addForce(new TurbulenceForce(glm::vec3(-90,-90,-90), glm::vec3(90, 90, 90)));
	exhaust.lifespan = .25;
	exhaust.particleColor = ofColor::white;
	
	exhaustPlayer.load("sounds/Exhaust.mp3");
	exhaustPlayer.setLoop(true);

	lander.setPosition(landerSystem.particles[0].position.x, landerSystem.particles[0].position.y, landerSystem.particles[0].position.z);
	exhaust.setPosition(landerSystem.particles[0].position);
}

void ofApp::loadVbo() 
{
	if (exhaust.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < exhaust.sys->particles.size(); i++) 
	{
		points.push_back(exhaust.sys->particles[i].position);
		sizes.push_back(ofVec3f(20));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vbo.clear();
	vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

//--------------------------------------------------------------
void ofApp::update()
{
	if (bStarted && !bEnded)
	{
		//update gravity
		gravity->set(glm::vec3(glm::vec3(0, -1 * gravitySlider, 0)));

		//add any required impulse forces
		if (bSpacePressed)
			landerSystem.addForce(new ImpulseForce(magnitude, glm::vec3(0, 1, 0)));
		if (bWPressed)
			landerSystem.addForce(new ImpulseForce(magnitude, glm::vec3(0, 0, -1)));
		if (bAPressed)
			landerSystem.addForce(new ImpulseForce(magnitude, glm::vec3(-1, 0, 0)));
		if (bSPressed)
			landerSystem.addForce(new ImpulseForce(magnitude, glm::vec3(0, 0, 1)));
		if (bDPressed)
			landerSystem.addForce(new ImpulseForce(magnitude, glm::vec3(1, 0, 0)));
		
		//get the location the lander particle would be at if updated
		Particle p = landerSystem.particles[0];
		landerSystem.test(&p);

		//check if any of the lander's feet would be colliding with the octree
		glm::vec3 collDist = glm::vec3(10000, 10000, 10000);

		if (tree.intersect(p.position + glm::vec3(2.8,0,0), tree.root, &collDist) ||
			tree.intersect(p.position + glm::vec3(-2.8, 0, 0), tree.root, &collDist) ||
			tree.intersect(p.position + glm::vec3(0, 0, 2.8), tree.root, &collDist) ||
			tree.intersect(p.position + glm::vec3(0, 0, -2.8), tree.root, &collDist))
		{
			if (glm::length(landerSystem.particles[0].velocity) > 7)
			{
				bEnded = true;
				message = "oops, you crashed!";
				score = 0;
			}
			else
			{
				//if any are, check if they are inside the landing area
				int feetCount = 0;
				if (glm::length(p.position + glm::vec3(2.8, 0, 0) - landingPosition) < landingRadius)
					feetCount++;
				if (glm::length(p.position + glm::vec3(-2.8, 0, 0) - landingPosition) < landingRadius)
					feetCount++;
				if (glm::length(p.position + glm::vec3(0, 0, 2.8) - landingPosition) < landingRadius)
					feetCount++;
				if (glm::length(p.position + glm::vec3(0, 0, -2.8) - landingPosition) < landingRadius)
					feetCount++;

				//if any are, end the game and save results
				if (feetCount > 0)
				{
					score += feetCount;
					switch (feetCount)
					{
					case 1:
						bEnded = true;
						message = "Good job, but you only got 1 foot in the landing area";
						break;
					case 2:
						bEnded = true;
						message = "Good job, but you were only half in the landing area";
						break;
					case 3:
						bEnded = true;
						message = "Good job, you were mostly in the landing area";
						break;
					case 4:
						bEnded = true;
						message = "Great job, you landed perfectly!";
						break;
					}
				}
				else
				{
					//if none of the feet are in the landing zone, either bounce the lander or end the game

					glm::vec3 n = glm::normalize(collDist);
					glm::vec3 impulse = (restitution ) * (glm::dot(-1 * landerSystem.particles[0].velocity, n)) * n;

					landerSystem.particles[0].velocity = impulse;
				}
			}
		}
		else
		{
			//update the particle position and the emitter
			landerSystem.update();
			exhaust.update();

			//set the lander position to the particle postition
			lander.setPosition(landerSystem.particles[0].position.x, landerSystem.particles[0].position.y, landerSystem.particles[0].position.z);

			TreeNode node;
			node.box = Box(glm::vec3(-1000, -1000, -1000), glm::vec3(-1000, -1000, -1000));
			if (tree.intersect(lander.getPosition(), glm::vec3(0, -1, 0), tree.root, &node))
				dist = glm::length(glm::vec3(lander.getPosition().x, lander.getPosition().y, lander.getPosition().z) - node.box.center());
			else
				dist = 99999;
			exhaust.setPosition(landerSystem.particles[0].position);

			trackCam.lookAt(landerSystem.particles[0].position);
			frontCam.setPosition(landerSystem.particles[0].position);
			bottomCam.setPosition(landerSystem.particles[0].position);
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw()
{
	if (!bStarted)
	{
		ofSetColor(ofColor::white);
		ofDrawBitmapString("WASD to move, space to rise", ofGetWindowWidth() / 2 - 108, ofGetHeight() / 2 - 22);
		ofDrawBitmapString("F1-F4 to change camera, double click to reset easy cam", ofGetWindowWidth() / 2 - 220, ofGetHeight() / 2 - 11);
		ofDrawBitmapString("Try to land in the circle", ofGetWindowWidth() / 2 - 100, ofGetHeight() / 2);
		ofDrawBitmapString("Press Space to start", ofGetWindowWidth()/2 - 80, ofGetHeight() / 2 + 11);
	}
	else if (bEnded)
	{
		ofSetColor(ofColor::white);
		ofDrawBitmapString(message, ofGetWindowWidth() / 2 - ((message.length()*8)/2), ofGetHeight() / 2);
		ofDrawBitmapString("Press r to restart", ofGetWindowWidth() / 2 - 72, ofGetHeight() / 2 + 11);
	}
	else
	{
		loadVbo();

		ofEnableDepthTest();
		ofEnableLighting(); 

		theCam->begin();
		ofPushMatrix();

		mars.drawFaces();
		ofSetColor(ofColor(200, 200, 200, 30));
		ofDrawCylinder(landingPosition, landingRadius, .75);

		if (bRoverLoaded)
		{
			lander.drawFaces();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));

		if (bDrawTree)
		{
			ofColor current = ofGetGLRenderer()->getStyle().color;
			bool fill = ofGetFill();
			ofNoFill();
			ofSetColor(ofColor::blue);
			tree.drawLeafNodes(tree.root);
			if (fill)
				ofFill();
			ofSetColor(current);
		}

		glDepthMask(GL_FALSE);
		ofSetColor(255, 230, 230);

		// this makes everything look glowy :) 
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		ofEnablePointSprites();

		// begin drawing in the camera
		shader.begin();

		// draw particle emitter here..
		particleTex.bind();
		vbo.draw(GL_POINTS, 0, (int)exhaust.sys->particles.size());
		particleTex.unbind();

		//  end drawing in the camera
		// 
		shader.end();

		ofDisablePointSprites();
		ofDisableBlendMode();
		glDepthMask(GL_TRUE);
		ofEnableAlphaBlending();

		theCam->end();

		ofDisableDepthTest();
		ofDisableLighting();

		string str;
		str += "Height: " + std::to_string(dist);
		ofSetColor(ofColor::white);
		ofDrawBitmapString(str, ofGetWindowWidth() - 170, 15);
		str = "Score: " + std::to_string(score);
		ofDrawBitmapString(str, ofGetWindowWidth() - 170, 30);

		if (bShowGui)
			gui.draw();
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	switch (key)
	{
	case ' ':
		if (!bStarted)
			bStarted = true;
		bSpacePressed = true;
		exhaust.started = true;
		if(!exhaustPlayer.isPlaying())
			exhaustPlayer.play();
		break;
	case 'W':
	case'w':
		bWPressed = true;
		break;
	case 'A':
	case'a':
		bAPressed = true;
		break;
	case 'S':
	case's':
		bSPressed = true;
		break;
	case 'D':
	case'd':
		bDPressed = true;
		break;
	case 'R':
	case 'r':
		if (bEnded)
			restart();
		break;
	case OF_KEY_F1:
		theCam = &cam;
		break;
	case OF_KEY_F2:
		theCam = &trackCam;
		break;
	case OF_KEY_F3:
		theCam = &frontCam;
		break;
	case OF_KEY_F4:
		theCam = &bottomCam;
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
	switch (key)
	{
	case ' ':
		bSpacePressed = false;
		exhaust.started = false;
		exhaustPlayer.stop();
		break;
	case 'W':
	case'w':
		bWPressed = false;
		break;
	case 'A':
	case'a':
		bAPressed = false;
		break;
	case 'S':
	case's':
		bSPressed = false;
		break;
	case 'D':
	case'd':
		bDPressed = false;
		break;
	case '`':
		bShowGui = !bShowGui;
		break;
	case 'b':
	case'B':
		tree.create(mars.getMesh(0), numLevels);
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
	bDragging = true;
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
	if (bDragging)
		bDragging = false;
	else
	{
		bDragging = false;
		glm::vec3 dir = glm::normalize(cam.screenToWorld(glm::vec3(ofGetMouseX(), ofGetMouseY(), 0)) - cam.getPosition());
	}
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::drawAxis(glm::vec3 location) 
{

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));


	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}

void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{ 5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
}

void ofApp::restart()
{
	landerSystem.particles[0].position = startingPosition;
	landerSystem.particles[0].velocity = glm::vec3(0, 0, 0);
	landerSystem.particles[0].forces = glm::vec3(0, 0, 0);

	theCam = &cam;

	bSpacePressed = false;
	bWPressed = false;
	bAPressed = false;
	bSPressed = false;
	bDPressed = false;

	string message = "";

	bStarted = true;
	bEnded = false;
}