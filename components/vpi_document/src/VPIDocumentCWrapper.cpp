#include "VPIDocumentCWrapper.h"
#include "VPIDocument.h"
#include "IVPIObserver.h"
#include <cstring>

extern "C" {

vpi_document_handle_t vpi_document_create(void) {
    VPIDocument* doc = new VPIDocument();
    return static_cast<vpi_document_handle_t>(doc);
}

void vpi_document_destroy(vpi_document_handle_t handle) {
    if (handle != nullptr) {
        VPIDocument* doc = static_cast<VPIDocument*>(handle);
        delete doc;
    }
}

void vpi_document_update_data(vpi_document_handle_t handle, const vpi_data_snapshot_t* snapshot) {
    if (handle != nullptr && snapshot != nullptr) {
        VPIDocument* doc = static_cast<VPIDocument*>(handle);
        doc->updateData(*snapshot);
    }
}

void vpi_document_update_state(vpi_document_handle_t handle, vpi_state_t state) {
    if (handle != nullptr) {
        VPIDocument* doc = static_cast<VPIDocument*>(handle);
        doc->updateState(state);
    }
}

void vpi_document_update_pm2230(vpi_document_handle_t handle, const pm2230_data_t* pm2230) {
    if (handle != nullptr && pm2230 != nullptr) {
        VPIDocument* doc = static_cast<VPIDocument*>(handle);
        doc->updatePM2230(*pm2230);
    }
}

void vpi_document_update_pressure(vpi_document_handle_t handle, const pressure_data_t* pressure) {
    if (handle != nullptr && pressure != nullptr) {
        VPIDocument* doc = static_cast<VPIDocument*>(handle);
        doc->updatePressure(*pressure);
    }
}

void vpi_document_get_current_data(vpi_document_handle_t handle, vpi_data_snapshot_t* snapshot) {
    if (handle != nullptr && snapshot != nullptr) {
        VPIDocument* doc = static_cast<VPIDocument*>(handle);
        *snapshot = doc->getCurrentData();
    }
}

void vpi_document_attach_observer(vpi_document_handle_t handle, void* observer) {
    if (handle != nullptr && observer != nullptr) {
        VPIDocument* doc = static_cast<VPIDocument*>(handle);
        IVPIObserver* obs = static_cast<IVPIObserver*>(observer);
        doc->attach(obs);
    }
}

} // extern "C"
