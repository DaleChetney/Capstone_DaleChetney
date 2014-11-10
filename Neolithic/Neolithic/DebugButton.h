#pragma once
#include <Qt\qobject.h>
#include <Qt\qpushbutton.h>
#include "FastDelegate.h"
class DebugButton : public QObject
{
	Q_OBJECT
public:
	QPushButton* button;
	fastdelegate::FastDelegate0<> delegate;
private slots:
	void buttonClicked();
};