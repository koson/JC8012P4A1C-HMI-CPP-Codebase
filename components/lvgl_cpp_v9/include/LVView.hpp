/**
 * @file LVView.hpp
 * @brief View base class for UI components with automatic cleanup
 * 
 * Phase 8.2: Core Framework - Document/View Architecture
 * Provides:
 * - View base class (similar to MFC CView)
 * - Automatic resource cleanup
 * - Document association
 * - Screen management integration
 * - Lifecycle hooks
 */

#ifndef LV_VIEW_HPP
#define LV_VIEW_HPP

#include "LVScreen.hpp"
#include "LVDocument.hpp"
#include <functional>

/**
 * @class LVView
 * @brief Base class for views with document association
 * 
 * Document/View pattern implementation. Views display and interact
 * with document data. Provides automatic cleanup and lifecycle management.
 * 
 * Usage:
 * @code
 * class MyView : public LVView {
 * public:
 *     MyView(MyDocument* doc) : LVView("MyView") {
 *         setDocument(doc);
 *     }
 *     
 *     void onInitialUpdate() override {
 *         // Create UI from document data
 *         createUI();
 *     }
 *     
 *     void onUpdate(LVDocument* doc, void* hint) override {
 *         // Refresh UI when document changes
 *         updateDisplay();
 *     }
 * };
 * @endcode
 */
class LVView : public LVScreen {
public:
    /**
     * @brief Constructor
     * @param className Class name for RTTI system
     */
    explicit LVView(const char* className = "LVView");
    
    /**
     * @brief Virtual destructor
     */
    virtual ~LVView();
    
    // === Document Association ===
    
    /**
     * @brief Associate document with view
     * @param doc Pointer to document (can be nullptr)
     */
    virtual void setDocument(LVDocument* doc);
    
    /**
     * @brief Get associated document
     * @return Pointer to document or nullptr
     */
    LVDocument* getDocument() const { return m_document; }
    
    // === View Lifecycle ===
    
    /**
     * @brief Called when view is first created
     * Override to create UI elements
     */
    virtual void onInitialUpdate();
    
    /**
     * @brief Called when document data changes
     * @param doc Pointer to document that changed
     * @param hint Optional hint about what changed (user-defined)
     */
    virtual void onUpdate(LVDocument* doc, void* hint = nullptr);
    
    /**
     * @brief Called when view is activated (shown)
     */
    virtual void onActivate();
    
    /**
     * @brief Called when view is deactivated (hidden)
     */
    virtual void onDeactivate();
    
    /**
     * @brief Called before view is destroyed
     * @return true to allow destruction, false to cancel
     */
    virtual bool onDestroy();
    
    // === Update Triggers ===
    
    /**
     * @brief Notify view that document data changed
     * @param hint Optional hint about what changed
     */
    void updateView(void* hint = nullptr);
    
    /**
     * @brief Mark document as modified from view
     * @param modified Modified state (default true)
     */
    void setDocumentModified(bool modified = true);
    
    // === View State ===
    
    /**
     * @brief Check if view is active (currently shown)
     * @return true if active
     */
    bool isActive() const { return m_active; }
    
    /**
     * @brief Get the class name
     */
    const char* getClassName() const { return m_className; }

protected:
    /**
     * @brief Get typed document pointer
     * @tparam T Document type
     * @return Typed document pointer or nullptr
     */
    template<typename T>
    T* getDocumentAs() const {
        return static_cast<T*>(m_document);
    }
    
    /**
     * @brief Create default UI
     * Called by onInitialUpdate if not overridden
     */
    virtual void createDefaultUI();
    
private:
    LVDocument* m_document;  ///< Associated document
    bool m_active;           ///< Active state
    bool m_initialized;      ///< Initialization flag
    const char* m_className; ///< Class name
    
    friend class LVNavigator;  // Allow navigator to manage view lifecycle
};

#endif // LV_VIEW_HPP
