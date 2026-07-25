#include "SvgPathParser.hpp"
#include "esp_log.h"
#include <cctype>
#include <cstdlib>
#include <cstring>


static const char *TAG = "SvgPathParser";

namespace SvgRenderer {

SvgPathParser::SvgPathParser() : m_currentPos(0, 0), m_startPos(0, 0) {}

SvgPathParser::~SvgPathParser() {}

std::vector<PathCommand> SvgPathParser::parse(const char *pathData) {
  std::vector<PathCommand> commands;

  if (!pathData || strlen(pathData) == 0) {
    m_lastError = "Empty path data";
    ESP_LOGW(TAG, "Empty path data");
    return commands;
  }

  // ESP_LOGD(TAG, "Parsing path: %.80s...", pathData);
  ESP_LOGD(TAG, "Parsing path: %s", pathData);

  const char *ptr = pathData;
  m_currentPos = Point(0, 0);
  m_startPos = Point(0, 0);

  // Skip leading whitespace
  skipWhitespace(&ptr);

  int cmdCount = 0;
  char lastCommand = '\0';

  while (*ptr != '\0') {
    // Skip whitespace
    skipWhitespace(&ptr);

    if (*ptr == '\0')
      break;

    char cmd;

    // Check if this is a command character or implicit repetition
    if (isCommand(*ptr)) {
      cmd = *ptr;
      ptr++;
      lastCommand = cmd;
    } else if (lastCommand != '\0' && lastCommand != 'Z' &&
               lastCommand != 'z') {
      // Implicit command repetition — SVG spec: after M→L, after m→l, others
      // repeat
      if (lastCommand == 'M')
        cmd = 'L';
      else if (lastCommand == 'm')
        cmd = 'l';
      else
        cmd = lastCommand;
      ESP_LOGD(TAG, "  Implicit repeat of command '%c' (last='%c')", cmd,
               lastCommand);
    } else {
      // Invalid - not a command and no lastCommand to repeat
      m_lastError = "Invalid command character";
      ESP_LOGW(TAG, "Invalid command at position %d: '%c'",
               (int)(ptr - pathData), *ptr);
      break;
    }

    // Parse command
    PathCommand command = parseCommand(cmd, &ptr);
    commands.push_back(command);
    cmdCount++;

    ESP_LOGD(TAG, "  Command %d: '%c' with %d args", cmdCount, cmd,
             (int)command.args.size());

    // Skip trailing whitespace
    skipWhitespace(&ptr);
  }

  ESP_LOGD(TAG, "Parsed %d commands total", cmdCount);

  return commands;
}

PathCommand SvgPathParser::parseCommand(char cmd, const char **ptr) {
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

float SvgPathParser::parseNumber(const char **ptr) {
  skipWhitespace(ptr);

  char *endPtr;
  float value = strtof(*ptr, &endPtr);

  if (endPtr == *ptr) {
    m_lastError = "Failed to parse number";
    return 0.0f;
  }

  *ptr = endPtr;
  return value;
}

void SvgPathParser::skipWhitespace(const char **ptr) {
  while (**ptr != '\0' && (isspace(**ptr) || **ptr == ',')) {
    (*ptr)++;
  }
}

bool SvgPathParser::isCommand(char c) const {
  return (c == 'M' || c == 'm' || c == 'L' || c == 'l' || c == 'H' ||
          c == 'h' || c == 'V' || c == 'v' || c == 'C' || c == 'c' ||
          c == 'Q' || c == 'q' || c == 'A' || c == 'a' || c == 'Z' || c == 'z');
}

int SvgPathParser::getArgCount(char cmd) const {
  switch (cmd) {
  case 'M':
  case 'm': // Move to
  case 'L':
  case 'l': // Line to
    return 2;

  case 'H':
  case 'h': // Horizontal line
  case 'V':
  case 'v': // Vertical line
    return 1;

  case 'C':
  case 'c': // Cubic Bezier
    return 6;

  case 'Q':
  case 'q': // Quadratic Bezier
    return 4;

  case 'A':
  case 'a': // Arc
    return 7;

  case 'Z':
  case 'z': // Close path
    return 0;

  default:
    return 0;
  }
}

} // namespace SvgRenderer
