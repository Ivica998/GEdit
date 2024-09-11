#include "binding_manager.h"

std::map <DepObjB*, std::vector<DepObjB*>> BindingManager::Map;
std::unordered_set<DepObjB*> BindingManager::collected;

void DepObjB::BeginNotify() {
	BindingManager::collected.clear();
	BindingManager::Notify(this);
	BindingManager::Process(this);
}

void DepObjB::Notify() {
	BindingManager::Notify(this);
}
