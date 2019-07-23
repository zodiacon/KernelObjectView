#include "stdafx.h"
#include "ObjectManager.h"
#include <assert.h>

#pragma comment(lib, "ntdll")

ObjectManager::ObjectManager() {
}

ObjectManager::~ObjectManager() {
}

int ObjectManager::EnumObjectTypes() {
	ULONG len = 1 << 17;
	auto buffer = std::make_unique<BYTE[]>(len);
	if (!NT_SUCCESS(NT::NtQueryObject(nullptr, NT::ObjectTypesInformation, buffer.get(), len, &len)))
		return 0;

	auto p = reinterpret_cast<NT::OBJECT_TYPES_INFORMATION*>(buffer.get());
	bool empty = _types.empty();

	auto count = p->NumberOfTypes;
	if (empty) {
		_types.reserve(count);
		_typesMap.reserve(count);
		_changes.reserve(32);
	}
	else {
		_changes.clear();
		assert(count == _types.size());
	}
	auto raw = &p->TypeInformation[0];
	_totalHandles = _totalObjects = 0;

	for (ULONG i = 0; i < count; i++) {
		auto type = empty ? std::make_shared<ObjectType>() : _typesMap[raw->TypeIndex];
		if (empty) {
			type->GenericMapping = raw->GenericMapping;
			type->TypeIndex = raw->TypeIndex;
			type->DefaultNonPagedPoolCharge = raw->DefaultNonPagedPoolCharge;
			type->DefaultPagedPoolCharge = raw->DefaultPagedPoolCharge;
			type->TypeName = CString(raw->TypeName.Buffer, raw->TypeName.Length / 2);
			type->PoolType = (PoolType)raw->PoolType;
			type->DefaultNonPagedPoolCharge = raw->DefaultNonPagedPoolCharge;
			type->DefaultPagedPoolCharge = raw->DefaultPagedPoolCharge;
			type->ValidAccessMask = raw->ValidAccessMask;
		}
		else {
			if (type->TotalNumberOfHandles != raw->TotalNumberOfHandles)
				_changes.push_back({ type, ChangeType::TotalHandles, (int32_t)raw->TotalNumberOfHandles - (int32_t)type->TotalNumberOfHandles });
			if (type->TotalNumberOfObjects != raw->TotalNumberOfObjects)
				_changes.push_back({ type, ChangeType::TotalObjects, (int32_t)raw->TotalNumberOfObjects - (int32_t)type->TotalNumberOfObjects });
			if (type->HighWaterNumberOfHandles != raw->HighWaterNumberOfHandles)
				_changes.push_back({ type, ChangeType::PeakHandles, (int32_t)raw->HighWaterNumberOfHandles - (int32_t)type->HighWaterNumberOfHandles });
			if (type->HighWaterNumberOfObjects != raw->HighWaterNumberOfObjects)
				_changes.push_back({ type, ChangeType::PeakObjects, (int32_t)raw->HighWaterNumberOfObjects - (int32_t)type->HighWaterNumberOfObjects });
		}

		type->TotalNumberOfHandles = raw->TotalNumberOfHandles;
		type->TotalNumberOfObjects = raw->TotalNumberOfObjects;
		type->TotalNonPagedPoolUsage = raw->TotalNonPagedPoolUsage;
		type->TotalPagedPoolUsage = raw->TotalPagedPoolUsage;
		type->HighWaterNumberOfObjects = raw->HighWaterNumberOfObjects;
		type->HighWaterNumberOfHandles = raw->HighWaterNumberOfHandles;
		type->TotalNamePoolUsage = raw->TotalNamePoolUsage;

		_totalObjects += raw->TotalNumberOfObjects;
		_totalHandles += raw->TotalNumberOfHandles;

		if (empty) {
			_types.emplace_back(type);
			_typesMap.insert({ type->TypeIndex, type });
		}

		auto temp = (BYTE*)raw + sizeof(NT::OBJECT_TYPE_INFORMATION) + raw->TypeName.MaximumLength;
		temp += sizeof(PVOID) - 1;
		raw = reinterpret_cast<NT::OBJECT_TYPE_INFORMATION*>((ULONG_PTR)temp / sizeof(PVOID) * sizeof(PVOID));
	}
	return static_cast<int>(_types.size());
}

const std::vector<std::shared_ptr<ObjectType>>& ObjectManager::GetObjectTypes() const {
	return _types;
}

const std::vector<ObjectManager::Change>& ObjectManager::GetChanges() const {
	return _changes;
}

int64_t ObjectManager::GetTotalObjects() const {
	return _totalObjects;
}

int64_t ObjectManager::GetTotalHandles() const {
	return _totalHandles;
}
