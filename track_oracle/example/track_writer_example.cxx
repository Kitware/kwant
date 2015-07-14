/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

///
/// An example program demonstrating track writing.
///

#include <track_oracle/track_oracle.h>
#include <track_oracle/file_format_base.h>
#include <track_oracle/file_format_manager.h>

#include <logger/logger.h>
#undef VIDTK_DEFAULT_LOGGER
#define VIDTK_DEFAULT_LOGGER __vidtk_logger_auto_track_reader_example_cxx__
VIDTK_LOGGER("track_writer_example_cxx");

using std::cout;
using namespace vidtk;

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    LOG_INFO("Usage: " << argv[0] << " input_file format [output_file]\n\n" <<
             "Attempts to load track data from <input_file> and write it out\n"
             "again using <format> to <output_file>. If not <output_file> is\n"
             "specified, use stdout. Note that writing to stdout requires\n"
             "that <format> supports writing to streams, while specifying\n"
             "an <output_file> requires that <format> supports writing to a\n"
             "file.");
    return EXIT_FAILURE;
  }

  track_handle_list_type tracks;
  if (!file_format_manager::read(argv[1], tracks))
  {
    LOG_ERROR("Error: could not read tracks from '" << argv[1] << '\'');
    return EXIT_FAILURE;
  }

  file_format_enum const ff_type = file_format_type::from_string(argv[2]);
  if (ff_type == TF_INVALID_TYPE)
  {
    LOG_ERROR("Error: could not find format '" << argv[2] << '\'');
    return EXIT_FAILURE;
  }

  file_format_base* const ff = file_format_manager::get_format(ff_type);
  if (!ff)
  {
    LOG_ERROR("Error: failed to load file format manager for '" << argv[2] << '\'');
    return EXIT_FAILURE;
  }

  if (argc > 3)
  {
    if (!ff->write(argv[3], tracks))
    {
      LOG_ERROR("Error: failed to write tracks to '" << argv[3] << '\'');
      return EXIT_FAILURE;
    }
  }
  else
  {
    if (!ff->write(cout, tracks))
    {
      LOG_ERROR("Error: failed to write tracks to stdout");
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
