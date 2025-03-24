#pragma once

/**
 * Like FRefCountedObject, but internal ref count is thread safe
 */
class ThreadSafeRefCountedObject
{
public:
    __forceinline bool IsValid() const
    {
        return true;
    }
};


/** A virtual interface for ref counted objects to implement. */
class IRefCountedObject
{
public:
	virtual ~IRefCountedObject() { }
	virtual uint32 AddRef() = 0;
	virtual uint32 Release() = 0;
	virtual uint32 GetRefCount() = 0;
};