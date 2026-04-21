#ifndef I_VPI_OBSERVER_H
#define I_VPI_OBSERVER_H

#include "VPIDataSnapshot.h"

/**
 * @brief Observer Interface for VPI Data Updates
 * 
 * Implement this interface to receive notifications when VPI data changes.
 * Classic Observer pattern implementation for real-time data monitoring.
 */
class IVPIObserver {
public:
    virtual ~IVPIObserver() = default;
    
    /**
     * @brief Called when VPI data is updated
     * @param snapshot Immutable snapshot of current VPI data
     */
    virtual void onDataUpdate(const vpi_data_snapshot_t& snapshot) = 0;
};

#endif // I_VPI_OBSERVER_H
