#pragma once

#include "LVScreen.hpp"
#include <map>
#include <vector>
#include <string>

/**
 * @brief Screen navigation manager (Singleton)
 * 
 * Manages screen lifecycle and navigation with:
 * - Screen registration by name
 * - Navigation stack (push/pop)
 * - Transition effects
 * - History tracking
 * - Back navigation
 * 
 * Usage:
 * @code
 * auto& mgr = ScreenManager::getInstance();
 * mgr.registerScreen("main", mainScreen);
 * mgr.registerScreen("settings", settingsScreen);
 * mgr.navigateTo("main", LVScreen::Transition::FadeIn, 300);
 * mgr.navigateTo("settings", LVScreen::Transition::MoveLeft, 400);
 * mgr.back();  // Return to main
 * @endcode
 */
class ScreenManager {
public:
    /**
     * @brief Get singleton instance
     */
    static ScreenManager& getInstance();
    
    // Delete copy/move constructors
    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;
    ScreenManager(ScreenManager&&) = delete;
    ScreenManager& operator=(ScreenManager&&) = delete;

    // === Screen Registration ===
    
    /**
     * @brief Register a screen with a name
     * @param name Screen identifier
     * @param screen Screen object
     */
    void registerScreen(const std::string& name, LVScreen* screen);
    
    /**
     * @brief Unregister a screen
     * @param name Screen identifier
     */
    void unregisterScreen(const std::string& name);
    
    /**
     * @brief Get screen by name
     * @param name Screen identifier
     * @return Screen pointer or nullptr if not found
     */
    LVScreen* getScreen(const std::string& name) const;
    
    /**
     * @brief Check if screen exists
     * @param name Screen identifier
     */
    bool hasScreen(const std::string& name) const;

    // === Navigation ===
    
    /**
     * @brief Navigate to a screen by name (clears history)
     * @param name Screen identifier
     * @param transition Transition effect
     * @param duration Transition duration in ms
     * @return true if successful
     */
    bool navigateTo(const std::string& name, 
                    LVScreen::Transition transition = LVScreen::Transition::None,
                    uint32_t duration = 0);
    
    /**
     * @brief Push screen onto stack (adds to history)
     * @param name Screen identifier
     * @param transition Transition effect
     * @param duration Transition duration in ms
     * @return true if successful
     */
    bool push(const std::string& name,
              LVScreen::Transition transition = LVScreen::Transition::MoveLeft,
              uint32_t duration = 300);
    
    /**
     * @brief Pop current screen and return to previous (back navigation)
     * @param transition Transition effect (default: MoveRight)
     * @param duration Transition duration in ms
     * @return true if successful (false if no history)
     */
    bool back(LVScreen::Transition transition = LVScreen::Transition::MoveRight,
              uint32_t duration = 300);
    
    /**
     * @brief Pop to a specific screen in history
     * @param name Screen identifier
     * @param transition Transition effect
     * @param duration Transition duration in ms
     * @return true if successful
     */
    bool popTo(const std::string& name,
               LVScreen::Transition transition = LVScreen::Transition::MoveRight,
               uint32_t duration = 300);

    // === History Management ===
    
    /**
     * @brief Clear navigation history
     */
    void clearHistory();
    
    /**
     * @brief Get current screen name
     */
    std::string getCurrentScreenName() const;
    
    /**
     * @brief Get current screen
     */
    LVScreen* getCurrentScreen() const;
    
    /**
     * @brief Get history size
     */
    size_t getHistorySize() const;
    
    /**
     * @brief Check if can go back
     */
    bool canGoBack() const;
    
    /**
     * @brief Get screen name at history index (0 = oldest)
     */
    std::string getHistoryAt(size_t index) const;

    // === Settings ===
    
    /**
     * @brief Set default transition for push/pop
     * @param pushTrans Transition when pushing
     * @param popTrans Transition when popping
     */
    void setDefaultTransitions(LVScreen::Transition pushTrans, 
                              LVScreen::Transition popTrans);
    
    /**
     * @brief Set default transition duration
     */
    void setDefaultDuration(uint32_t ms);

private:
    ScreenManager();
    ~ScreenManager() = default;
    
    // Screen registry
    std::map<std::string, LVScreen*> screens;
    
    // Navigation history (stack of screen names)
    std::vector<std::string> history;
    
    // Current screen
    std::string current_screen;
    
    // Default settings
    LVScreen::Transition default_push_transition;
    LVScreen::Transition default_pop_transition;
    uint32_t default_duration;
};
