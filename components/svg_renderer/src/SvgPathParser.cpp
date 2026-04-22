#include "SvgPathParser.hpp"
#include <cstring>
#include <cctype>
#include <cstdlib>

namespace SvgRenderer {

SvgPathParser::SvgPathParser() 
    : m_currentPos(0, 0)
    , m_startPos(0, 0) {
}

SvgPathParser::~SvgPathParser() {
}

std::vector<PathCommand> SvgPathParser::parse(const char* pathData) {
    std::vector<PathCommand> commands;
    
    if (!pathData || strlen(pathData) == 0) {
        m_lastError = "Empty path data";
        return commands;
    }
    
    const char* ptr = pathData;
    m_currentPos = Point(0, 0);
    m_startPos = Point(0, 0);
    
    // Skip leading whitespace
    skipWhitespace(&ptr);
    
    while (*ptr != '\0') {
        // Skip whitespace
        skipWhitespace(&ptr);
        
        if (*ptr == '\0') break;
        
        // Get command character
        if (!isCommand(*ptr)) {
            m_lastError = "Invalid command character";
            break;
        }
        
        char cmd = *ptr;
        ptr++;
        
        // Parse command
        PathCommand command = parseCommand(cmd, &ptr);
        commands.push_back(command);
        
        // Skip trailing whitespace
        skipWhitespace(&ptr);
    }
    
    return commands;
}

PathCommand SvgPathParser::parseCommand(char cmd, const char** ptr) {
    PathCommand command(cmd);
    
    int argCount = getArgCount(cmd);
    
    // Z command has no arguments
    if (cmd == 'Z' || cmd == 'z') {
        return command;
    }
    
    // Parse arguments
    for (int i = 0; i < argCount; i++) {
        skipWhitespace(ptr);
        float value = parseNumber(ptr);
        command.args.push_back(value);
        skipWhitespace(ptr);
        
        // Skip comma separator
        if (**ptr == ',') {
            (*ptr)++;
            skipWhitespace(ptr);
        }
    }
    
    return command;
}

float SvgPathParser::parseNumber(const char** ptr) {
    skipWhitespace(ptr);
    
    char* endPtr;
    float value = strtof(*ptr, &endPtr);
    
    if (endPtr == *ptr) {
        m_lastError = "Failed to parse number";
        return 0.0f;
    }
    
    *ptr = endPtr;
    return value;
}

void SvgPathParser::skipWhitespace(const char** ptr) {
    while (**ptr != '\0' && (isspace(**ptr) || **ptr == ',')) {
        (*ptr)++;
    }
}

bool SvgPathParser::isCommand(char c) const {
    return (c == 'M' || c == 'm' ||
            c == 'L' || c == 'l' ||
            c == 'H' || c == 'h' ||
            c == 'V' || c == 'v' ||
            c == 'C' || c == 'c' ||
            c == 'Q' || c == 'q' ||
            c == 'A' || c == 'a' ||
            c == 'Z' || c == 'z');
}

int SvgPathParser::getArgCount(char cmd) const {
    switch (cmd) {
        case 'M': case 'm':  // Move to
        case 'L': case 'l':  // Line to
            return 2;
            
        case 'H': case 'h':  // Horizontal line
        case 'V': case 'v':  // Vertical line
            return 1;
            
        case 'C': case 'c':  // Cubic Bezier
            return 6;
            
        case 'Q': case 'q':  // Quadratic Bezier
            return 4;
            
        case 'A': case 'a':  // Arc
            return 7;
            
        case 'Z': case 'z':  // Close path
            return 0;
            
        default:
            return 0;
    }
}

} // namespace SvgRenderer
