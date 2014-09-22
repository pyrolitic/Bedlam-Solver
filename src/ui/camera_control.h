#ifndef CAMERA_CONTROL_H
#define CAMERA_CONTROL_H

#include <cmath>

#include "../maths/vec.h"

#include "ui_elem.h"

//this is an invisible element that acts as a kind of trackpad, behind the vsible UI
class CameraControl : public UIElem{
public:
	//pleasant limits, radians
	#define CAMERA_MIN_DIVE -2.3f
	#define CAMERA_MAX_DIVE -0.3f
	#define CAMERA_DIST_MIN 1.5f

	CameraControl() : UIElem(){
		resetCamera();
	}

	~CameraControl(){
	}

	void update(){
		//position child elements as UIElem would
		UIElem::update();

		//TODO: maybe smoother camera transitions
	}

	UIElem* collides(ivec2 at) const{
		UIElem* above = UIElem::collides(at);
		if (above){
			return above;
		}
		else{
			return (UIElem*)this; //only difference from UIElem::collides
		}
	}

	void resetCamera(){
		turn = 0.0f;
		dive = (CAMERA_MIN_DIVE + CAMERA_MAX_DIVE) / 2.0f;
		dist = 15.0f;
	}

	vec3 getEyePosition(){
		vec3 eye;

		//dive
		eye.z = 1.0f * sin(dive);
		eye.y = 1.0f * cos(dive);

		//turn
		eye.x = -eye.z * sin(turn);
		eye.z =  eye.z * cos(turn);

		//distance
		eye *= dist;

		return eye;
	}

	vec3 getTurnDiveDist(){
		return vec3(turn, dive, dist);
	}

protected:
	void privateOnMouseDown(ivec2 at, int button) {
		if (button == GLUT_RIGHT_BUTTON){
			turnDragBase = turn;
			diveDragBase = dive;
			//distDragBase = dist;
		}
	}

	void privateOnMouseUp(ivec2 at, int button) {
		if (button == GLUT_RIGHT_BUTTON){
			//drag end, angle wrap
			turn = fmodf(turn, M_PI * 2);
		}
	}
	
	void privateOnMouseDrag(ivec2 to, ivec2 base, int button) {
		if (button == GLUT_RIGHT_BUTTON){
			ivec2 delta = to - base;

			turn = turnDragBase + delta.x * 0.01f;
			dive = diveDragBase + delta.y * 0.01f;

			if (dive < CAMERA_MIN_DIVE) dive = CAMERA_MIN_DIVE;
			if (dive > CAMERA_MAX_DIVE) dive = CAMERA_MAX_DIVE;
		}
	}

	bool privateOnWheel(ivec2 at, int delta){
		//zoom
		dist += delta * log10(dist);
		if (dist < CAMERA_DIST_MIN) dist = CAMERA_DIST_MIN;

		//dist += delta * 3;
		return true;
	}

private:
	//mouselook
	float turn, turnDragBase; //radians, about the center, angle wrapped
	float dive, diveDragBase; //radians
	float dist; //from the center
};

#endif