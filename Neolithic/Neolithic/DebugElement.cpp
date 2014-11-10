#include "DebugElement.h"

DebugElement::DebugElement(char* label, float* ref)
{				
	floatRef=ref;
	boolRef=NULL;
	vecRef=NULL;
	addWidget(control);
	connect(control, SIGNAL(valueChanged(float)),this,SLOT(changeReference()));		   
}	

DebugElement::DebugElement(char* label, float* ref,float min,float max,float init)
{				
	floatRef=ref;
	boolRef=NULL;
	vecRef=NULL;
	control = new DebugSlider(label,min,max,true,100);
	reinterpret_cast<DebugSlider*>(control)->setValue(init);
	*floatRef=init;
	addWidget(control);
	connect(control, SIGNAL(valueChanged(float)),this,SLOT(changeReference()));
}						   
DebugElement::DebugElement(char* label, vec3* ref,vec3 init)
{						   
	floatRef=NULL;
	boolRef=NULL;
	vecRef=ref;			   
}						   
DebugElement::DebugElement(char* label, bool* ref,bool init)
{
	floatRef=NULL;
	boolRef=ref;
	vecRef=NULL;
	control=new QCheckBox(label);
	reinterpret_cast<QCheckBox*>(control)->setChecked(init);
	*boolRef=init;
	addWidget(control);
	connect(control, SIGNAL(stateChanged(int)),this,SLOT(changeReference()));
}
void DebugElement::setHidden(bool currentState)
{
	control->setHidden(currentState);
}

void DebugElement::changeReference()
{
	if(floatRef!=NULL) 
		*floatRef=reinterpret_cast<DebugSlider*>(control)->value();
	else if(boolRef!=NULL) 
		*boolRef=((QCheckBox*)control)->isChecked();
	else if(vecRef!=NULL)
	{
		
	}
}