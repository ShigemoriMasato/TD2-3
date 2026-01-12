#include "Collider.h"
#include "ColliderManager.h"

ColliderManager* Collider::colliderManager_ = nullptr;

uint32_t operator|(CollTag a, CollTag b) {
    return 0;
}

uint32_t operator|(uint32_t a, CollTag b) {
    return 0;
}

uint32_t operator&(uint32_t a, CollTag b) {
    return 0;
}

uint32_t operator~(CollTag a) {
    return 0;
}

void Collider::Initialize() {
	isActive_ = true;
	colliderManager_->AddCollider(this);

}

void Collider::SetColliderConfig(const CollConfig& circle) {
}
