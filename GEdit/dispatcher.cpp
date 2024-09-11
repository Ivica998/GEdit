#include "dispatcher.h"
#include "utils.h"
#include "control.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, TraceData& td)
{
	return os;
}

TraceData Dispatcher::s_TraceInfo;

std::vector<Invokable*> Dispatcher::InvokeQ;
std::vector<Invokable*> Dispatcher::UpdateQ;
std::vector<MouseClickData*> Dispatcher::MouseClickQ;
std::vector<MouseClickData*> Dispatcher::MousePressQ;
std::vector<MouseClickData*> Dispatcher::MouseReleaseQ;
std::vector<ScrollData*> Dispatcher::ScrollQ;

Event_E Dispatcher::e_Update = Event_E();


void Dispatcher::InvokeMouseClickQ()
{
	for (auto iter = MouseClickQ.begin(); iter != MouseClickQ.end(); ++iter)
	{
		auto& item = (**iter);
		item.Invoke();
	}
	MouseClickQ.clear();
}
void Dispatcher::InvokeMousePressQ()
{
	for (auto iter = MousePressQ.begin(); iter != MousePressQ.end(); ++iter)
	{
		auto& item = (**iter);
		item.Invoke();
	}
}
void Dispatcher::InvokeMouseReleaseQ()
{
	for (auto iter = MouseReleaseQ.begin(); iter != MouseReleaseQ.end(); ++iter)
	{
		auto& item = (**iter);
		item.Invoke();
	}
	MousePressQ.clear();
	MouseReleaseQ.clear();
}
void Dispatcher::InvokeScrollQ()
{
	for (auto iter = ScrollQ.begin(); iter != ScrollQ.end(); ++iter)
	{
		auto& item = (**iter);
		item.Invoke();
	}
	ScrollQ.clear();
}
void Dispatcher::InvokeInvokeQ()
{
	for (auto iter = InvokeQ.begin(); iter != InvokeQ.end(); ++iter)
	{
		auto& item = (**iter);
		item.Invoke();
	}
	InvokeQ.clear();
}

void Dispatcher::InvokeUpdateQ()
{
	for (auto iter = UpdateQ.begin(); iter != UpdateQ.end(); ++iter)
	{
		auto& item = (**iter);
		item.Invoke();
	}
}


void Dispatcher::Perform()
{
	InvokeInvokeQ();
	e_Update.Invoke();
	InvokeScrollQ();
	InvokeMouseClickQ();
	InvokeMousePressQ();
}


void Dispatcher::TraceIn(DrawObj2D* this_cd)
{
	s_TraceInfo.count++;
	s_TraceInfo.Parent = s_TraceInfo.Current;
	s_TraceInfo.Current = this_cd;
	s_TraceInfo.Width *= this_cd->m_Bounds.SpreadX();
	s_TraceInfo.Height *= this_cd->m_Bounds.SpreadY();
	s_TraceInfo.Path.push_back(this_cd);
}

void Dispatcher::TraceOut()
{
	s_TraceInfo.count++;
	DrawObj2D* cur = s_TraceInfo.Current;
	s_TraceInfo.Width /= cur->m_Bounds.SpreadX();
	s_TraceInfo.Height /= cur->m_Bounds.SpreadY();
	s_TraceInfo.Current = s_TraceInfo.Parent;
	if(s_TraceInfo.Path.size() >= 3)
		s_TraceInfo.Parent = s_TraceInfo.Path.rbegin()[2];
	s_TraceInfo.Path.pop_back();
}

