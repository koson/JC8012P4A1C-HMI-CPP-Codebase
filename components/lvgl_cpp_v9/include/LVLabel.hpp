#ifndef LVLABEL_HPP
#define LVLABEL_HPP

#include "LVWidget.hpp"
#include <string>
#include <sstream>

/**
 * @brief Label Widget Class - C++ Version
 * 
 * คล้าย QLabel ใน Qt หรือ Label ใน WPF
 */


class LVLabel : public LVWidget {
public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit LVLabel(LVWidget* parent = nullptr);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~LVLabel() override;
    
    // ===== Label-Specific Methods =====
    
    /**
     * @brief Set label text (รองรับ std::string!)
     */
    LVLabel* setText(const std::string& text);
    LVLabel* setText(const char* text);
    
    /**
     * @brief Get label text
     */
    const std::string& text() const { return m_text; }
    
    /**
     * @brief Set text with formatting (C++ style using stringstream)
     * 
     * Example:
     *   label->setTextFormat("Count: ", counter);
     */
    template<typename... Args>
    void setTextFormat(Args&&... args) {
        std::ostringstream oss;
        (oss << ... << args);  // C++17 fold expression
        setText(oss.str());
    }
    
    /**
     * @brief Set text font
     */
    void setFont(const lv_font_t* font);
    
    /**
     * @brief Set text color
     */
    void setTextColor(lv_color_t color);
    
    /**
     * @brief Set long text mode
     */
    void setLongMode(lv_label_long_mode_t mode);
    
    /**
     * @brief Fluent API
     */
    LVLabel* withText(const std::string& text) {
        setText(text);
        return this;
    }
    
    LVLabel* withFont(const lv_font_t* font) {
        setFont(font);
        return this;
    }
    
    LVLabel* withColor(lv_color_t color) {
        setTextColor(color);
        return this;
    }
    
protected:
    std::string m_text;
    const lv_font_t* m_font;
    lv_color_t m_textColor;
    lv_label_long_mode_t m_longMode;
    
private:
    static constexpr const char* TAG = "LVLabel";
};

#endif // LVLABEL_HPP
