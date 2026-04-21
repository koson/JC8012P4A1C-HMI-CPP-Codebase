/**
 * @file LVDocument.hpp
 * @brief Document base class for data model with file I/O integration
 * 
 * Phase 8.1: Core Framework - Document/View Architecture
 * Provides:
 * - Abstract data model base class
 * - File I/O integration (save/load)
 * - Modified flag tracking
 * - Serialization support
 * - Integration with LVStorage
 */

#ifndef LV_DOCUMENT_HPP
#define LV_DOCUMENT_HPP

#include <string>
#include <functional>

// Forward declarations
class LVStorage;

/**
 * @class LVDocument
 * @brief Base class for data models with file persistence
 * 
 * Document/View pattern implementation similar to MFC CDocument.
 * Handles data storage, serialization, and file I/O operations.
 * 
 * Usage:
 * @code
 * class MyDocument : public LVDocument {
 * public:
 *     MyDocument() : LVDocument("MyDoc") {}
 *     
 *     bool serialize(bool isLoading, std::string& data) override {
 *         if (isLoading) {
 *             // Parse data and load into members
 *             return parseData(data);
 *         } else {
 *             // Convert members to string
 *             data = formatData();
 *             return true;
 *         }
 *     }
 * };
 * @endcode
 */
class LVDocument {
public:
    /**
     * @brief Constructor
     * @param className Class name for identification
     */
    explicit LVDocument(const char* className = "LVDocument");
    
    /**
     * @brief Virtual destructor
     */
    virtual ~LVDocument();
    
    // === File Operations ===
    
    /**
     * @brief Create new document (reset to default state)
     * @return true if successful
     */
    virtual bool newDocument();
    
    /**
     * @brief Open document from file
     * @param filePath Full path to file
     * @return true if successful
     */
    virtual bool open(const char* filePath);
    
    /**
     * @brief Save document to current file
     * @return true if successful
     */
    virtual bool save();
    
    /**
     * @brief Save document to specified file
     * @param filePath Full path to file
     * @return true if successful
     */
    virtual bool saveAs(const char* filePath);
    
    /**
     * @brief Close document (prompt if modified)
     * @param force Force close without saving
     * @return true if closed successfully
     */
    virtual bool close(bool force = false);
    
    // === Serialization ===
    
    /**
     * @brief Serialize document data (override in derived classes)
     * @param isLoading true = load from data, false = save to data
     * @param data String buffer for serialized data
     * @return true if successful
     */
    virtual bool serialize(bool isLoading, std::string& data);
    
    // === Modified State ===
    
    /**
     * @brief Check if document has unsaved changes
     * @return true if modified
     */
    bool isModified() const { return m_modified; }
    
    /**
     * @brief Mark document as modified
     * @param modified Modified state
     */
    void setModified(bool modified = true);
    
    // === File Path Management ===
    
    /**
     * @brief Get current file path
     * @return File path or empty string if untitled
     */
    const char* getFilePath() const { return m_filePath.c_str(); }
    
    /**
     * @brief Get document title (filename without path)
     * @return Document title
     */
    const char* getTitle() const;
    
    /**
     * @brief Check if document is untitled (not saved yet)
     * @return true if untitled
     */
    bool isUntitled() const { return m_filePath.empty(); }
    
    /**
     * @brief Get the class name
     */
    const char* getClassName() const { return m_className; }
    
    // === Event Callbacks ===
    
    /**
     * @brief Set callback for document modified event
     * @param callback Function called when document is modified
     * @param userData User data passed to callback
     */
    void setModifiedCallback(std::function<void(LVDocument*, void*)> callback, void* userData = nullptr);
    
protected:
    /**
     * @brief Called when document is created/reset
     * Override to initialize document-specific data
     */
    virtual void onNewDocument();
    
    /**
     * @brief Called after document is loaded
     * Override to process loaded data
     */
    virtual void onDocumentOpened();
    
    /**
     * @brief Called before document is saved
     * Override to prepare data for saving
     */
    virtual void onDocumentSaving();
    
    /**
     * @brief Called before document is closed
     * @return true to allow closing, false to cancel
     */
    virtual bool onDocumentClosing();
    
    /**
     * @brief Get storage instance
     * @return Pointer to LVStorage singleton
     */
    LVStorage* getStorage() const;
    
private:
    std::string m_filePath;           ///< Current file path
    bool m_modified;                  ///< Modified flag
    mutable std::string m_title;      ///< Cached title
    const char* m_className;          ///< Class name for this document
    
    // Callbacks
    std::function<void(LVDocument*, void*)> m_modifiedCallback;
    void* m_modifiedCallbackUserData;
    
    /**
     * @brief Update title cache
     */
    void updateTitle() const;
};

#endif // LV_DOCUMENT_HPP
