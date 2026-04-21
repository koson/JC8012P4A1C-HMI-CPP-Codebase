/**
 * @file chart_demo.cpp
 * @brief Phase 2 Test #11 - LVChart (Data Visualization)
 * 
 * Tests:
 * - TC1: Line chart with multiple series
 * - TC2: Bar chart visualization
 * - TC3: Real-time data updates
 * - TC4: Add/remove series dynamically
 * - TC5: Custom range and divisions
 * - TC6: Color-coded series
 * - TC7: Point count configuration
 * - TC8: Data refresh and animation
 */

#include "../include/LVChart.hpp"
#include "../include/LVLabel.hpp"
#include "../include/LVButton.hpp"
#include "../include/LVPanel.hpp"
#include "esp_log.h"
#include <cstdlib>
#include <ctime>

static const char* TAG = "CHART_TEST";

// Global widgets
static LVChart* chartTemp = nullptr;
static LVChart* chartSales = nullptr;
static LVLabel* lblTempValue = nullptr;
static LVLabel* lblStatus = nullptr;

// Chart series
static lv_chart_series_t* seriesTempActual = nullptr;
static lv_chart_series_t* seriesTempTarget = nullptr;
static lv_chart_series_t* seriesProduct1 = nullptr;
static lv_chart_series_t* seriesProduct2 = nullptr;
static lv_chart_series_t* seriesProduct3 = nullptr;

// Data tracking
static int32_t tempActual = 25;
static int32_t tempTarget = 22;

/**
 * @brief Update temperature chart with new data
 */
static void updateTempChart()
{
    // Simulate temperature fluctuation
    tempActual += (rand() % 5) - 2;  // -2 to +2 degrees
    if (tempActual < 18) tempActual = 18;
    if (tempActual > 32) tempActual = 32;
    
    // Add new data point (shifts old data left)
    chartTemp->setNextSeriesValue(seriesTempActual, tempActual);
    chartTemp->setNextSeriesValue(seriesTempTarget, tempTarget);
    
    // Update current value label
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Current: %ldC", tempActual);
    lblTempValue->setText(buffer);
    
    // Color code based on temperature
    if (tempActual > tempTarget + 3) {
        lblTempValue->setTextColor(lv_color_hex(0xD32F2F));  // Red - too hot
        lblStatus->setText("Temperature: HIGH");
        lblStatus->setTextColor(lv_color_hex(0xD32F2F));
    } else if (tempActual < tempTarget - 3) {
        lblTempValue->setTextColor(lv_color_hex(0x1976D2));  // Blue - too cold
        lblStatus->setText("Temperature: LOW");
        lblStatus->setTextColor(lv_color_hex(0x1976D2));
    } else {
        lblTempValue->setTextColor(lv_color_hex(0x388E3C));  // Green - OK
        lblStatus->setText("Temperature: NORMAL");
        lblStatus->setTextColor(lv_color_hex(0x388E3C));
    }
    
    chartTemp->refresh();
    
    ESP_LOGI(TAG, "Temperature: Actual=%ldC, Target=%ldC", tempActual, tempTarget);
}

/**
 * @brief Update sales chart with random data
 */
static void updateSalesChart()
{
    // Random sales data for 3 products
    int32_t sales1 = 50 + (rand() % 50);
    int32_t sales2 = 40 + (rand() % 60);
    int32_t sales3 = 30 + (rand() % 70);
    
    chartSales->setNextSeriesValue(seriesProduct1, sales1);
    chartSales->setNextSeriesValue(seriesProduct2, sales2);
    chartSales->setNextSeriesValue(seriesProduct3, sales3);
    
    chartSales->refresh();
    
    ESP_LOGI(TAG, "Sales: P1=%ld, P2=%ld, P3=%ld", sales1, sales2, sales3);
}

/**
 * @brief Add data button callback
 */
static void onAddDataClicked(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED) return;
    
    updateTempChart();
    updateSalesChart();
}

/**
 * @brief Increase target button callback
 */
static void onIncreaseTargetClicked(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED) return;
    
    tempTarget++;
    if (tempTarget > 30) tempTarget = 30;
    
    // Update all target points
    chartTemp->setAllSeriesValue(seriesTempTarget, tempTarget);
    chartTemp->refresh();
    
    ESP_LOGI(TAG, "Target increased to %ldC", tempTarget);
}

/**
 * @brief Decrease target button callback
 */
static void onDecreaseTargetClicked(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED) return;
    
    tempTarget--;
    if (tempTarget < 18) tempTarget = 18;
    
    // Update all target points
    chartTemp->setAllSeriesValue(seriesTempTarget, tempTarget);
    chartTemp->refresh();
    
    ESP_LOGI(TAG, "Target decreased to %ldC", tempTarget);
}

/**
 * @brief Create Chart Demo UI
 */
extern "C" void test_chart_demo()
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Phase 2 Test #11: LVChart - Data Visualization");
    ESP_LOGI(TAG, "========================================");
    
    // Initialize random seed
    srand(time(nullptr));
    
    // Main container
    auto* container = new LVPanel(nullptr);
    lv_obj_set_size(container->obj(), 900, 560);
    lv_obj_align(container->obj(), LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(container->obj(), lv_color_hex(0xF5F5F5), 0);
    lv_obj_set_style_border_width(container->obj(), 0, 0);
    lv_obj_set_style_pad_all(container->obj(), 20, 0);
    
    // Title
    auto* lblTitle = new LVLabel(container);
    lblTitle->setText("Chart Visualization");
    lv_obj_set_style_text_font(lblTitle->obj(), &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(lblTitle->obj(), lv_color_hex(0x1976D2), 0);
    lv_obj_align(lblTitle->obj(), LV_ALIGN_TOP_MID, 0, 0);
    
    // ===== Temperature Chart (Line Chart) =====
    auto* lblTempTitle = new LVLabel(container);
    lblTempTitle->setText("Temperature Monitor (Line Chart)");
    lv_obj_set_style_text_font(lblTempTitle->obj(), &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lblTempTitle->obj(), lv_color_hex(0x424242), 0);
    lv_obj_align(lblTempTitle->obj(), LV_ALIGN_TOP_LEFT, 0, 45);
    
    chartTemp = new LVChart(container);
    lv_obj_set_size(chartTemp->obj(), 400, 200);
    lv_obj_align(chartTemp->obj(), LV_ALIGN_TOP_LEFT, 0, 70);
    
    // Configure chart
    chartTemp->setType(LV_CHART_TYPE_LINE);
    chartTemp->setPointCount(20);
    chartTemp->setRange(LV_CHART_AXIS_PRIMARY_Y, 15, 35);  // 15-35°C
    chartTemp->setDivLineCount(5, 8);
    
    // Styling
    lv_obj_set_style_bg_color(chartTemp->obj(), lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(chartTemp->obj(), lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_border_width(chartTemp->obj(), 2, 0);
    lv_obj_set_style_radius(chartTemp->obj(), 5, 0);
    lv_obj_set_style_pad_all(chartTemp->obj(), 5, 0);
    
    // Add series
    seriesTempActual = chartTemp->addSeries(lv_color_hex(0xD32F2F), LV_CHART_AXIS_PRIMARY_Y);  // Red line
    seriesTempTarget = chartTemp->addSeries(lv_color_hex(0x388E3C), LV_CHART_AXIS_PRIMARY_Y);  // Green line
    
    // Initialize with starting values
    chartTemp->setAllSeriesValue(seriesTempActual, tempActual);
    chartTemp->setAllSeriesValue(seriesTempTarget, tempTarget);
    
    // Set line width
    lv_obj_set_style_line_width(chartTemp->obj(), 3, LV_PART_ITEMS);
    
    // Temperature legend
    auto* lblLegendActual = new LVLabel(container);
    lblLegendActual->setText("Actual");
    lv_obj_set_style_text_font(lblLegendActual->obj(), &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lblLegendActual->obj(), lv_color_hex(0xD32F2F), 0);
    lv_obj_align(lblLegendActual->obj(), LV_ALIGN_TOP_LEFT, 0, 280);
    
    auto* lblLegendTarget = new LVLabel(container);
    lblLegendTarget->setText("Target");
    lv_obj_set_style_text_font(lblLegendTarget->obj(), &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lblLegendTarget->obj(), lv_color_hex(0x388E3C), 0);
    lv_obj_align(lblLegendTarget->obj(), LV_ALIGN_TOP_LEFT, 80, 280);
    
    // Current value label
    lblTempValue = new LVLabel(container);
    lblTempValue->setText("Current: 25C");
    lv_obj_set_style_text_font(lblTempValue->obj(), &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(lblTempValue->obj(), lv_color_hex(0x388E3C), 0);
    lv_obj_align(lblTempValue->obj(), LV_ALIGN_TOP_LEFT, 200, 278);
    
    // Control buttons for temperature
    auto* btnIncTarget = new LVButton(container);
    lv_obj_set_size(btnIncTarget->obj(), 80, 35);
    lv_obj_align(btnIncTarget->obj(), LV_ALIGN_TOP_LEFT, 0, 310);
    lv_obj_set_style_bg_color(btnIncTarget->obj(), lv_color_hex(0xD32F2F), 0);
    btnIncTarget->setText("+1C");
    btnIncTarget->setTextColor(lv_color_hex(0xFFFFFF));
    lv_obj_add_event_cb(btnIncTarget->obj(), onIncreaseTargetClicked, LV_EVENT_CLICKED, nullptr);
    
    auto* btnDecTarget = new LVButton(container);
    lv_obj_set_size(btnDecTarget->obj(), 80, 35);
    lv_obj_align(btnDecTarget->obj(), LV_ALIGN_TOP_LEFT, 90, 310);
    lv_obj_set_style_bg_color(btnDecTarget->obj(), lv_color_hex(0x1976D2), 0);
    btnDecTarget->setText("-1C");
    btnDecTarget->setTextColor(lv_color_hex(0xFFFFFF));
    lv_obj_add_event_cb(btnDecTarget->obj(), onDecreaseTargetClicked, LV_EVENT_CLICKED, nullptr);
    
    // ===== Sales Chart (Bar Chart) =====
    auto* lblSalesTitle = new LVLabel(container);
    lblSalesTitle->setText("Product Sales (Bar Chart)");
    lv_obj_set_style_text_font(lblSalesTitle->obj(), &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lblSalesTitle->obj(), lv_color_hex(0x424242), 0);
    lv_obj_align(lblSalesTitle->obj(), LV_ALIGN_TOP_LEFT, 440, 45);
    
    chartSales = new LVChart(container);
    lv_obj_set_size(chartSales->obj(), 420, 200);
    lv_obj_align(chartSales->obj(), LV_ALIGN_TOP_LEFT, 440, 70);
    
    // Configure chart
    chartSales->setType(LV_CHART_TYPE_BAR);
    chartSales->setPointCount(6);  // 6 time periods
    chartSales->setRange(LV_CHART_AXIS_PRIMARY_Y, 0, 100);  // 0-100 units
    chartSales->setDivLineCount(5, 6);
    
    // Styling
    lv_obj_set_style_bg_color(chartSales->obj(), lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(chartSales->obj(), lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_border_width(chartSales->obj(), 2, 0);
    lv_obj_set_style_radius(chartSales->obj(), 5, 0);
    lv_obj_set_style_pad_all(chartSales->obj(), 5, 0);
    
    // Add 3 product series
    seriesProduct1 = chartSales->addSeries(lv_color_hex(0x1976D2), LV_CHART_AXIS_PRIMARY_Y);  // Blue
    seriesProduct2 = chartSales->addSeries(lv_color_hex(0xF57C00), LV_CHART_AXIS_PRIMARY_Y);  // Orange
    seriesProduct3 = chartSales->addSeries(lv_color_hex(0x388E3C), LV_CHART_AXIS_PRIMARY_Y);  // Green
    
    // Initialize with random values
    for (int i = 0; i < 6; i++) {
        chartSales->setSeriesValue(seriesProduct1, i, 50 + (rand() % 50));
        chartSales->setSeriesValue(seriesProduct2, i, 40 + (rand() % 60));
        chartSales->setSeriesValue(seriesProduct3, i, 30 + (rand() % 70));
    }
    
    // Sales legend
    auto* lblProduct1 = new LVLabel(container);
    lblProduct1->setText("Product A");
    lv_obj_set_style_text_font(lblProduct1->obj(), &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lblProduct1->obj(), lv_color_hex(0x1976D2), 0);
    lv_obj_align(lblProduct1->obj(), LV_ALIGN_TOP_LEFT, 440, 280);
    
    auto* lblProduct2 = new LVLabel(container);
    lblProduct2->setText("Product B");
    lv_obj_set_style_text_font(lblProduct2->obj(), &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lblProduct2->obj(), lv_color_hex(0xF57C00), 0);
    lv_obj_align(lblProduct2->obj(), LV_ALIGN_TOP_LEFT, 560, 280);
    
    auto* lblProduct3 = new LVLabel(container);
    lblProduct3->setText("Product C");
    lv_obj_set_style_text_font(lblProduct3->obj(), &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lblProduct3->obj(), lv_color_hex(0x388E3C), 0);
    lv_obj_align(lblProduct3->obj(), LV_ALIGN_TOP_LEFT, 680, 280);
    
    // ===== Add Data Button =====
    auto* btnAddData = new LVButton(container);
    lv_obj_set_size(btnAddData->obj(), 200, 50);
    lv_obj_align(btnAddData->obj(), LV_ALIGN_TOP_LEFT, 330, 380);
    lv_obj_set_style_bg_color(btnAddData->obj(), lv_color_hex(0x1976D2), 0);
    btnAddData->setText("Add New Data");
    btnAddData->setTextColor(lv_color_hex(0xFFFFFF));
    btnAddData->setFont(&lv_font_montserrat_18);
    lv_obj_add_event_cb(btnAddData->obj(), onAddDataClicked, LV_EVENT_CLICKED, nullptr);
    
    // ===== Status Label =====
    lblStatus = new LVLabel(container);
    lblStatus->setText("Temperature: NORMAL");
    lv_obj_set_style_text_font(lblStatus->obj(), &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(lblStatus->obj(), lv_color_hex(0x388E3C), 0);
    lv_obj_align(lblStatus->obj(), LV_ALIGN_BOTTOM_MID, 0, -10);
    
    ESP_LOGI(TAG, "✅ Chart Demo UI created");
    ESP_LOGI(TAG, "   - Temperature chart: Line chart, 20 points, 15-35°C");
    ESP_LOGI(TAG, "   - Sales chart: Bar chart, 3 products, 6 periods");
    ESP_LOGI(TAG, "   - Click 'Add New Data' to simulate real-time updates");
}
