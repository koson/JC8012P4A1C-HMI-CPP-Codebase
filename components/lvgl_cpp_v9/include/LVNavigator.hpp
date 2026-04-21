/**
 * @file LVNavigator.hpp
 * @brief Navigation stack management for views
 * 
 * Phase 8.3: Core Framework - Navigation System
 * Provides:
 * - View stack management (push/pop)
 * - Navigation history
 * - Back button support
 * - View lifecycle management
 * - Transition animations (future)
 */

#ifndef LV_NAVIGATOR_HPP
#define LV_NAVIGATOR_HPP

#include "LVView.hpp"
#include <vector>
#include <memory>

/**
 * @class LVNavigator
 * @brief Manages view navigation stack (singleton)
 * 
 * Provides iOS/Android-style view navigation with push/pop operations.
 * Manages view lifecycle and maintains navigation history.
 * 
 * Usage:
 * @code
 * // Get navigator instance
 * LVNavigator& nav = LVNavigator::getInstance();
 * 
 * // Push new view
 * MyView* view = new MyView();
 * nav.push(view);
 * 
 * // Go back
 * nav.pop();
 * 
 * // Navigate to root
 * nav.popToRoot();
 * @endcode
 */
class LVNavigator {
public:
    /**
     * @brief Get navigator singleton instance
     * @return Reference to navigator
     */
    static LVNavigator& getInstance();
    
    // === Navigation Operations ===
    
    /**
     * @brief Push view onto navigation stack
     * @param view View to push (takes ownership)
     * @param animated Enable transition animation (future)
     * @return true if successful
     */
    bool push(LVView* view, bool animated = true);
    
    /**
     * @brief Pop current view from stack
     * @param animated Enable transition animation (future)
     * @return true if successful (false if at root)
     */
    bool pop(bool animated = true);
    
    /**
     * @brief Pop to specific view in stack
     * @param view View to pop to
     * @param animated Enable transition animation (future)
     * @return true if successful
     */
    bool popTo(LVView* view, bool animated = true);
    
    /**
     * @brief Pop to root view (clear entire stack except root)
     * @param animated Enable transition animation (future)
     * @return true if successful
     */
    bool popToRoot(bool animated = true);
    
    /**
     * @brief Replace current view with new view
     * @param view New view (takes ownership)
     * @param animated Enable transition animation (future)
     * @return true if successful
     */
    bool replace(LVView* view, bool animated = true);
    
    // === Stack Queries ===
    
    /**
     * @brief Get current (top) view
     * @return Pointer to current view or nullptr
     */
    LVView* getCurrentView() const;
    
    /**
     * @brief Get root view
     * @return Pointer to root view or nullptr
     */
    LVView* getRootView() const;
    
    /**
     * @brief Get number of views in stack
     * @return Stack depth
     */
    size_t getStackDepth() const { return m_viewStack.size(); }
    
    /**
     * @brief Check if navigator can go back
     * @return true if stack depth > 1
     */
    bool canGoBack() const { return m_viewStack.size() > 1; }
    
    /**
     * @brief Check if view is in stack
     * @param view View to check
     * @return true if in stack
     */
    bool isInStack(LVView* view) const;
    
    // === Stack Management ===
    
    /**
     * @brief Clear entire navigation stack
     * Destroys all views
     */
    void clearStack();
    
    /**
     * @brief Set root view (clears stack first)
     * @param view Root view (takes ownership)
     * @return true if successful
     */
    bool setRootView(LVView* view);
    
private:
    // Singleton
    LVNavigator();
    ~LVNavigator();
    LVNavigator(const LVNavigator&) = delete;
    LVNavigator& operator=(const LVNavigator&) = delete;
    
    // View stack (owns views)
    std::vector<std::unique_ptr<LVView>> m_viewStack;
    
    /**
     * @brief Activate view (show on screen)
     * @param view View to activate
     */
    void activateView(LVView* view);
    
    /**
     * @brief Deactivate view (hide from screen)
     * @param view View to deactivate
     */
    void deactivateView(LVView* view);
    
    /**
     * @brief Remove view from stack
     * @param index Stack index to remove
     */
    void removeViewAt(size_t index);
};

#endif // LV_NAVIGATOR_HPP
