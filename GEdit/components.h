/*
#pragma once
#include "utils.h"
#include "control.h"


class CompStorBase;
class Component2Manager {
public:
	static std::vector<CompStorBase*> Component2Types;
	static void Register(CompStorBase* newComponent2Type) {
		Component2Types.push_back(newComponent2Type);
	}
private:
	Component2Manager() {};
};

class CompStorBase {
	virtual void OnAwake() = 0;
	virtual void OnStart() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnFixedUpdate() = 0;
};
template<typename T>
class Component2Storage : public CompStorBase {
public:
	static Component2Storage<T> instance;
	friend class constructor;
	struct constructor {
	public:
		constructor() {
			Component2Manager::Register(this);
			instance = Component2Storage();
		}
		~constructor() {
		}
	};
	static constructor con;
	std::map<void*, T*> Registered;
	static void Register(void* owner, T* obj) {
		Registered.insert(owner, obj);
	}
	static void Unregister(void* owner, T* obj) {
		Registered.erase(owner);
	}

	void OnAwake() override {
		for (auto item : Registered) {
			item->second.OnAwake();
		}
	}
	void OnStart() override {
		for (auto item : Registered) {
			item->second.OnStart();
		}
	}
	void OnUpdate() override {
		for (auto item : Registered) {
			item->second.OnUpdate();
		}
	}
	void OnFixedUpdate() override {
		for (auto item : Registered) {
			item->second.OnFixedUpdate();
		}
	}
private:
	Component2Storage() {};
};
template<typename T>
Component2Storage<T>::constructor Component2Storage<T>::con;

class Component2Base {
public:
	virtual void OnAwake() = 0;
	virtual void OnStart() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnFixedUpdate() = 0;
};
class Component2 {
public:
	public:
	template<typename T>
	Component2(Control3D* owner) {
		Component2Storage<T>::Register(owner, this);
	}
		virtual void OnAwake() = 0;
		virtual void OnStart() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnFixedUpdate() = 0;
};*/