#include "io/csv_writer.hpp"

#include <cstddef>
#include <iomanip>
#include <locale>
#include <ostream>
#include <string>
#include <string_view>

namespace sortbench {
namespace {

/// Quotes a field only when it contains a delimiter, a quote or a newline.
std::string csv_field(std::string_view text) {
    const bool needs_quotes = text.find_first_of(",\"\r\n") != std::string_view::npos;
    if (!needs_quotes) {
        return std::string(text);
    }

    std::string quoted;
    quoted.reserve(text.size() + 2);
    quoted.push_back('"');
    for (char character : text) {
        if (character == '"') {
            quoted.push_back('"');
        }
        quoted.push_back(character);
    }
    quoted.push_back('"');
    return quoted;
}

} // namespace

void write_csv(std::ostream& out, const std::vector<ResultRow>& rows) {
    out.imbue(std::locale::classic());

    out << "algorithm,distribution,n,repetitions,mean_time_ms,stddev_time_ms,"
           "min_time_ms,max_time_ms,mean_comparisons,mean_moves,"
           "aux_memory,aux_bytes,verified\n";

    for (const ResultRow& row : rows) {
        out << csv_field(row.algorithm) << ',' << csv_field(row.distribution) << ',' << row.n << ','
            << row.repetitions << ',' << std::fixed << std::setprecision(6) << row.mean_ms << ','
            << row.stddev_ms << ',' << row.min_ms << ',' << row.max_ms << ','
            << std::setprecision(2) << row.mean_comparisons << ',' << row.mean_moves << ','
            << csv_field(row.aux_memory) << ',' << row.aux_bytes << ','
            << (row.verified ? 1 : 0) << '\n';
    }
}

} // namespace sortbench
