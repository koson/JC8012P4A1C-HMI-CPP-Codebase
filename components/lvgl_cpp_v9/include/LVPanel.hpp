#ifndef LVPANEL_HPP
#define LVPANEL_HPP

#include "LVWidget.hpp"
#include <vector>
#include <memory>

/**
 * @brief Panel Widget Class - C++ Version
 * 
 * คล้าย QWidget container ใน Qt หรือ Panel ใน WPF
 * รองรับ child management และ layout
 */

class LVPanel : public LVWidget {
public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit LVPanel(LVWidget* parent = nullptr);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~LVPanel() override;
    
    // ===== Panel-Specific Methods =====
    
    /**
     * @brief Set flex flow layout
     */
    void setFlexFlow(lv_flex_flow_t flow);
    
    /**
     * @brief Set padding
     */
    void setPadding(int32_t padding);
    
    /**
     * @brief Set gap between items
     */
    void setGap(int32_t gap);
    
    /**
     * @brief Enable/disable scrolling
     */
    void setScrollable(bool scrollable);
    
    /**
     * @brief Fluent API
     */
    LVPanel* withFlexFlow(lv_flex_flow_t flow) {
        setFlexFlow(flow);
        return this;
    }
    
    LVPanel* withPadding(int32_t padding) {
        setPadding(padding);
        return this;
    }
    
    LVPanel* withGap(int32_t gap) {
        setGap(gap);
        return this;
    }
    
protected:
    lv_flex_flow_t m_flexFlow;
    int32_t m_padding;
    int32_t m_gap;
    
private:
    static constexpr const char* TAG = "LVPanel";
};

#endif // LVPANEL_HPP
