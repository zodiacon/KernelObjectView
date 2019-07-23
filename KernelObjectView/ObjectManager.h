#pragma once

#include <winternl.h>

enum class PoolType {
	PagedPool = 1,
	NonPagedPool = 0,
	NonPagedPoolNx = 0x200,
	NonPagedPoolSessionNx = NonPagedPoolNx + 32,
	PagedPoolSessionNx = NonPagedPoolNx + 33
};

namespace NT {
	extern "C" {
		typedef struct _OBJECT_TYPE_INFORMATION {
			UNICODE_STRING TypeName;
			ULONG TotalNumberOfObjects;
			ULONG TotalNumberOfHandles;
			ULONG TotalPagedPoolUsage;
			ULONG TotalNonPagedPoolUsage;
			ULONG TotalNamePoolUsage;
			ULONG TotalHandleTableUsage;
			ULONG HighWaterNumberOfObjects;
			ULONG HighWaterNumberOfHandles;
			ULONG HighWaterPagedPoolUsage;
			ULONG HighWaterNonPagedPoolUsage;
			ULONG HighWaterNamePoolUsage;
			ULONG HighWaterHandleTableUsage;
			ULONG InvalidAttributes;
			GENERIC_MAPPING GenericMapping;
			ULONG ValidAccessMask;
			BOOLEAN SecurityRequired;
			BOOLEAN MaintainHandleCount;
			UCHAR TypeIndex; // since WINBLUE
			CHAR ReservedByte;
			ULONG PoolType;
			ULONG DefaultPagedPoolCharge;
			ULONG DefaultNonPagedPoolCharge;
		} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

		typedef enum _OBJECT_INFORMATION_CLASS {
			ObjectBasicInformation,
			ObjectNameInformation,
			ObjectTypeInformation,
			ObjectTypesInformation,
			ObjectHandleFlagInformation,
			ObjectSessionInformation,
			MaxObjectInfoClass
		} OBJECT_INFORMATION_CLASS;

		typedef struct _OBJECT_TYPES_INFORMATION {
			ULONG NumberOfTypes;
			OBJECT_TYPE_INFORMATION TypeInformation[1];
		} OBJECT_TYPES_INFORMATION, *POBJECT_TYPES_INFORMATION;

		NTSTATUS NTAPI NtQueryObject(
			_In_opt_   HANDLE Handle,
			_In_       OBJECT_INFORMATION_CLASS ObjectInformationClass,
			_Out_opt_  PVOID ObjectInformation,
			_In_       ULONG ObjectInformationLength,
			_Out_opt_  PULONG ReturnLength
		);
	}
};

struct ObjectType {
	uint32_t TotalNumberOfObjects;
	uint32_t TotalNumberOfHandles;
	uint32_t TotalPagedPoolUsage;
	uint32_t TotalNonPagedPoolUsage;
	uint32_t TotalNamePoolUsage;
	uint32_t TotalHandleTableUsage;
	uint32_t HighWaterNumberOfObjects;
	uint32_t HighWaterNumberOfHandles;
	uint32_t HighWaterPagedPoolUsage;
	uint32_t HighWaterNonPagedPoolUsage;
	uint32_t HighWaterNamePoolUsage;
	uint32_t HighWaterHandleTableUsage;
	uint32_t InvalidAttributes;
	GENERIC_MAPPING GenericMapping;
	uint32_t ValidAccessMask;
	PoolType PoolType;
	uint32_t DefaultPagedPoolCharge;
	uint32_t DefaultNonPagedPoolCharge;
	CString TypeName;
	uint8_t TypeIndex;
	bool SecurityRequired;
	bool MaintainHandleCount;
};

class ObjectManager {
public:
	ObjectManager();
	~ObjectManager();

	enum class ChangeType {
		NoChange,
		TotalHandles,
		TotalObjects,
		PeakHandles,
		PeakObjects,
	};

	using Change = std::tuple<std::shared_ptr<ObjectType>, ChangeType, int32_t>;

	int EnumObjectTypes();
	const std::vector<std::shared_ptr<ObjectType>>& GetObjectTypes() const;
	const std::vector<Change>& GetChanges() const;
	int64_t GetTotalObjects() const;
	int64_t GetTotalHandles() const;

private:
	std::vector<std::shared_ptr<ObjectType>> _types;
	std::unordered_map<uint32_t, std::shared_ptr<ObjectType>> _typesMap;
	std::vector<Change> _changes;
	int64_t _totalObjects, _totalHandles;
};

