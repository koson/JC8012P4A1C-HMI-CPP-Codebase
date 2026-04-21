#pragma once

#include "lvgl.h"
#include <functional>
#include <unordered_map>

/**
 * @brief Input device wrapper for LVGL v9 (non-owning)
 *
 * Provides a C++ interface over lv_indev_t for querying type/state,
 * setting cursor/group, and attaching std::function read callbacks.
 * The wrapper does not create or delete devices; it references existing
 * devices created by the BSP or LVGL port.
 */
class LVIndev {
public:
    enum class Type {
        Pointer,
        Keypad,
        Encoder,
        Button,
        Unknown,
    };

    using ReadCallback = std::function<void(lv_indev_data_t&)>;

    /**
     * @brief Get the first registered input device (usually touch)
     */
    static LVIndev getDefault();

    /**
     * @brief Wrap an existing lv_indev_t* (non-owning)
     */
    explicit LVIndev(lv_indev_t* indev = nullptr);

    /**
     * @brief Get raw lv_indev_t pointer
     */
    lv_indev_t* raw() const { return m_indev; }

    /**
     * @brief Device type
     */
    Type type() const;

    /**
     * @brief Check if wrapper holds a valid device
     */
    bool valid() const { return m_indev != nullptr; }

    /**
     * @brief Enable rotation-aware coordinate transform using the given display
     * (handles 90/180/270 based on lv_display_get_rotation). Safe to skip if
     * the BSP already remaps touch.
     */
    void enableRotationTransform(lv_display_t* display);

    /**
     * @brief Disable rotation transform
     */
    void disableRotationTransform();

    /**
     * @brief Set user data pointer on the device
     */
    void setUserData(void* user_data);

    /**
     * @brief Get user data pointer from the device
     */
    void* userData() const;

    /**
     * @brief Attach a C++ read callback. Overrides the device read callback.
     * The callback should fill lv_indev_data_t fields (state, points, etc.).
     */
    void setReadCallback(ReadCallback cb);

    /**
     * @brief Set LVGL cursor object (pointer devices only)
     */
    void setCursor(lv_obj_t* cursor_obj);

    /**
     * @brief Assign LVGL group (keypad/encoder)
     */
    void setGroup(lv_group_t* group);

    /**
     * @brief Get last sampled point (if pointer device)
     * @return true if point is valid
     */
    bool getPoint(lv_point_t& out) const;

    /**
     * @brief Get current state (pressed/released)
     */
    lv_indev_state_t getState() const;

    /**
     * @brief Convenience: is device pressed
     */
    bool isPressed() const { return getState() == LV_INDEV_STATE_PRESSED; }

private:
    struct CallbackEntry {
        ReadCallback cb;
        lv_indev_read_cb_t previous;
    };

    static void readThunk(lv_indev_t* indev, lv_indev_data_t* data);

    lv_indev_t* m_indev;
    bool m_applyTransform = false;
    lv_display_t* m_transformDisplay = nullptr;
    static std::unordered_map<lv_indev_t*, CallbackEntry> s_callbacks;
};
