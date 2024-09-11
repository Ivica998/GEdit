#pragma once
#include <vector>
#include <map>
#include <unordered_set>
#include "utils.h"

enum class BindingMode {
	StoD,
	DtoS,
	Both
};
/*
class DepObjB {
public:
	inline virtual void* GetValue() = 0;
	virtual void SetValue(DepObjB* src) = 0;
	void BeginNotify();
	void Notify();
};

template<typename T>
class DepObj : public DepObjB{
public:
	T* value;
	DepObj() {
		value = new T();
	}
	DepObj(T* value_p) {
		value = value_p;
	}
	inline virtual void* GetValue() { return (void*)value;}
	virtual void SetValue(DepObjB* src)  { 
		T* newValue = (T*)src->GetValue();
		if (*value != *newValue) {
			*value = *newValue;
			Notify();
		}
	}
	inline virtual T& Get() {
		return *value;
	}
	virtual void Set(T* newValue) {
		if (*value != *newValue) {
			*value = *newValue;
			BeginNotify();
		}
	}
};
*/
class DepObjB {
public:
	virtual void SetValue(DepObjB* src) = 0;
	inline virtual void* GetValue() = 0;
	virtual void* Adjust(void* val) = 0;
	void BeginNotify();
	void Notify();
};

template<typename T>
class DepObj : public DepObjB {
public:
	void* owner = nullptr;
	T* value = nullptr;
	DepObj() {}
	DepObj(void* owner_p) {
		value = new T();
		owner = owner_p;
	}
	DepObj(void* owner_p, T* value_p) {
		value = value_p;
		owner = owner_p;
	}
	inline T& Get() {
		return *value;
	}
	void Set(T newValue) {
		if (*value != newValue) {
			*value = newValue;
			OnSet(owner, newValue);
			BeginNotify();
		}
	}
	void Remap(void* owner_p, T* value_p) {
		owner = owner_p;
		value = value_p;
		BeginNotify();
	}
	virtual T* AdjustT(T* value) { return value; }
	virtual void OnSet(void* owner, T newValue) { }
private:
	inline void* GetValue() override { return value; }
	void SetValue(DepObjB* src) override {
		T* newValue = (T*)src->GetValue();	
		if (*value != *newValue)
		{
			*value = *newValue;
			OnSet(owner,*newValue);
			Notify();
		}
	}
	void* Adjust(void* val) override {
		T* newValue = AdjustT((T*)val);
		return newValue;
	}
};


class BindingManager {
public:
	static std::map <DepObjB*, std::vector<DepObjB*>> Map;
	static std::unordered_set<DepObjB*> processed;
	static std::unordered_set<DepObjB*> collected;


	static void SetBinding(DepObjB* dest, DepObjB* src, BindingMode mode = BindingMode::Both) {
		switch (mode) {
		case BindingMode::StoD:
			Map[src].push_back(dest);
			break;
		case BindingMode::DtoS:
			Map[dest].push_back(src);
			break;
		case BindingMode::Both:
			Map[src].push_back(dest);
			Map[dest].push_back(src);
			break;
		default:
			break;
		}
	}
	static void RemoveBinding(DepObjB* dest, DepObjB* src, BindingMode mode = BindingMode::Both) {
		switch (mode) {
		case BindingMode::StoD:
			utils::ErraseFromVec(Map[src], dest);
			break;
		case BindingMode::DtoS:
			utils::ErraseFromVec(Map[dest], src);
			break;
		case BindingMode::Both:
			utils::ErraseFromVec(Map[src], dest);
			utils::ErraseFromVec(Map[dest], src);
			break;
		default:
			break;
		}
	}
	static void Notify(DepObjB* src) {
		auto vec = Map[src];
		for (int i = 0; i < vec.size(); i++) {
			if (collected.count(vec[i]))
				return;
			collected.insert({ vec[i] });
			//vec[i]->SetValue(src);
		}
	}
	static void Process(DepObjB* src) {
		void* val = src->GetValue();
		for (auto obj : collected) {
			obj->Adjust(val);
		}
		for (auto obj : collected) {
			obj->SetValue(src);
		}
	}

};

typedef BindingManager bmtd;