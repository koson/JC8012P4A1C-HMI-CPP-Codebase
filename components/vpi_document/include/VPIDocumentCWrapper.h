#ifndef VPI_DOCUMENT_C_WRAPPER_H
#define VPI_DOCUMENT_C_WRAPPER_H

#include "VPIDataSnapshot.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle to VPIDocument instance
 */
typedef void* vpi_document_handle_t;

/**
 * @brief Create VPIDocument instance
 * @return Handle to VPIDocument or NULL on failure
 */
vpi_document_handle_t vpi_document_create(void);

/**
 * @brief Destroy VPIDocument instance
 * @param handle VPIDocument handle
 */
void vpi_document_destroy(vpi_document_handle_t handle);

/**
 * @brief Update VPI data snapshot
 * @param handle VPIDocument handle
 * @param snapshot Data snapshot
 */
void vpi_document_update_data(vpi_document_handle_t handle, const vpi_data_snapshot_t* snapshot);

/**
 * @brief Update only VPI state
 * @param handle VPIDocument handle
 * @param state VPI state
 */
void vpi_document_update_state(vpi_document_handle_t handle, vpi_state_t state);

/**
 * @brief Update only PM2230 data
 * @param handle VPIDocument handle
 * @param pm2230 Power meter data
 */
void vpi_document_update_pm2230(vpi_document_handle_t handle, const pm2230_data_t* pm2230);

/**
 * @brief Update only pressure data
 * @param handle VPIDocument handle
 * @param pressure Pressure sensor data
 */
void vpi_document_update_pressure(vpi_document_handle_t handle, const pressure_data_t* pressure);

/**
 * @brief Get current data snapshot
 * @param handle VPIDocument handle
 * @param snapshot Output snapshot
 */
void vpi_document_get_current_data(vpi_document_handle_t handle, vpi_data_snapshot_t* snapshot);

/**
 * @brief Attach observer (internal use - for C++ observers only)
 * @param handle VPIDocument handle
 * @param observer Observer instance
 */
void vpi_document_attach_observer(vpi_document_handle_t handle, void* observer);

#ifdef __cplusplus
}
#endif

#endif // VPI_DOCUMENT_C_WRAPPER_H
