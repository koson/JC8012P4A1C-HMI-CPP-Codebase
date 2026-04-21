/**
 * @file LVApplication.hpp
 * @brief Application framework class
 * 
 * Phase 8.4: Core Framework - Application Lifecycle
 * Provides:
 * - Application initialization
 * - Main loop management
 * - Display/touch setup
 * - Document/View integration
 * - Global app state
 */

#ifndef LV_APPLICATION_HPP
#define LV_APPLICATION_HPP

#include "LVNavigator.hpp"
#include "LVDocument.hpp"
#include "LVStorage.hpp"
#include <functional>

/**
 * @class LVApplication
 * @brief Main application framework (singleton)
 * 
 * Manages application lifecycle, initialization, and global state.
 * Integrates display, navigator, and document management.
 * 
 * Usage:
 * @code
 * // In app_main()
 * LVApplication& app = LVApplication::getInstance();
 * app.initialize();
 * 
 * // Create root view
 * MyView* rootView = new MyView();
 * app.setRootView(rootView);
 * 
 * // App runs automatically via LVGL
 * @endcode
 */
class LVApplication {
public:
    /**
     * @brief Get application singleton instance
     * @return Reference to application
     */
    static LVApplication& getInstance();
    
    // === Initialization ===
    
    /**
     * @brief Initialize application
     * Sets up display, touch, navigator
     * @return true if successful
     */
    bool initialize();
    
    /**
     * @brief Check if application is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return m_initialized; }
    
    // === View Management ===
    
    /**
     * @brief Set root view
     * @param view Root view (takes ownership)
     * @return true if successful
     */
    bool setRootView(LVView* view);
    
    /**
     * @brief Get navigator
     * @return Reference to navigator
     */
    LVNavigator& getNavigator() { return LVNavigator::getInstance(); }
    
    // === Storage Management ===
    
    /**
     * @brief Get storage instance
     * @return Reference to storage
     */
    LVStorage& getStorage() { return LVStorage::getInstance(); }
    
    /**
     * @brief Mount SD card storage
     * @param mountPoint Mount point (default: /sdcard)
     * @return true if successful
     */
    bool mountSDCard(const char* mountPoint = "/sdcard");
    
    /**
     * @brief Unmount storage
     * @return true if successful
     */
    bool unmountStorage();
    
    // === Application State ===
    
    /**
     * @brief Run application (called automatically by LVGL)
     * Override to add custom app logic
     */
    virtual void run();
    
    /**
     * @brief Terminate application
     */
    virtual void terminate();
    
    /**
     * @brief Check if application is running
     * @return true if running
     */
    bool isRunning() const { return m_running; }
    
    // === Callbacks ===
    
    /**
     * @brief Set idle callback (called periodically)
     * @param callback Idle callback function
     * @param userData User data passed to callback
     */
    void setIdleCallback(std::function<void(void*)> callback, void* userData = nullptr);
    
protected:
    /**
     * @brief Called during initialization
     * Override to add custom init logic
     * @return true if successful
     */
    virtual bool onInitialize();
    
    /**
     * @brief Called when application starts running
     */
    virtual void onStart();
    
    /**
     * @brief Called when application terminates
     */
    virtual void onTerminate();
    
private:
    // Singleton
    LVApplication();
    ~LVApplication();
    LVApplication(const LVApplication&) = delete;
    LVApplication& operator=(const LVApplication&) = delete;
    
    bool m_initialized;  ///< Initialization flag
    bool m_running;      ///< Running flag
    
    // Callbacks
    std::function<void(void*)> m_idleCallback;
    void* m_idleCallbackUserData;
};

#endif // LV_APPLICATION_HPP
