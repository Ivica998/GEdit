#pragma once

#include "my_events.h"
#include <iostream>

class Control;
class DrawObj2D;


struct TraceData
{
	std::vector<DrawObj2D*> Path;
	DrawObj2D* Parent = nullptr;
	DrawObj2D* Current = nullptr;
	float Width = 0;
	float Height = 0;
	int count = 0;

	friend std::ostream& operator<<(std::ostream& os, TraceData& td);
	void Clear()
	{
		Path.clear();
		count = 0;
		Parent = nullptr;
		Current = nullptr;
	}
};
	

class Dispatcher
{
public:
	static TraceData s_TraceInfo;


	static std::vector<Invokable*> InvokeQ;
	static void InvokeInvokeQ();
	static std::vector<Invokable*> UpdateQ;
	static void InvokeUpdateQ();

	static std::vector<MouseClickData*> MouseClickQ;
	static void InvokeMouseClickQ();
	static std::vector<MouseClickData*> MousePressQ;
	static void InvokeMousePressQ();
	static std::vector<MouseClickData*> MouseReleaseQ;
	static void InvokeMouseReleaseQ();

	static std::vector<ScrollData*> ScrollQ;
	static void InvokeScrollQ();

	static Event_E e_Update;

	static void Perform();
	static void TraceIn(DrawObj2D* this_cd);
	static void TraceOut();


};
typedef Dispatcher dpch;

