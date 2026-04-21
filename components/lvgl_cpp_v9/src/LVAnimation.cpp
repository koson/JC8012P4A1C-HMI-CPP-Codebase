#include "LVAnimation.hpp"

LVAnimation::LVAnimation(lv_obj_t* target)
    : target_obj(target)
    , current_property(Property::X)
    , duration_ms(500)
    , custom_exec_cb(nullptr)
    , start_cb(nullptr)
    , ready_cb(nullptr)
{
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, target);
    lv_anim_set_user_data(&anim, this);
    lv_anim_set_deleted_cb(&anim, deleted_cb_wrapper);
}

LVAnimation::~LVAnimation()
{
    lv_anim_delete(&anim, nullptr);
}

LVAnimation& LVAnimation::setProperty(Property prop)
{
    current_property = prop;
    setupPropertyExec();
    return *this;
}

LVAnimation& LVAnimation::setValues(int32_t start, int32_t end)
{
    lv_anim_set_values(&anim, start, end);
    return *this;
}

LVAnimation& LVAnimation::setDuration(uint32_t ms)
{
    duration_ms = ms;
    lv_anim_set_duration(&anim, ms);
    return *this;
}

LVAnimation& LVAnimation::setDelay(uint32_t ms)
{
    lv_anim_set_delay(&anim, ms);
    return *this;
}

LVAnimation& LVAnimation::setPath(Path path)
{
    switch (path) {
        case Path::Linear:
            lv_anim_set_path_cb(&anim, lv_anim_path_linear);
            break;
        case Path::EaseIn:
            lv_anim_set_path_cb(&anim, lv_anim_path_ease_in);
            break;
        case Path::EaseOut:
            lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
            break;
        case Path::EaseInOut:
            lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
            break;
        case Path::Overshoot:
            lv_anim_set_path_cb(&anim, lv_anim_path_overshoot);
            break;
        case Path::Bounce:
            lv_anim_set_path_cb(&anim, lv_anim_path_bounce);
            break;
    }
    return *this;
}

LVAnimation& LVAnimation::setRepeatCount(int32_t count)
{
    lv_anim_set_repeat_count(&anim, count);
    return *this;
}

LVAnimation& LVAnimation::setRepeatDelay(uint32_t ms)
{
    lv_anim_set_repeat_delay(&anim, ms);
    return *this;
}

LVAnimation& LVAnimation::setPlayback(bool enable)
{
    lv_anim_set_playback_duration(&anim, enable ? duration_ms : 0);
    return *this;
}

LVAnimation& LVAnimation::setPlaybackDelay(uint32_t ms)
{
    lv_anim_set_playback_delay(&anim, ms);
    return *this;
}

LVAnimation& LVAnimation::setCustomCallback(std::function<void(int32_t)> callback)
{
    custom_exec_cb = callback;
    lv_anim_set_exec_cb(&anim, exec_cb_wrapper);
    return *this;
}

LVAnimation& LVAnimation::onStart(std::function<void()> callback)
{
    start_cb = callback;
    lv_anim_set_start_cb(&anim, start_cb_wrapper);
    return *this;
}

LVAnimation& LVAnimation::onReady(std::function<void()> callback)
{
    ready_cb = callback;
    lv_anim_set_completed_cb(&anim, ready_cb_wrapper);
    return *this;
}

void LVAnimation::start()
{
    lv_anim_start(&anim);
}

void LVAnimation::stop()
{
    lv_anim_delete(&anim, nullptr);
}

// Static callback wrappers
void LVAnimation::exec_cb_wrapper(void* var, int32_t value)
{
    // Get animation from var (it's the target object)
    // We need to get LVAnimation* from user_data instead
    // This is called by custom callback setup
}

void LVAnimation::start_cb_wrapper(lv_anim_t* a)
{
    LVAnimation* anim = static_cast<LVAnimation*>(lv_anim_get_user_data(a));
    if (anim && anim->start_cb) {
        anim->start_cb();
    }
}

void LVAnimation::ready_cb_wrapper(lv_anim_t* a)
{
    LVAnimation* anim = static_cast<LVAnimation*>(lv_anim_get_user_data(a));
    if (anim && anim->ready_cb) {
        anim->ready_cb();
    }
}

void LVAnimation::deleted_cb_wrapper(lv_anim_t* a)
{
    // Cleanup if needed
}

void LVAnimation::setupPropertyExec()
{
    switch (current_property) {
        case Property::X:
            lv_anim_set_exec_cb(&anim, [](void* var, int32_t value) {
                lv_obj_set_x(static_cast<lv_obj_t*>(var), value);
            });
            break;
            
        case Property::Y:
            lv_anim_set_exec_cb(&anim, [](void* var, int32_t value) {
                lv_obj_set_y(static_cast<lv_obj_t*>(var), value);
            });
            break;
            
        case Property::Width:
            lv_anim_set_exec_cb(&anim, [](void* var, int32_t value) {
                lv_obj_set_width(static_cast<lv_obj_t*>(var), value);
            });
            break;
            
        case Property::Height:
            lv_anim_set_exec_cb(&anim, [](void* var, int32_t value) {
                lv_obj_set_height(static_cast<lv_obj_t*>(var), value);
            });
            break;
            
        case Property::Opacity:
            lv_anim_set_exec_cb(&anim, [](void* var, int32_t value) {
                lv_obj_set_style_opa(static_cast<lv_obj_t*>(var), value, 0);
            });
            break;
            
        case Property::Rotation:
            lv_anim_set_exec_cb(&anim, [](void* var, int32_t value) {
                lv_obj_set_style_transform_rotation(static_cast<lv_obj_t*>(var), value, 0);
            });
            break;
            
        case Property::Custom:
            // Custom callback will be set separately
            break;
    }
}
