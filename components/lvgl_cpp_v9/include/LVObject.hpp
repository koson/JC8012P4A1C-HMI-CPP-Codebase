#pragma once
#include <cstring>

/**
 * @file LVObject.hpp
 * @brief Base class for all LVGL C++ wrapper objects (similar to MFC CObject)
 * 
 * LVObject provides:
 * - Runtime Type Information (RTTI)
 * - Common base for all framework classes
 * - Type checking and casting
 * - Foundation for future serialization support
 * 
 * Hierarchy:
 * LVObject (base)
 * ├── LVWidget (all UI widgets)
 * ├── LVView (views and screens)
 * ├── LVDocument (data models)
 * ├── LVStorage (file system)
 * ├── LVApplication (app singleton)
 * └── Other framework classes
 * 
 * Example:
 * @code
 * class MyClass : public LVObject {
 * public:
 *     const char* getClassName() const override { return "MyClass"; }
 *     bool isKindOf(const char* className) const override {
 *         if (strcmp(className, "MyClass") == 0) return true;
 *         return LVObject::isKindOf(className);
 *     }
 * };
 * 
 * MyClass obj;
 * if (obj.isKindOf("LVObject")) {
 *     // Type check passed
 * }
 * @endcode
 */
class LVObject {
protected:
    /**
     * @brief Protected destructor - prevents direct deletion of base class
     * Derived classes should have virtual destructors
     */
    virtual ~LVObject() = default;
    
public:
    /**
     * @brief Get the class name (RTTI)
     * 
     * Each derived class must override this to return its class name.
     * This enables runtime type identification without C++ RTTI overhead.
     * 
     * @return const char* Class name string
     */
    virtual const char* getClassName() const { return "LVObject"; }
    
    /**
     * @brief Check if object is of specified type or derived from it
     * 
     * This provides type checking similar to MFC's IsKindOf.
     * Derived classes should override and chain to parent.
     * 
     * @param className Name of the class to check
     * @return true if object is of specified type or derived from it
     * @return false otherwise
     * 
     * Example:
     * @code
     * bool MyClass::isKindOf(const char* className) const {
     *     if (strcmp(className, "MyClass") == 0) return true;
     *     return LVObject::isKindOf(className);
     * }
     * @endcode
     */
    virtual bool isKindOf(const char* className) const {
        return (strcmp(className, "LVObject") == 0);
    }
    
    /**
     * @brief Check if two objects are of the same type
     * 
     * @param other Object to compare with
     * @return true if both objects have the same class name
     */
    bool isSameType(const LVObject& other) const {
        return (strcmp(getClassName(), other.getClassName()) == 0);
    }
    
    // Future: Serialization support (Phase 10)
    // virtual void serialize(Archive& ar);
    // virtual void deserialize(Archive& ar);
};

/**
 * @brief Helper macro for RTTI implementation in derived classes
 * 
 * Use this macro in derived class declaration to implement getClassName() and isKindOf()
 * 
 * Example:
 * @code
 * class MyView : public LVView {
 *     DECLARE_LVOBJECT_CLASS(MyView, LVView)
 * public:
 *     // ... class implementation
 * };
 * @endcode
 */
#define DECLARE_LVOBJECT_CLASS(ClassName, BaseClass) \
public: \
    const char* getClassName() const override { return #ClassName; } \
    bool isKindOf(const char* className) const override { \
        if (strcmp(className, #ClassName) == 0) return true; \
        return BaseClass::isKindOf(className); \
    }
