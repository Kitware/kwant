/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

///
/// An example program demonstrating querying for a dynamically selected attribute.
/// This example never uses knowledge of the track format, although it could.
///

#include <iostream>
#include <map>
#include <string>
#include <cstdlib>

#include <track_oracle/track_oracle.h>
#include <track_oracle/element_descriptor.h>
#include <track_oracle/track_field.h>
#include <track_oracle/file_format_manager.h>

#include <logger/logger.h>


using std::string;


#undef VIDTK_DEFAULT_LOGGER
#define VIDTK_DEFAULT_LOGGER __vidtk_logger_auto_runtime_field_query_example_cxx__
VIDTK_LOGGER("runtime_field_query_example_cxx");


using namespace vidtk;

int main( int argc, char *argv[] )
{
  if (argc != 3)
  {
    LOG_INFO( "Usage: " << argv[0] << " track-file field_name\n"
             << "Load in a track file, attempt to see if it supplies field_name.\n"
             << "(Type is assumed to be double.)");
    return EXIT_FAILURE;
  }

  // try to load the tracks
  const string track_fn( argv[1] );
  track_handle_list_type tracks;
  if (! file_format_manager::read( track_fn, tracks ))
  {
    LOG_ERROR( "Error: couldn't read tracks from '" << track_fn << "'; exiting");
    return EXIT_FAILURE;
  }
  LOG_INFO( "Info: read " << tracks.size() << " tracks");
  if ( tracks.empty() )
  {
    LOG_INFO( "Info: reader succeeded but no tracks were loaded?  Weird!");
    return EXIT_FAILURE;
  }

  // Create a track field for the user-specified name
  track_field<double> user_field( argv[2] );

  // our example operation on the user-specified field will be...
  // ... (drum roll) ...
  // ... take the average of it!  BOOOORRRRING yes, but you get the idea.

  double sum = 0.0;
  size_t count = 0;

  // This approach is conservative, in that each individual track and
  // frame is queried to see if the user_field exists... As soon as
  // we find it once, we could remember if it was on the track-level-data
  // or frame-level-data, but here we just blindly ask each time.

  for (size_t t=0; t<tracks.size(); ++t)
  {
    if (user_field.exists( tracks[t].row ))
    {
      sum += user_field( tracks[t].row );
      ++count;
    }

    frame_handle_list_type frames = track_oracle::get_frames( tracks[t] );
    for (size_t f=0; f<frames.size(); ++f)
    {
      if (user_field.exists( frames[f].row ))
      {
        sum += user_field( frames[f].row );
        ++count;
      }
    }
  }

  // all done

  LOG_INFO( "Info: found " << count << " instances of '" << argv[2] << "' in the file");
  if (count > 0)
  {
    LOG_INFO( "Info: ... the sum was " << sum << " and the average was " << sum / count << "");
  }

}
