#ifndef LVBUTTON_HPP
#define LVBUTTON_HPP

#include "LVWidget.hpp"
#include <string>

/**
 * @brief Button Widget Class - C++ Version
 * 
 * คล้าย QPushButton ใน Qt หรือ Button ใน WPF
 */

class LVButton : public LVWidget {
public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit LVButton(LVWidget* parent = nullptr);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~LVButton() override;
    
    // ===== Button-Specific Methods =====
    
    /**
     * @brief Set button text (รองรับ std::string!)
     */
    LVButton* setText(const std::string& text);
    LVButton* setText(const char* text);
    
    /**
     * @brief Get button text
     */
    const std::string& text() const { return m_text; }
    
    /**
     * @brief Set text font
     */
    void setFont(const lv_font_t* font);
    
    /**
     * @brief Set text color
     */
    void setTextColor(lv_color_t color);
    
    /**
     * @brief Fluent API - return this
     */
    LVButton* withText(const std::string& text) {
        setText(text);
        return this;
    }
    
    LVButton* withFont(const lv_font_t* font) {
        setFont(font);
        return this;
    }
    
    // ===== Virtual Event Handler Override =====
    
    /**
     * @brief Override handleClicked สำหรับ button-specific behavior
     */
    virtual void handleClicked(lv_event_t* event) override;
    
protected:
    lv_obj_t* m_label;      // Label object for text
    std::string m_text;     // Button text (std::string!)
    const lv_font_t* m_font;
    lv_color_t m_textColor;
    
private:
    static constexpr const char* TAG = "LVButton";
};

#endif // LVBUTTON_HPP
