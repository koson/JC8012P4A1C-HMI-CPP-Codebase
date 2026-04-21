#include "LVIndev.hpp"

#include <algorithm>

std::unordered_map<lv_indev_t*, LVIndev::CallbackEntry> LVIndev::s_callbacks;

LVIndev::LVIndev(lv_indev_t* indev) : m_indev(indev) {}

LVIndev LVIndev::getDefault()
{
    return LVIndev(lv_indev_get_next(nullptr));
}

LVIndev::Type LVIndev::type() const
{
    if (!m_indev) return Type::Unknown;
    switch (lv_indev_get_type(m_indev)) {
        case LV_INDEV_TYPE_POINTER: return Type::Pointer;
        case LV_INDEV_TYPE_KEYPAD:  return Type::Keypad;
        case LV_INDEV_TYPE_ENCODER: return Type::Encoder;
        case LV_INDEV_TYPE_BUTTON:  return Type::Button;
        default: return Type::Unknown;
    }
}

void LVIndev::setUserData(void* user_data)
{
#if LV_USE_USER_DATA
    if (m_indev) {
        lv_indev_set_user_data(m_indev, user_data);
    }
#else
    LV_UNUSED(user_data);
#endif
}

void LVIndev::enableRotationTransform(lv_display_t* display)
{
    m_transformDisplay = display;
    m_applyTransform = (display != nullptr);
}

void LVIndev::disableRotationTransform()
{
    m_applyTransform = false;
    m_transformDisplay = nullptr;
}

void* LVIndev::userData() const
{
#if LV_USE_USER_DATA
    return m_indev ? lv_indev_get_user_data(m_indev) : nullptr;
#else
    return nullptr;
#endif
}

void LVIndev::setReadCallback(ReadCallback cb)
{
    if (!m_indev) return;
    if (cb) {
        CallbackEntry entry;
        entry.cb = std::move(cb);
        entry.previous = lv_indev_get_read_cb(m_indev);
        s_callbacks[m_indev] = entry;
        lv_indev_set_read_cb(m_indev, readThunk);
    } else {
        auto it = s_callbacks.find(m_indev);
        if (it != s_callbacks.end()) {
            lv_indev_set_read_cb(m_indev, it->second.previous);
            s_callbacks.erase(it);
        }
    }
}

void LVIndev::setCursor(lv_obj_t* cursor_obj)
{
    if (!m_indev) return;
    lv_indev_set_cursor(m_indev, cursor_obj);
}

void LVIndev::setGroup(lv_group_t* group)
{
    if (!m_indev) return;
    lv_indev_set_group(m_indev, group);
}

bool LVIndev::getPoint(lv_point_t& out) const
{
    if (!m_indev) return false;
    lv_indev_get_point(m_indev, &out);

    if (m_applyTransform && m_transformDisplay) {
        const lv_display_rotation_t rot = lv_display_get_rotation(m_transformDisplay);
        const int32_t hres = lv_display_get_horizontal_resolution(m_transformDisplay);
        const int32_t vres = lv_display_get_vertical_resolution(m_transformDisplay);
        int32_t x = out.x;
        int32_t y = out.y;
        switch (rot) {
            case LV_DISPLAY_ROTATION_0:
                break;
            case LV_DISPLAY_ROTATION_90:
                out.x = y;
                out.y = hres - 1 - x;
                break;
            case LV_DISPLAY_ROTATION_180:
                out.x = hres - 1 - x;
                out.y = vres - 1 - y;
                break;
            case LV_DISPLAY_ROTATION_270:
                out.x = vres - 1 - y;
                out.y = x;
                break;
            default:
                break;
        }
    }
    return true;
}

lv_indev_state_t LVIndev::getState() const
{
    if (!m_indev) return LV_INDEV_STATE_RELEASED;
    return lv_indev_get_state(m_indev);
}

void LVIndev::readThunk(lv_indev_t* indev, lv_indev_data_t* data)
{
    auto it = s_callbacks.find(indev);
    if (it == s_callbacks.end()) return;

    if (it->second.previous) {
        it->second.previous(indev, data);
    }

    if (it->second.cb) {
        it->second.cb(*data);
    }
}
