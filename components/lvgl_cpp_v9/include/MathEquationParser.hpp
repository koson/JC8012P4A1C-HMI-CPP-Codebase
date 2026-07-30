#pragma once

#include <string>
#include <algorithm>

namespace MathEquationParser
{
    /**
     * @brief Converts a single LaTeX math expression (inside $...$) into clean UTF-8 text representation.
     * E.g., "\overline{A \cdot B}" -> "(A · B)'"
     *       "\overline{\overline{A \cdot B}}" -> "((A · B)')'"
     *       "A \oplus B" -> "A ⊕ B"
     *       "C_{IN}" -> "C_IN"
     */
    // Control bytes for overbar rendering in ThaiLabel & LVCanvas
    // \x01 = Start Overbar, \x02 = End Overbar
    inline std::string format_math_expr(const std::string &math_in)
    {
        std::string s = math_in;

        // 1. Handle \overline{...}, \overbar{...}, and \bar{...} -> \x01 ... \x02
        auto process_overline_tags = [&](const std::string &tag_prefix) {
            size_t tag_len = tag_prefix.length();
            size_t ov_pos = 0;
            while ((ov_pos = s.find(tag_prefix, ov_pos)) != std::string::npos)
            {
                size_t start_content = ov_pos + tag_len;
                int depth = 1;
                size_t end_content = start_content;
                while (end_content < s.length() && depth > 0)
                {
                    if (s[end_content] == '{')
                        depth++;
                    else if (s[end_content] == '}')
                        depth--;
                    if (depth > 0)
                        end_content++;
                }

                if (depth == 0)
                {
                    std::string inner = s.substr(start_content, end_content - start_content);
                    std::string formatted_inner = format_math_expr(inner);
                    std::string replacement = "\x01" + formatted_inner + "\x02";
                    s.replace(ov_pos, (end_content + 1) - ov_pos, replacement);
                    ov_pos += replacement.length();
                }
                else
                {
                    ov_pos += tag_len;
                }
            }
        };

        process_overline_tags("\\overline{");
        process_overline_tags("\\overbar{");
        process_overline_tags("\\bar{");

        // 3. Replace LaTeX symbols with ASCII math operators (compatible with all HMI fonts)
        auto replace_all = [&](const std::string &from, const std::string &to) {
            size_t pos = 0;
            while ((pos = s.find(from, pos)) != std::string::npos)
            {
                s.replace(pos, from.length(), to);
                pos += to.length();
            }
        };

        replace_all("\\cdot", " . ");
        replace_all("\\oplus", " ^ ");
        replace_all("\\otimes", " ~^ ");
        replace_all("\\times", " * ");
        replace_all("\\div", " / ");
        replace_all("\\pm", " +/- ");
        replace_all("\\leq", " <= ");
        replace_all("\\geq", " >= ");
        replace_all("\\neq", " != ");
        replace_all("\\alpha", "alpha");
        replace_all("\\beta", "beta");
        replace_all("\\pi", "pi");
        replace_all("\\mu", "u");
        replace_all("\\Omega", "Ohm");

        // Clean up multi-spaces e.g. " . " -> " . "
        size_t sp_pos = 0;
        while ((sp_pos = s.find("  ", sp_pos)) != std::string::npos)
        {
            s.erase(sp_pos, 1);
        }

        // 4. Subscripts & Superscripts cleanup
        replace_all("_{OUT}", "_OUT");
        replace_all("_{IN}", "_IN");
        replace_all("_{out}", "_out");
        replace_all("_{in}", "_in");
        replace_all("_{0}", "₀");
        replace_all("_{1}", "₁");
        replace_all("_{2}", "₂");
        replace_all("_{3}", "₃");
        replace_all("^2", "²");
        replace_all("^3", "³");

        // Clean up remaining { } brackets if any
        s.erase(std::remove(s.begin(), s.end(), '{'), s.end());
        s.erase(std::remove(s.begin(), s.end(), '}'), s.end());

        return s;
    }

    /**
     * @brief Parses full text containing inline math expressions delimited by $...$.
     * Also strips markdown bold markers (**).
     * E.g. "NAND Gate ($Y = \overline{A \cdot B}$) คือวงจร..."
     *   -> "NAND Gate (Y = (A · B)') คือวงจร..."
     */
    inline std::string process_text_math(const std::string &input)
    {
        if (input.empty())
            return input;

        std::string text = input;

        // Strip markdown bold markers **
        size_t bpos = 0;
        while ((bpos = text.find("**", bpos)) != std::string::npos)
        {
            text.erase(bpos, 2);
        }

        if (text.find('$') == std::string::npos)
            return text;

        std::string result;
        result.reserve(text.length());

        size_t pos = 0;
        while (pos < text.length())
        {
            size_t start = text.find('$', pos);
            if (start == std::string::npos)
            {
                result.append(text, pos, text.length() - pos);
                break;
            }

            // Append text before $
            result.append(text, pos, start - pos);

            size_t end = text.find('$', start + 1);
            if (end == std::string::npos)
            {
                // Unmatched $, append remaining
                result.append(text, start, text.length() - start);
                break;
            }

            // Extract math expression inside $...$
            std::string math_expr = text.substr(start + 1, end - start - 1);
            std::string formatted = format_math_expr(math_expr);
            result.append(formatted);

            pos = end + 1;
        }

        return result;
    }
} // namespace MathEquationParser
