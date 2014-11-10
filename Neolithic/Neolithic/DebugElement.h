#pragma once
#include "DebugSlider.h"
#include <QtGui\qboxlayout.h>
#include <glm\glm.hpp>
#include <Qt\qwidget.h>
#include <QtGui\qcheckbox.h>

using glm::vec3;

class DebugElement : public QVBoxLayout
{
	Q_OBJECT

	float* floatRef;
	vec3* vecRef;
	bool* boolRef;
	QWidget* control;
public slots:
	void changeReference();
public:
	DebugElement(char* label, float* ref,float min,float max,float init);
	DebugElement(char* label, float* ref);
	DebugElement(char* label, vec3* ref,vec3 init);
	DebugElement(char* label, bool* ref,bool init);
	void setHidden(bool currentState);
};

