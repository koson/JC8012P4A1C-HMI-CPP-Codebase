#ifndef DEMO_MANAGER_H
#define DEMO_MANAGER_H

#include "lvgl.h"
#include <vector>
#include <string>
#include <functional>

/**
 * @brief Demo item structure - represents a single demo
 */
struct DemoItem {
    std::string name;
    std::string description;
    std::function<void(lv_obj_t*)> createFunction;
    
    DemoItem(const char* n, const char* d, std::function<void(lv_obj_t*)> fn)
        : name(n), description(d), createFunction(fn) {}
};

/**
 * @brief DemoManager - C++ OOP class for managing LVGL demos
 * 
 * Features:
 * - Register demos dynamically
 * - Auto-generate menu UI
 * - Handle screen navigation
 * - Easy to add new demos
 */
class DemoManager {
private:
    std::vector<DemoItem> demos;
    lv_obj_t* menu_screen;
    lv_obj_t* current_demo_screen;
    
    // Private methods
    void createMenuUI();
    void createDemoScreen(size_t index);
    static void menuEventHandler(lv_event_t* e);
    static void backButtonHandler(lv_event_t* e);
    
public:
    DemoManager();
    ~DemoManager();
    
    /**
     * @brief Register a new demo
     * @param name Demo name shown in menu
     * @param description Brief description
     * @param createFunction Function to create demo UI
     */
    void registerDemo(const char* name, const char* description, 
                     std::function<void(lv_obj_t*)> createFunction);
    
    /**
     * @brief Show the main menu
     */
    void showMenu();
    
    /**
     * @brief Get number of registered demos
     */
    size_t getDemoCount() const { return demos.size(); }
    
    /**
     * @brief Clear all demos
     */
    void clearDemos();
};

#endif // DEMO_MANAGER_H
