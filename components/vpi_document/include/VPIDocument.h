#ifndef VPI_DOCUMENT_H
#define VPI_DOCUMENT_H

#include "VPIDataSnapshot.h"
#include "IVPIObserver.h"
#include <vector>
#include <mutex>

/**
 * @brief VPI Document - Central Data Model
 * 
 * Manages VPI system data using Document-View pattern inspired by MFC.
 * Implements Observer pattern for real-time updates to multiple views/loggers.
 * Thread-safe for use in FreeRTOS multi-task environment.
 */
class VPIDocument {
public:
    VPIDocument();
    ~VPIDocument();
    
    // ========== Observer Pattern ==========
    /**
     * @brief Attach an observer to receive data updates
     * @param observer Observer instance (DataLogger, UI, etc.)
     */
    void attach(IVPIObserver* observer);
    
    /**
     * @brief Detach an observer
     * @param observer Observer instance to remove
     */
    void detach(IVPIObserver* observer);
    
    // ========== Data Management ==========
    /**
     * @brief Update VPI data and notify all observers
     * @param snapshot New data snapshot
     */
    void updateData(const vpi_data_snapshot_t& snapshot);
    
    /**
     * @brief Get current data snapshot (thread-safe)
     * @return Current VPI data snapshot
     */
    vpi_data_snapshot_t getCurrentData() const;
    
    /**
     * @brief Update only VPI state
     * @param state New VPI state
     */
    void updateState(vpi_state_t state);
    
    /**
     * @brief Update only PM2230 data
     * @param pm2230 Power meter data
     */
    void updatePM2230(const pm2230_data_t& pm2230);
    
    /**
     * @brief Update only pressure data
     * @param pressure Pressure sensor data
     */
    void updatePressure(const pressure_data_t& pressure);
    
private:
    /**
     * @brief Notify all observers about data change
     */
    void notifyObservers();
    
    vpi_data_snapshot_t current_data_;         // Current data snapshot
    std::vector<IVPIObserver*> observers_;     // Registered observers
    mutable std::mutex mutex_;                 // Thread safety
};

#endif // VPI_DOCUMENT_H
