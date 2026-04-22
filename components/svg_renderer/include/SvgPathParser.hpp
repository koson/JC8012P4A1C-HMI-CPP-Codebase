#pragma once

#include "SvgTypes.hpp"
#include <vector>
#include <string>

namespace SvgRenderer {

/**
 * @brief SVG Path Parser
 * 
 * Parses SVG path data strings into structured commands.
 * Supports: M, L, H, V, C, Q, A, Z (absolute and relative)
 * 
 * Example:
 *   SvgPathParser parser;
 *   auto commands = parser.parse("M 25,70 L 50,30 Z");
 */
class SvgPathParser {
public:
    SvgPathParser();
    ~SvgPathParser();
    
    /**
     * @brief Parse SVG path data string
     * @param pathData SVG path string (e.g., "M 25,70 L 50,30")
     * @return Vector of parsed path commands
     */
    std::vector<PathCommand> parse(const char* pathData);
    
    /**
     * @brief Get last error message
     */
    const char* getLastError() const { return m_lastError.c_str(); }
    
private:
    /**
     * @brief Parse a single command
     * @param cmd Command character (M, L, C, etc.)
     * @param ptr Pointer to current position in string (will be updated)
     */
    PathCommand parseCommand(char cmd, const char** ptr);
    
    /**
     * @brief Parse a single number from string
     * @param ptr Pointer to current position in string (will be updated)
     */
    float parseNumber(const char** ptr);
    
    /**
     * @brief Skip whitespace and commas
     */
    void skipWhitespace(const char** ptr);
    
    /**
     * @brief Check if character is a command
     */
    bool isCommand(char c) const;
    
    /**
     * @brief Get number of arguments for a command
     */
    int getArgCount(char cmd) const;
    
    std::string m_lastError;
    Point m_currentPos;         // Current position for relative commands
    Point m_startPos;           // Start position of current subpath
};

} // namespace SvgRenderer
